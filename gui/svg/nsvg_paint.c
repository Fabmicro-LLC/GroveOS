/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
//#define SVGLIB_IMPL
#include "svglib.h"

#define NSVG_PAINT_IMPL
#include "nsvg_paint.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

#define NSVG__SUBSAMPLES	3
#define NSVG__FIXSHIFT		10
#define NSVG__FIX		(1 << NSVG__FIXSHIFT)
#define NSVG__FIXMASK		(NSVG__FIX-1)
#define NSVG__MEMPAGE_SIZE	1024

typedef struct NSVGactiveEdge {
	int x,dx;
	float ey;
	int dir;
	struct NSVGactiveEdge *next;
} NSVGactiveEdge;

typedef struct NSVGmemPage {
	unsigned char mem[NSVG__MEMPAGE_SIZE];
	int size;
	struct NSVGmemPage* next;
} NSVGmemPage;

typedef struct NSVGcachedPaint {
	char type;
	char spread;
	float xform[6];
	unsigned int colors[256];
} NSVGcachedPaint;

struct NSVGfromEdgesRasterizer
{
	float px, py;

	const NSVGedge* edges;
	int nedges;


	NSVGactiveEdge* freelist;
	NSVGmemPage* pages;
	NSVGmemPage* curpage;

	unsigned char* scanline;
	int cscanline;

	unsigned char* bitmap;
	int width, height, stride;
	Context *gc;
};

extern unsigned char* pixel_addr(Context *gc, int x, int y);


NSVGfromEdgesRasterizer* nsvgCreateFromEdgesRasterizer()
{
	NSVGfromEdgesRasterizer* r = (NSVGfromEdgesRasterizer*)malloc(sizeof(NSVGfromEdgesRasterizer));
	if (r == NULL) goto error;
	memset(r, 0, sizeof(NSVGfromEdgesRasterizer));

	return r;

error:
	return NULL;
}

void nsvgDeleteFromEdgesRasterizer(NSVGfromEdgesRasterizer* r)
{
	NSVGmemPage* p;

	if (r == NULL) return;

	p = r->pages;
	while (p != NULL) {
		NSVGmemPage* next = p->next;
		free(p);
		p = next;
	}

	//if (r->edges) free(r->edges);
	if (r->scanline) free(r->scanline);

	free(r);
}

typedef unsigned char COLOR;

typedef struct {
	unsigned char a;
	unsigned char b;
	unsigned char g;
	unsigned char r;
} COLOR_COMPONENTS;

inline static COLOR make_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	//r = (r & 0xc0);
	//g = (g & 0xc0);
	//b = (b & 0xc0);
	
	//return (a<<24 | b<<16 | g<<8 | r);
	return ((((r >> 5) & 0x07) << 5) | (((g >> 5) & 0x07) << 2) | ((b >> 6) & 0x07));
}

inline static void set_pixel(NSVGfromEdgesRasterizer *r, int x, int y, COLOR rgba) {
	unsigned char *addr = pixel_addr(r->gc, x, y);
	*(COLOR*) addr = rgba;
}

inline static COLOR get_pixel(NSVGfromEdgesRasterizer *r, int x, int y) {
	unsigned char *addr = pixel_addr(r->gc, x, y);
	return *(COLOR*) addr;
}

inline static COLOR_COMPONENTS get_color_components(COLOR c) {
	COLOR_COMPONENTS res;
	res.r = ((c >> 5) & 0x07) << 5;
        res.g = ((c >> 2) & 0x07) << 5;
        res.b = (c & 0x03) << 6;
        res.a = 0xff;

	return res;
}


static NSVGmemPage* nsvg__nextPage(NSVGfromEdgesRasterizer* r, NSVGmemPage* cur)
{
	NSVGmemPage *newp;

	// If using existing chain, return the next page in chain
	if (cur != NULL && cur->next != NULL) {
		return cur->next;
	}

	// Alloc new page
	print("nsvg__nextPage Alloc new page of size: %d\n", sizeof(NSVGmemPage));
	newp = (NSVGmemPage*)malloc(sizeof(NSVGmemPage));
	if (newp == NULL) return NULL;
	memset(newp, 0, sizeof(NSVGmemPage));

	// Add to linked list
	if (cur != NULL)
		cur->next = newp;
	else
		r->pages = newp;

	return newp;
}

static void nsvg__resetPool(NSVGfromEdgesRasterizer* r)
{
	NSVGmemPage* p = r->pages;
	while (p != NULL) {
		p->size = 0;
		p = p->next;
	}
	r->curpage = r->pages;
}

static unsigned char* nsvg__alloc(NSVGfromEdgesRasterizer* r, int size)
{
	unsigned char* buf;
	if (size > NSVG__MEMPAGE_SIZE) return NULL;
	if (r->curpage == NULL || r->curpage->size+size > NSVG__MEMPAGE_SIZE) {
		r->curpage = nsvg__nextPage(r, r->curpage);
	}
	buf = &r->curpage->mem[r->curpage->size];
	r->curpage->size += size;
	return buf;
}

static NSVGactiveEdge* nsvg__addActive(NSVGfromEdgesRasterizer* r, const NSVGedge* e, float startPoint)
{
	 NSVGactiveEdge* z;

	if (r->freelist != NULL) {
		// Restore from freelist.
		z = r->freelist;
		r->freelist = z->next;
	} else {
		// Alloc new edge.
		z = (NSVGactiveEdge*)nsvg__alloc(r, sizeof(NSVGactiveEdge));
		if (z == NULL) return NULL;
	}

	float dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
//	STBTT_assert(e->y0 <= start_point);
	// round dx down to avoid going too far
	if (dxdy < 0)
		z->dx = (int)(-floorf(NSVG__FIX * -dxdy));
	else
		z->dx = (int)floorf(NSVG__FIX * dxdy);
	z->x = (int)floorf(NSVG__FIX * (e->x0 + dxdy * (startPoint - e->y0)));
	z->ey = e->y1;
	z->next = 0;
	z->dir = e->dir;

	return z;
}

static void nsvg__freeActive(NSVGfromEdgesRasterizer* r, NSVGactiveEdge* z)
{
	z->next = r->freelist;
	r->freelist = z;
}

static void nsvg__fillScanline(unsigned char* scanline, int len, int x0, int x1, int maxWeight, int* xmin, int* xmax)
{
	//print("fillScanline  len=%d, x0=%d, x1=%d, xmin=%d, xmax=%d\n", len, (x0 >> NSVG__FIXSHIFT) , (x1 >> NSVG__FIXSHIFT), *xmin, *xmax);
	int i = x0 >> NSVG__FIXSHIFT;
	int j = x1 >> NSVG__FIXSHIFT;
	if (i < *xmin) *xmin = i;
	if (j > *xmax) *xmax = j;
	if (i < len && j >= 0) {
		if (i == j) {
			// x0,x1 are the same pixel, so compute combined coverage
			scanline[i] = (unsigned char)(scanline[i] + ((x1 - x0) * maxWeight >> NSVG__FIXSHIFT));
		} else {
			if (i >= 0) // add antialiasing for x0
				scanline[i] = (unsigned char)(scanline[i] + (((NSVG__FIX - (x0 & NSVG__FIXMASK)) * maxWeight) >> NSVG__FIXSHIFT));
			else
				i = -1; // clip

			if (j < len) // add antialiasing for x1
				scanline[j] = (unsigned char)(scanline[j] + (((x1 & NSVG__FIXMASK) * maxWeight) >> NSVG__FIXSHIFT));
			else
				j = len; // clip

			for (++i; i < j; ++i) // fill pixels between x0 and x1
				scanline[i] = (unsigned char)(scanline[i] + maxWeight);
		}
	}
}

// note: this routine clips fills that extend off the edges... ideally this
// wouldn't happen, but it could happen if the truetype glyph bounding boxes
// are wrong, or if the user supplies a too-small bitmap
static void nsvg__fillActiveEdges(unsigned char* scanline, int len, NSVGactiveEdge* e, int maxWeight, int* xmin, int* xmax, char fillRule)
{
	// non-zero winding fill
	int x0 = 0, w = 0;

	if (fillRule == NSVG_FILLRULE_NONZERO) {
		// Non-zero
		while (e != NULL) {
			if (w == 0) {
				// if we're currently at zero, we need to record the edge start point
				x0 = e->x; w += e->dir;
			} else {
				int x1 = e->x; w += e->dir;
				// if we went to zero, we need to draw
				if (w == 0)
					nsvg__fillScanline(scanline, len, x0, x1, maxWeight, xmin, xmax);
			}
			e = e->next;
		}
	} else if (fillRule == NSVG_FILLRULE_EVENODD) {
		// Even-odd
		while (e != NULL) {
			if (w == 0) {
				// if we're currently at zero, we need to record the edge start point
				x0 = e->x; w = 1;
			} else {
				int x1 = e->x; w = 0;
				nsvg__fillScanline(scanline, len, x0, x1, maxWeight, xmin, xmax);
			}
			e = e->next;
		}
	}
}

static float nsvg__clampf(float a, float mn, float mx) { return a < mn ? mn : (a > mx ? mx : a); }

static unsigned int nsvg__RGBA(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	return (r) | (g << 8) | (b << 16) | (a << 24);
}

static unsigned int nsvg__lerpRGBA(unsigned int c0, unsigned int c1, float u)
{
	int iu = (int)(nsvg__clampf(u, 0.0f, 1.0f) * 256.0f);
	int r = (((c0) & 0xff)*(256-iu) + (((c1) & 0xff)*iu)) >> 8;
	int g = (((c0>>8) & 0xff)*(256-iu) + (((c1>>8) & 0xff)*iu)) >> 8;
	int b = (((c0>>16) & 0xff)*(256-iu) + (((c1>>16) & 0xff)*iu)) >> 8;
	int a = (((c0>>24) & 0xff)*(256-iu) + (((c1>>24) & 0xff)*iu)) >> 8;
	return nsvg__RGBA((unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a);
}

static unsigned int nsvg__applyOpacity(unsigned int c, float u)
{
	int iu = (int)(nsvg__clampf(u, 0.0f, 1.0f) * 256.0f);
	int r = (c) & 0xff;
	int g = (c>>8) & 0xff;
	int b = (c>>16) & 0xff;
	int a = (((c>>24) & 0xff)*iu) >> 8;
	return nsvg__RGBA((unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a);
}

static inline int nsvg__div255(int x)
{
    return ((x+1) * 257) >> 16;
}

static void nsvg__scanlineSolid(NSVGfromEdgesRasterizer *rast, int count, unsigned char* cover, int x, int y,
								float tx, float ty, float scale, NSVGcachedPaint* cache)
{

	if (cache->type == NSVG_PAINT_COLOR) {
		COLOR_COMPONENTS psrc;
		psrc.r = (cache->colors[0]) & 0xff;
        	psrc.g = (cache->colors[0] >> 8) & 0xff;
        	psrc.b = (cache->colors[0] >> 16) & 0xff;
        	psrc.a = (cache->colors[0] >> 24) & 0xff;

		for (int i = 0; i < count; i++) {
			if(!PtInRect(&rast->gc->clip_rect, x+i, y))  continue;

			int r, g, b;
			int a = nsvg__div255((int)cover[i] * psrc.a);
			int ia = 255 - a;

			if(a == 0xff) {
				set_pixel(rast, x+i, y, make_color(psrc.r,psrc.g,psrc.b,a));
			} else if(a == 0x00) {
				//
			} else {
				// Premultiply
				r = nsvg__div255(psrc.r * a);
				g = nsvg__div255(psrc.g * a);
				b = nsvg__div255(psrc.b * a);


				// Blend over
				COLOR_COMPONENTS pdst=get_color_components(get_pixel(rast, x+i, y));

				r += nsvg__div255(ia * (int)pdst.r);
				g += nsvg__div255(ia * (int)pdst.g);
				b += nsvg__div255(ia * (int)pdst.b);
				a += nsvg__div255(ia * (int)pdst.a);

				set_pixel(rast, x+i, y, make_color(r,g,b,a));
			}
		}

	}
	/*
	} else if (cache->type == NSVG_PAINT_LINEAR_GRADIENT) {
		// TODO: spread modes.
		// TODO: plenty of opportunities to optimize.
		float fx, fy, dx, gy;
		float* t = cache->xform;
		int i, cr, cg, cb, ca;
		unsigned int c;

		fx = ((float)x - tx) / scale;
		fy = ((float)y - ty) / scale;
		dx = 1.0f / scale;

		for (i = 0; i < count; i++) {
			int r,g,b,a,ia;
			gy = fx*t[1] + fy*t[3] + t[5];
			c = cache->colors[(int)nsvg__clampf(gy*255.0f, 0, 255.0f)];
			cr = (c) & 0xff;
			cg = (c >> 8) & 0xff;
			cb = (c >> 16) & 0xff;
			ca = (c >> 24) & 0xff;

			a = nsvg__div255((int)cover[0] * ca);
			ia = 255 - a;

			// Premultiply
			r = nsvg__div255(cr * a);
			g = nsvg__div255(cg * a);
			b = nsvg__div255(cb * a);

			// Blend over
			r += nsvg__div255(ia * (int)dst[0]);
			g += nsvg__div255(ia * (int)dst[1]);
			b += nsvg__div255(ia * (int)dst[2]);
			a += nsvg__div255(ia * (int)dst[3]);

			dst[0] = (unsigned char)r;
			dst[1] = (unsigned char)g;
			dst[2] = (unsigned char)b;
			dst[3] = (unsigned char)a;

			cover++;
			dst += 4;
			fx += dx;
		}
	} else if (cache->type == NSVG_PAINT_RADIAL_GRADIENT) {
		// TODO: spread modes.
		// TODO: plenty of opportunities to optimize.
		// TODO: focus (fx,fy)
		float fx, fy, dx, gx, gy, gd;
		float* t = cache->xform;
		int i, cr, cg, cb, ca;
		unsigned int c;

		fx = ((float)x - tx) / scale;
		fy = ((float)y - ty) / scale;
		dx = 1.0f / scale;

		for (i = 0; i < count; i++) {
			int r,g,b,a,ia;
			gx = fx*t[0] + fy*t[2] + t[4];
			gy = fx*t[1] + fy*t[3] + t[5];
			gd = sqrtf(gx*gx + gy*gy);
			c = cache->colors[(int)nsvg__clampf(gd*255.0f, 0, 255.0f)];
			cr = (c) & 0xff;
			cg = (c >> 8) & 0xff;
			cb = (c >> 16) & 0xff;
			ca = (c >> 24) & 0xff;

			a = nsvg__div255((int)cover[0] * ca);
			ia = 255 - a;

			// Premultiply
			r = nsvg__div255(cr * a);
			g = nsvg__div255(cg * a);
			b = nsvg__div255(cb * a);

			// Blend over
			r += nsvg__div255(ia * (int)dst[0]);
			g += nsvg__div255(ia * (int)dst[1]);
			b += nsvg__div255(ia * (int)dst[2]);
			a += nsvg__div255(ia * (int)dst[3]);

			dst[0] = (unsigned char)r;
			dst[1] = (unsigned char)g;
			dst[2] = (unsigned char)b;
			dst[3] = (unsigned char)a;

			cover++;
			dst += 4;
			fx += dx;
		}
	}

	*/
}

static void nsvg__rasterizeSortedEdges(NSVGfromEdgesRasterizer *r, float tx, float ty, float scale, NSVGcachedPaint* cache, char fillRule)
{
	NSVGactiveEdge *active = NULL;
	int y, s;
	int e = 0;
	int maxWeight = (255 / NSVG__SUBSAMPLES);  // weight per vertical scanline
	int xmin, xmax;

	for (y = 0; y < r->height; y++) {
		if(y < r->gc->clip_rect.top - r->gc->translate.y || y >= r->gc->clip_rect.bottom  - r->gc->translate.y) continue;

		memset(r->scanline, 0, r->width);
		xmin = r->width;
		xmax = 0;
		for (s = 0; s < NSVG__SUBSAMPLES; ++s) {
			// find center of pixel for this scanline
			float scany = (float)(y*NSVG__SUBSAMPLES + s) + 0.5f;
			NSVGactiveEdge **step = &active;

			// update all active edges;
			// remove all active edges that terminate before the center of this scanline
			while (*step) {
				NSVGactiveEdge *z = *step;
				if (z->ey <= scany) {
					*step = z->next; // delete from list
//					NSVG__assert(z->valid);
					nsvg__freeActive(r, z);
				} else {
					z->x += z->dx; // advance to position for current scanline
					step = &((*step)->next); // advance through list
				}
			}

			// resort the list if needed
			for (;;) {
				int changed = 0;
				step = &active;
				while (*step && (*step)->next) {
					if ((*step)->x > (*step)->next->x) {
						NSVGactiveEdge* t = *step;
						NSVGactiveEdge* q = t->next;
						t->next = q->next;
						q->next = t;
						*step = q;
						changed = 1;
					}
					step = &(*step)->next;
				}
				if (!changed) break;
			}

			// insert all edges that start before the center of this scanline -- omit ones that also end on this scanline
			while (e < r->nedges && r->edges[e].y0 <= scany) {
				if (r->edges[e].y1 > scany) {
					NSVGactiveEdge* z = nsvg__addActive(r, &r->edges[e], scany);
					if (z == NULL) break;
					// find insertion point
					if (active == NULL) {
						active = z;
					} else if (z->x < active->x) {
						// insert at front
						z->next = active;
						active = z;
					} else {
						// find thing to insert AFTER
						NSVGactiveEdge* p = active;
						while (p->next && p->next->x < z->x)
							p = p->next;
						// at this point, p->next->x is NOT < z->x
						z->next = p->next;
						p->next = z;
					}
				}
				e++;
			}

			// now process all active edges in non-zero fashion
			if (active != NULL)
				nsvg__fillActiveEdges(r->scanline, r->width, active, maxWeight, &xmin, &xmax, fillRule);
		}



		// Blit
		if (xmin < 0) xmin = 0;
		if (xmax > r->width-1) xmax = r->width-1;

		if (xmin <= xmax) {
			//print("RASTER: y=%d, xmax=%d, xmin=%d\n", y, xmax, xmin);
			
			int x2 = xmax + r->gc->translate.x;
			int x1 = xmin + r->gc->translate.x;

			//if(x2 >= r->gc->clip_rect.right ) x2 = r->gc->clip_rect.right-1;
			//if(x1 < r->gc->clip_rect.left) x1 = r->gc->clip_rect.left;

			int count = x2-x1+1;
			if(count>0) nsvg__scanlineSolid(r, count, &r->scanline[xmin], x1, y + r->gc->translate.y, tx,ty, scale, cache);
		}
	}

}

static void nsvg__unpremultiplyAlpha(unsigned char* image, int w, int h, int stride)
{
	int x,y;

	// Unpremultiply
	for (y = 0; y < h; y++) {
		unsigned char *row = &image[y*stride];
		for (x = 0; x < w; x++) {
			int r = row[0], g = row[1], b = row[2], a = row[3];
			if (a != 0) {
				row[0] = (unsigned char)(r*255/a);
				row[1] = (unsigned char)(g*255/a);
				row[2] = (unsigned char)(b*255/a);
			}
			row += 4;
		}
	}

	// Defringe
	for (y = 0; y < h; y++) {
		unsigned char *row = &image[y*stride];
		for (x = 0; x < w; x++) {
			int r = 0, g = 0, b = 0, a = row[3], n = 0;
			if (a == 0) {
				if (x-1 > 0 && row[-1] != 0) {
					r += row[-4];
					g += row[-3];
					b += row[-2];
					n++;
				}
				if (x+1 < w && row[7] != 0) {
					r += row[4];
					g += row[5];
					b += row[6];
					n++;
				}
				if (y-1 > 0 && row[-stride+3] != 0) {
					r += row[-stride];
					g += row[-stride+1];
					b += row[-stride+2];
					n++;
				}
				if (y+1 < h && row[stride+3] != 0) {
					r += row[stride];
					g += row[stride+1];
					b += row[stride+2];
					n++;
				}
				if (n > 0) {
					row[0] = (unsigned char)(r/n);
					row[1] = (unsigned char)(g/n);
					row[2] = (unsigned char)(b/n);
				}
			}
			row += 4;
		}
	}
}


static void nsvg__initPaint(NSVGcachedPaint* cache, NSVGpaint* paint, float opacity)
{
	int i, j;
	NSVGgradient* grad;

	cache->type = paint->type;

	if (paint->type == NSVG_PAINT_COLOR) {
		cache->colors[0] = nsvg__applyOpacity(paint->color, opacity);
		return;
	}

	grad = paint->gradient;

	cache->spread = grad->spread;
	memcpy(cache->xform, grad->xform, sizeof(float)*6);

	if (grad->nstops == 0) {
		for (i = 0; i < 256; i++)
			cache->colors[i] = 0;
	} if (grad->nstops == 1) {
		for (i = 0; i < 256; i++)
			cache->colors[i] = nsvg__applyOpacity(grad->stops[i].color, opacity);
	} else {
		unsigned int ca, cb = 0;
		float ua, ub, du, u;
		int ia, ib, count;

		ca = nsvg__applyOpacity(grad->stops[0].color, opacity);
		ua = nsvg__clampf(grad->stops[0].offset, 0, 1);
		ub = nsvg__clampf(grad->stops[grad->nstops-1].offset, ua, 1);
		ia = (int)(ua * 255.0f);
		ib = (int)(ub * 255.0f);
		for (i = 0; i < ia; i++) {
			cache->colors[i] = ca;
		}

		for (i = 0; i < grad->nstops-1; i++) {
			ca = nsvg__applyOpacity(grad->stops[i].color, opacity);
			cb = nsvg__applyOpacity(grad->stops[i+1].color, opacity);
			ua = nsvg__clampf(grad->stops[i].offset, 0, 1);
			ub = nsvg__clampf(grad->stops[i+1].offset, 0, 1);
			ia = (int)(ua * 255.0f);
			ib = (int)(ub * 255.0f);
			count = ib - ia;
			if (count <= 0) continue;
			u = 0;
			du = 1.0f / (float)count;
			for (j = 0; j < count; j++) {
				cache->colors[ia+j] = nsvg__lerpRGBA(ca,cb,u);
				u += du;
			}
		}

		for (i = ib; i < 256; i++)
			cache->colors[i] = cb;
	}

}

void nsvgRasterizeFromEdges(NSVGfromEdgesRasterizer* rast,
				   NSVGimage* image, float tx, float ty, float scale,
				   unsigned char* dst, int w, int h, int stride, Context* gc)
{

	NSVGshape *shape = NULL;
	NSVGedge *e = NULL;
	NSVGcachedPaint cache;
	int i;

	rast->bitmap = dst;
	rast->width = w;
	rast->height = h;
	rast->stride = stride;
	rast->gc = gc;

	if (w > rast->cscanline) {
		rast->cscanline = w;
		rast->scanline = (unsigned char*)realloc(rast->scanline, w);
		if (rast->scanline == NULL) return;
	}


	for(int i=0; i<image->shapes_len; i++) {
		shape = (NSVGshape*) &image->shapes[i];

		if (!(shape->flags & NSVG_FLAGS_VISIBLE))
			continue;

		if (shape->fill.type != NSVG_PAINT_NONE) {

			rast->edges = shape->edges_fill;
			rast->nedges = shape->nedges_fill;

			nsvg__resetPool(rast);
			rast->freelist = NULL;

			// now, traverse the scanlines and find the intersections on each scanline, use non-zero rule
			nsvg__initPaint(&cache, &shape->fill, shape->opacity);
			nsvg__rasterizeSortedEdges(rast, tx,ty,scale, &cache, shape->fillRule);
		}

		if (shape->stroke.type != NSVG_PAINT_NONE && (shape->strokeWidth * scale) > 0.01f) {

                        rast->edges = shape->edges_stroke;
                        rast->nedges = shape->nedges_stroke;

                        nsvg__resetPool(rast);
                        rast->freelist = NULL;

			// now, traverse the scanlines and find the intersections on each scanline, use non-zero rule
			nsvg__initPaint(&cache, &shape->stroke, shape->opacity);
			nsvg__rasterizeSortedEdges(rast, tx,ty,scale, &cache, NSVG_FILLRULE_NONZERO);
		}
	}

	//nsvg__unpremultiplyAlpha(dst, w, h, stride);

	rast->bitmap = NULL;
	rast->width = 0;
	rast->height = 0;
	rast->stride = 0;
}

/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include "rect.h"
#include "pixel_font.h"
#include "bitmap.h"

typedef uint8_t Color;

#define SET_PIXEL(addr, rgb) {*((uint8_t*)addr) = rgb;}

#define COLOR(r,g,b) (((r & 0x07)<<5) | ((g & 0x07)<<2) | (b & 0x03))

#define COLOR332(R,G,B)  ((((R >> 5) & 0x07) << 5) | (((G >> 5) & 0x07) << 2) | ((B >> 6) & 0x07))
#define WHITE 0xff
#define BLACK 0x00
#define BLUE (COLOR332(0,0,0xff))
#define GREEN (COLOR332(0,0xff,0))
#define RED (COLOR332(0xff,0,0))
#define GREY_DARK (2<<5 | 2<<2 | 1)
#define GREY (4<<5 | 4<<2 | 1)
#define GREY_LIGHT (5<<5 | 5<<2 | 2)
#define GREY_LIGHT2 (6<<5 | 6<<2 | 2)
#define YELLOW (COLOR332(0xff,0xff,0x0))


typedef struct {
	unsigned char *framebuffer;
	Rect framebuffer_rect;	
	int bytes_per_pixel;
	int line_size_in_bytes;
	int width;//in pixels
	int height;//in pixels
	Font *font;
	Point translate;
	Rect clip_rect;
} Context;

typedef enum  {
	SRC_PAINT,
	SRC_PAINT_IF_DST0,
} PixelOperation;

typedef unsigned short UCHAR;

void gc_init(Context* gc, unsigned char *framebuffer, Rect *framebuffer_rect, Font *font);
void gc_set_font(Context* gc, Font *font);
void gc_translate(Context* gc, int x, int y);
void gc_clip(Context* gc, Rect rect);

void gc_draw_line(Context* gc, int x1, int y1, int x2, int y2, Color rgb);
void gc_draw_wide_line(Context* gc, int x1, int y1, int x2, int y2, int width, Color rgb);
void gc_draw_line_from_center(Context *gc, int center_x, int center_y, double angle_degree, double radius, int line_width, Color rgb);

void gc_draw_text(Context* gc, char *s, int len, int x, int y, Color rgb);

void gc_draw_bitmap(Context* gc, const unsigned char* bitmap, int bitmap_size, int bitmap_x, int bitmap_y, int bitmap_width, int bitmap_height, int dst_x, int dst_y);
void gc_draw_bitmap_pixels(Context* gc, Bitmap* bitmap, int bitmap_x, int bitmap_y, int bitmap_width, int bitmap_height, int dst_x, int dst_y, PixelOperation op);


void gc_fill_circle(Context *gc, int xm, int ym, int radius, Color rgb);
void gc_draw_circle(Context *gc, int xm, int ym, int r, Color rgb);

void gc_fill_rect(Context* gc, int x1, int y1, int width, int height, Color rgb);
void gc_draw_rect(Context *gc, int x1, int y1, int width, int height, int line_width, Color rgb);

Color gc_alpha_blend(Color src, Color dst, unsigned int alpha);

#endif //_GRAPHICS_H_

#ifdef GRAPHICS_IMPL

#ifndef _GRAPHICS_H_IMPL_
#define _GRAPHICS_H_IMPL_

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "uilib/utf8.h"

#define my_lrint(a) ((long)(a+0.5))
#define HYPOT(x,y)      sqrt((x)*(x)+(y)*(y))
#define EPS 1E-9

typedef struct {
        unsigned int alpha;
        unsigned int color;
        int u, v;         /* delta x , delta y */
        int ku, kt, kv, kd;     /* loop constants */
        int oct2;
        int quad4;
} murphy_info;


void set_pixel(Context* gc, int x, int y, Color rgb);
unsigned char* pixel_addr(Context *gc, int x, int y);

static void draw_horizontal_line(Context* gc, int x1, int y1, int x2, Color rgb);
static void draw_vertical_line(Context* gc, int x1, int y1, int y2, Color rgb);
static void fill_rect(Context* gc, int x1, int y1, int width, int height, Color rgb);
static void draw_line(Context* gc, int x1, int y1, int x2, int y2, Color rgb);
static void draw_wide_line(Context* gc, int x1, int y1, int x2, int y2, int width, Color rgb);
static void draw_text(Context* gc, char *s, int len, int x, int y, Color rgb);
static void draw_bitmap(Context* gc, const unsigned char* bitmap, int bitmap_size, int bitmap_x, int bitmap_y, int bitmap_width, int bitmap_height, int dst_x, int dst_y);
static void draw_bitmap_pixels(Context* gc, Bitmap* bitmap, int bitmap_x, int bitmap_y, int bitmap_width, int bitmap_height, int dst_x, int dst_y, PixelOperation op);
static void draw_circle(Context *gc, int xm, int ym, int r, Color rgb);
static void fill_circle(Context *gc, int xm, int ym, int radius, Color rgb);
static void draw_line_from_center(Context *gc, int center_x, int center_y, double angle_degree, double radius, int line_width, Color rgb);
static void draw_rect(Context *gc, int x1, int y1, int width, int height, int line_width, Color rgb);
static void murphy_wideline(Context *gc, Point p0, Point p1, int width, Color color);
static void murphy_paraline(Context* gc, murphy_info *murphy, Point pt, int d1);

void gc_draw_line(Context* gc, int x1, int y1, int x2, int y2, Color rgb) {
	draw_line(gc, gc->translate.x + x1, gc->translate.y + y1, gc->translate.x + x2, gc->translate.y + y2, rgb);
}

void gc_draw_wide_line(Context* gc, int x1, int y1, int x2, int y2, int width, Color rgb) {
	draw_wide_line(gc, gc->translate.x + x1, gc->translate.y + y1, gc->translate.x + x2, gc->translate.y + y2, width, rgb);
}

void gc_draw_line_from_center(Context *gc, int center_x, int center_y, double angle_degree, double radius, int line_width, Color rgb) {
	draw_line_from_center(gc, gc->translate.x + center_x, gc->translate.y + center_y, angle_degree, radius, line_width, rgb);
}

void gc_draw_text(Context* gc, char *s, int len, int x, int y, Color rgb) {
	draw_text(gc, s,len,  gc->translate.x + x, gc->translate.y + y, rgb);
}

void gc_draw_bitmap(Context* gc, const unsigned char* bitmap, int bitmap_size, int bitmap_x, int bitmap_y, int bitmap_width, int bitmap_height, int dst_x, int dst_y) {
	draw_bitmap(gc, bitmap, bitmap_size, bitmap_x, bitmap_y, bitmap_width, bitmap_height, gc->translate.x + dst_x, gc->translate.y + dst_y);
}
	
void gc_draw_bitmap_pixels(Context* gc, Bitmap* bitmap, int bitmap_x, int bitmap_y, int bitmap_width, int bitmap_height, int dst_x, int dst_y, PixelOperation op) {
	draw_bitmap_pixels(gc, bitmap, bitmap_x, bitmap_y, bitmap_width, bitmap_height, gc->translate.x + dst_x, gc->translate.y + dst_y, op);
}

void gc_fill_circle(Context *gc, int xm, int ym, int radius, Color rgb) {
	fill_circle(gc, gc->translate.x + xm, gc->translate.y + ym, radius, rgb);
}

void gc_draw_circle(Context *gc, int xm, int ym, int r, Color rgb) {
	draw_circle(gc, gc->translate.x + xm, gc->translate.y + ym, r, rgb);
}

void gc_fill_rect(Context* gc, int x1, int y1, int width, int height, Color rgb) {
	//print("gc_fill_rect x1=%d, y1=%d, translate.x=%d, translate.y=%d\n", x1,y1,gc->translate.x,gc->translate.y);
	fill_rect(gc, gc->translate.x + x1, gc->translate.y + y1, width, height, rgb);
}

void gc_draw_rect(Context *gc, int x1, int y1, int width, int height, int line_width, Color rgb) {
	draw_rect(gc, gc->translate.x + x1, gc->translate.y + y1, width, height, line_width, rgb);
}

void gc_set_font(Context *gc, Font *font) {
	gc->font = font;
}

void gc_init(Context* gc, unsigned char *framebuffer, Rect *framebuffer_rect, Font *font) {
	gc->framebuffer = framebuffer;
	gc->framebuffer_rect= *framebuffer_rect;
        gc->bytes_per_pixel=sizeof(Color);
        gc->line_size_in_bytes=sizeof(Color)*RECT_WIDTH(*framebuffer_rect);


        gc->width = RECT_WIDTH(*framebuffer_rect);
        gc->height = RECT_HEIGHT(*framebuffer_rect);

	gc->font = font;


	gc->clip_rect = *framebuffer_rect;
	gc->translate.x = gc->translate.y = 0;
}

void gc_translate(Context* gc, int x, int y) {
	gc->translate.x = x;
	gc->translate.y = y;
}

void gc_clip(Context* gc, Rect rect) {
	IntersectRect(&gc->clip_rect, &gc->clip_rect, &rect);
}

unsigned char* pixel_addr(Context *gc, int x, int y) {
	return gc->framebuffer+(y - gc->framebuffer_rect.top)*gc->line_size_in_bytes+(x - gc->framebuffer_rect.left)*gc->bytes_per_pixel;
}


void fill_rect(Context* gc, int x1, int y1, int width, int height, Color rgb) {
        Rect rect;
	SetRect(&rect, x1, y1, x1+width, y1+height);
        if(!IntersectRect(&rect, &rect, &gc->clip_rect)) return;

	
	for(int y=rect.top; y<rect.bottom; y++) {
		memset(pixel_addr(gc, rect.left, y), rgb, RECT_WIDTH(rect)*gc->bytes_per_pixel);
	}

}

void set_pixel(Context* gc, int x, int y, Color rgb) {
        if(PtInRect(&gc->clip_rect, x, y)) {
		SET_PIXEL(pixel_addr(gc, x, y), rgb);
	}
}

void draw_horizontal_line(Context* gc, int x1, int y1, int x2, Color rgb) {
        if(x1>x2) {
                int tmp=x1;
                x1=x2;
                x2=tmp;
        }

        Rect rect;
	SetRect(&rect, x1, y1, x2, y1+1);
        if(!IntersectRect(&rect, &rect, &gc->clip_rect)) return;

        memset(pixel_addr(gc, rect.left, y1), rgb, RECT_WIDTH(rect)*gc->bytes_per_pixel);

}

void draw_vertical_line(Context* gc, int x1, int y1, int y2, Color rgb) {
        if(y1>y2) {
                int tmp=y1;
                y1=y2;
                y2=tmp;
        }

        Rect rect;
	SetRect(&rect, x1, y1, x1+1, y2);
        if(!IntersectRect(&rect, &rect, &gc->clip_rect)) return;

	for(int y=rect.top; y<rect.bottom; y++) {
		SET_PIXEL(pixel_addr(gc, rect.left, y), rgb);
	}
}


void draw_line(Context* gc, int x1, int y1, int x2, int y2, Color rgb) {

        if(y1==y2) {
                draw_horizontal_line(gc, x1, y1, x2, rgb);
                return;
        } else if(x1==x2) {
                draw_vertical_line(gc, x1, y1, y2, rgb);
                return;
        }

        int dx = x2 - x1;
        int dy = y2 - y1;

        if (abs(dx) < abs(dy)) {
                if (y1 > y2) {
                        int tmp = x1; x1 = x2; x2 = tmp;
                        tmp = y1; y1 = y2; y2 = tmp;
                        dx = -dx; dy = -dy;
                }
                x1 <<= 16;
                dx = (dx << 16) / dy;
                while (y1 <= y2) {
                        int x=x1>>16;
			set_pixel(gc, x, y1, rgb);
                        x1 += dx;
                        y1++;
                }

        } else {
                if (x1 > x2) {
                        int tmp = x1; x1 = x2; x2 = tmp;
                        tmp = y1; y1 = y2; y2 = tmp;
                        dx = -dx; dy = -dy;
                }
                y1 <<= 16;
                dy = dx ? (dy << 16) / dx : 0;
                while (x1 <= x2) {
                        int y=y1>>16;
			set_pixel(gc, x1, y, rgb);
                        y1 += dy;
                        x1++;
                }
        }

}

void draw_text(Context *gc, char *s, int len, int x, int y, Color color) {
	int k=0;
	while(k<len) {
		int c = u8_nextchar(s, &k);
		if(c==0) break;

                pixel_font_symbol symbol=get_pixel_font_symbol(gc->font, c);

		for( int i=0; i< gc->font->height; i++) {
                        for(int j=0; j< symbol.symbol_width; j++) {
                                unsigned char alpha = gc->font->symbols_data[symbol.symbol_index+i*symbol.symbol_width+j];
                                if(alpha == 0) continue;

                                int x1=x+j;
                                int y1=y+i;


				if(PtInRect(&gc->clip_rect, x1, y1)) {
                                	if(alpha == 255) {
						SET_PIXEL(pixel_addr(gc, x1, y1), color);
                                	} else {
						unsigned char* addr=pixel_addr(gc, x1, y1);
                                        	Color result=gc_alpha_blend(color, (Color)*addr, alpha);
                                        	SET_PIXEL(addr, result);
                                	}
				}
                        }
                }

                x += symbol.symbol_width;

        }
}


Color gc_alpha_blend(Color src, Color dst, unsigned int alpha) {

        alpha &= 0xff;
        alpha+=1;
        unsigned int inv_alpha = 256 - alpha;

        unsigned int r1=((src >> 5) & 0x07) << 5;
        unsigned int g1=((src >> 2) & 0x07) << 5;
        unsigned int b1=(src & 0x03) << 6;

        unsigned int r2=((dst >> 5) & 0x07) << 5;
        unsigned int g2=((dst >> 2) & 0x07) << 5;
        unsigned int b2=(dst & 0x03) << 6;


        unsigned int r3=((r1*alpha+r2*inv_alpha) >> 8) >> 5;
        unsigned int g3=((g1*alpha+g2*inv_alpha) >> 8) >> 5;
        unsigned int b3=((b1*alpha+b2*inv_alpha) >> 8) >> 6;

        unsigned int result=r3<<5 | g3<<2 | b3;

        return result;
}


void draw_bitmap(Context* gc, const unsigned char* bitmap, int bitmap_size, int bitmap_x, int bitmap_y, int bitmap_width, int bitmap_height, int dst_x, int dst_y) {
        Rect srcRect=MakeRect(bitmap_x, bitmap_y, bitmap_width, bitmap_height);
        Rect dstRect=MakeRect(dst_x, dst_y, bitmap_width, bitmap_height);

        Rect rect={0};
        if(!IntersectRect(&rect, &dstRect, &gc->clip_rect)) {
                return;
        }

        int x1=rect.left - dstRect.left+ srcRect.left;
        int y1=rect.top - dstRect.top + srcRect.top;
        int x2=rect.right - dstRect.left + srcRect.left-1;
        int y2=rect.bottom - dstRect.top + srcRect.top-1;
        int dstX1=rect.left;
        int dstY1=rect.top;

        int src_line_size = bitmap_width*gc->bytes_per_pixel;
        int dst_line_size = gc->line_size_in_bytes;

        unsigned char* srcAddr=(unsigned char*)bitmap + y1*src_line_size + x1 * gc->bytes_per_pixel;
	unsigned char* dstAddr=pixel_addr(gc, dstX1, dstY1);

        int len =  (x2-x1+1)*gc->bytes_per_pixel;

        for(int y=y1; y<=y2; y++) {
                memmove(dstAddr, srcAddr, len);
                srcAddr += src_line_size;
                dstAddr += dst_line_size;
        }

}

void draw_bitmap_pixels(Context* gc, Bitmap* bitmap, int bitmap_x, int bitmap_y, int bitmap_width, int bitmap_height, int dst_x, int dst_y, PixelOperation op) {
	if(bitmap == NULL) return;

	Rect srcRect=MakeRect(bitmap_x, bitmap_y, bitmap_width, bitmap_height);
        Rect dstRect=MakeRect(dst_x, dst_y, bitmap_width, bitmap_height);

        Rect rect={0};
        if(!IntersectRect(&rect, &dstRect, &gc->clip_rect)) {
                return;
        }

        int x1=rect.left - dstRect.left+ srcRect.left;
        int y1=rect.top - dstRect.top + srcRect.top;
        int x2=rect.right - dstRect.left + srcRect.left-1;
        int y2=rect.bottom - dstRect.top + srcRect.top-1;
        int dstX1=rect.left;
        int dstY1=rect.top;

        int src_line_size = bitmap_width*gc->bytes_per_pixel;
        int dst_line_size = gc->line_size_in_bytes;

        int len =  (x2-x1+1)*gc->bytes_per_pixel;

	unsigned char src_alpha=255;

        for(int y=y1, dstY=dstY1; y<=y2; y++, dstY++) {
		for(int x=x1, dstX=dstX1; x<=x2; x++, dstX++) {
        		unsigned char* srcAddr=(unsigned char*)bitmap->data + y*src_line_size + x * gc->bytes_per_pixel;
        		unsigned char* dstAddr=pixel_addr(gc, dstX, dstY);
			unsigned char dst_pixel=*dstAddr;
			unsigned char src_pixel=*srcAddr;
			if(bitmap->alpha_data) src_alpha = *(bitmap->alpha_data + y*src_line_size + x * gc->bytes_per_pixel);

			switch(op) {
				case SRC_PAINT: {
					switch(src_alpha) {
					case 255:
						SET_PIXEL(dstAddr, src_pixel);
						break;
					case 0:
						SET_PIXEL(dstAddr, dst_pixel);
						break;
					deafult:
                                                SET_PIXEL(dstAddr, gc_alpha_blend(src_pixel, dst_pixel, src_alpha));
						break;
                                        }

				} break;

				case SRC_PAINT_IF_DST0: {
					SET_PIXEL(dstAddr, (dst_pixel == 0 ? src_pixel : dst_pixel) );
				} break;
			}
		}
        }

}


void draw_circle(Context *gc, int xm, int ym, int r, Color color) {
        int x = -r, y = 0, err = 2-2*r; /* II. Quadrant */
        do {
                set_pixel(gc, xm-x, ym+y, color); /*   I. Quadrant */
                set_pixel(gc, xm-y, ym-x, color); /*  II. Quadrant */
                set_pixel(gc, xm+x, ym-y, color); /* III. Quadrant */
                set_pixel(gc, xm+y, ym+x, color); /*  IV. Quadrant */
                r = err;
                if (r >  x) err += ++x*2+1; /* e_xy+e_x > 0 */
                if (r <= y) err += ++y*2+1; /* e_xy+e_y < 0 */
        } while (x < 0);
}

void fill_circle(Context *gc, int xm, int ym, int radius, Color rgb) {
        Rect rect;
	SetRect(&rect, xm-radius, ym - radius, xm-radius+ radius*2, ym - radius+radius*2);

        if(!IntersectRect(&rect, &rect, &gc->clip_rect)) return;

        int r=radius;
        int x = -r, y = 0, err = 2-r*2; /* II. Quadrant */
        do {
                r = err;
                if (r >  x) err += ++x*2+1; /* e_xy+e_x > 0 */
                if (r <= y) {
                        fill_rect(gc, xm+x, ym+y, -2*x, 1, rgb);
                        if(y!=0) fill_rect(gc, xm+x, ym-y, -2*x, 1, rgb);

                        err += ++y*2+1; /* e_xy+e_y < 0 */

                }
        } while (x < 0);

}

void draw_line_from_center(Context *gc, int center_x, int center_y, double angle_degree, double radius, int line_width, Color rgb) {
        double angle_rad=angle_degree*M_PI/180.0;//DEG2RAD(angle_degree);

        if(line_width == 1) {
                draw_line(gc, center_x, center_y,  center_x+lrint(radius*cos(angle_rad)), center_y+lrint(radius*sin(angle_rad)), rgb);
        } else {
                draw_wide_line(gc, center_x, center_y,  center_x+lrint(radius*cos(angle_rad)), center_y+lrint(radius*sin(angle_rad)), line_width, rgb);
                if(line_width>2) fill_circle(gc, center_x+lrint(radius*cos(angle_rad)), center_y+lrint(radius*sin(angle_rad)), line_width/2, rgb);
        }

}

void draw_wide_line(Context* gc, int x1, int y1, int x2, int y2, int width, Color rgb) {

	if((x1 < gc->clip_rect.left && x2<gc->clip_rect.left) || (x1>=gc->clip_rect.right && x2>=gc->clip_rect.right) ||
		(y1 < gc->clip_rect.top && y2<gc->clip_rect.top) || (y1>=gc->clip_rect.bottom && y2>=gc->clip_rect.bottom)) {
			return;
	}

	Point a1={x1,y1};
	Point a2={x2,y2};

	murphy_wideline(gc, a1, a2, width, rgb);

}

void draw_rect(Context *gc, int x1, int y1, int width, int height, int line_width, Color rgb) {
        int x2=x1+width-1;
        int y2=y1+height-1;

	if(line_width>1) {
        	draw_wide_line(gc, x1, y1, x2, y1, line_width, rgb);
        	draw_wide_line(gc, x2, y1, x2, y2, line_width, rgb);
        	draw_wide_line(gc, x2, y2, x1, y2, line_width, rgb);
        	draw_wide_line(gc, x1, y2, x1, y1, line_width, rgb);
	} else {
		draw_line(gc, x1, y1, x2, y1, rgb);
                draw_line(gc, x2, y1, x2, y2, rgb);
                draw_line(gc, x2, y2, x1, y2, rgb);
                draw_line(gc, x1, y2, x1, y1, rgb);
	}
}


BOOL pt_cmp (Point *a, Point *b) {
	return (a->x < b->x || abs(a->x - b->x) ==0  && a->y < b->y) ;
}

void pt_swap(Point *a, Point *b) {
	double t;
	t=a->x;
	a->x=b->x;
	b->x=t;

        t=a->y;
        a->y=b->y;
        b->y=t;
}

int intersectLines(Point a1, Point a2, Point b1, Point b2, Point *c1, Point *c2) {
        //1 if there is one intersection point "c"
        //0 if chunks ar on parallel lines
        //-1 if there are no intersection points


        double d =(a1.x-a2.x)*(b2.y-b1.y) - (a1.y-a2.y)*(b2.x-b1.x);
        double da=(a1.x-b1.x)*(b2.y-b1.y) - (a1.y-b1.y)*(b2.x-b1.x);
        double db=(a1.x-a2.x)*(a1.y-b1.y) - (a1.y-a2.y)*(a1.x-b1.x);

        if (fabs(d)<EPS) {
                if(fabs(da)>EPS || fabs(db)>EPS) return 0;

                Point a={a1.x, a1.y};
                Point b={a2.x, a2.y};
                Point c={b1.x, b1.y};
                Point d={b2.x, b2.y};

                if (pt_cmp(&b, &a))  pt_swap (&a, &b);
                if (pt_cmp(&d, &c))  pt_swap (&c, &d);
	
		if(pt_cmp(&a, &c)) {
			c1->x=c.x;
			c1->y=c.y;
		} else {
			c1->x=a.x;
			c1->y=a.y;
		}

		if(pt_cmp(&b, &d)) {
			c2->x = b.x;
			c2->y = b.y;
		} else {
			c2->x = d.x;
			c2->y = d.y;
		}	

                return 2;
        }

        double ta=da/d;
        double tb=db/d;
        if(0<=ta && ta<=1 && 0<=tb && tb<=1) {
		Point res={a1.x+ta*(a2.x-a1.x), a1.y+ta*(a2.y-a1.y)};
                c1->x=c2->x=res.x;
		c1->y=c2->y=res.y;
                return 1;
        }

        return -1;

}


void murphy_paraline(Context* gc, murphy_info *murphy, Point pt, int d1) {
      int p;                  /* pel counter, p=along line */
      d1 = -d1;

      for (p = 0; p <= murphy->u; p++) {   /* test for end of parallel line */

	    set_pixel(gc, pt.x, pt.y, murphy->color);


            if (d1 <= murphy->kt) {  /* square move */
                  if (murphy->oct2 == 0) {
                        pt.x++;
                  } else {
                        if (murphy->quad4 == 0) {
                              pt.y++;
                        } else {
                              pt.y--;
                        }
                  }
                  d1 += murphy->kv;
            } else {    /* diagonal move */
                  pt.x++;
                  if (murphy->quad4 == 0) {
                        pt.y++;
                  } else {
                        pt.y--;
                  }
                  d1 += murphy->kd;
            }
      }
}

void murphy_wideline(Context *gc, Point p0, Point p1, int width, Color color) { 

	if(p0.x==p1.x && p0.y==p1.y) {
		return;
	}

	double offset = width / 2.;

	Point pt, ptx, ml1, ml2;//, ml1b, ml2b;
	

      int d0, d1;       /* difference terms d0=perpendicular to line, d1=along line */

      int q;                  /* pel counter,q=perpendicular to line */
      int tmp;

      int dd;                 /* distance along line */
      int tk;                 /* thickness threshold */
      double ang;       /* angle for initial point calculation */
      /* Initialisation */

      murphy_info murphy;
      murphy.color=color;
      //murphy.alpha=alpha;

      murphy.u = p1.x - p0.x; /* delta x */
      murphy.v = p1.y - p0.y; /* delta y */

      if (murphy.u < 0) {     /* swap to make sure we are in quadrants 1 or 4 */
            pt = p0;
            p0 = p1;
            p1 = pt;
            murphy.u *= -1;
            murphy.v *= -1;
      }

      if (murphy.v < 0) {     /* swap to 1st quadrant and flag */
            murphy.v *= -1;
            murphy.quad4 = 1;
      } else {
            murphy.quad4 = 0;
      }

      if (murphy.v > murphy.u) {    /* swap things if in 2 octant */
            tmp = murphy.u;
            murphy.u = murphy.v;
            murphy.v = tmp;
            murphy.oct2 = 1;
      } else {
            murphy.oct2 = 0;
      }

      murphy.ku = murphy.u + murphy.u;    /* change in l for square shift */
      murphy.kv = murphy.v + murphy.v;    /* change in d for square shift */
      murphy.kd = murphy.kv - murphy.ku;  /* change in d for diagonal shift */
      murphy.kt = murphy.u - murphy.kv;   /* diag/square decision threshold */

      d0 = 0;
      d1 = 0;
      dd = 0;

      ang = atan((double) murphy.v / (double) murphy.u);    /* calc new initial point - offset both sides of ideal */

      if (murphy.oct2 == 0) {
            pt.x = p0.x + my_lrint(offset * sin(ang));
            if (murphy.quad4 == 0) {
                  pt.y = p0.y - my_lrint(offset * cos(ang));
            } else {
                  pt.y = p0.y + my_lrint(offset * cos(ang));
            }
      } else {
            pt.x = p0.x - my_lrint(offset * cos(ang));
            if (murphy.quad4 == 0) {
                  pt.y = p0.y + my_lrint(offset * sin(ang));
            } else {
                  pt.y = p0.y - my_lrint(offset * sin(ang));
            }
      }

      tk = (int) (4. * HYPOT(pt.x - p0.x, pt.y - p0.y) * HYPOT(murphy.u, murphy.v));      /* used here for constant thickness line */

      ptx = pt;

      for (q = 0; dd <= tk; q++) {  /* outer loop, stepping perpendicular to line */

            murphy_paraline(gc, &murphy, pt, d1);      /* call to inner loop - right edge */
            if (q == 0) {
                  ml1 = pt;
            } else {
                  ml2 = pt;
            }
            if (d0 < murphy.kt) {   /* square move  - M2 */
                  if (murphy.oct2 == 0) {
                        if (murphy.quad4 == 0) {
                              pt.y++;
                        } else {
                              pt.y--;
                        }
                  } else {
                        pt.x++;
                  }
            } else {    /* diagonal move */
                  dd += murphy.kv;
                  d0 -= murphy.ku;
                  if (d1 < murphy.kt) {   /* normal diagonal - M3 */
                        if (murphy.oct2 == 0) {
                              pt.x--;
                              if (murphy.quad4 == 0) {
                                    pt.y++;
                              } else {
                                    pt.y--;
                              }
                        } else {
                              pt.x++;
                              if (murphy.quad4 == 0) {
                                    pt.y--;
                              } else {
                                    pt.y++;
                              }
                        }
                        d1 += murphy.kv;
                  } else {    /* double square move, extra parallel line */
                        if (murphy.oct2 == 0) {
                              pt.x--;
                        } else {
                              if (murphy.quad4 == 0) {
                                    pt.y--;
                              } else {
                                    pt.y++;
                              }
                        }
                        d1 += murphy.kd;
                        if (dd > tk) {
                              return;     /* breakout on the extra line */
                        }
                        murphy_paraline(gc, &murphy, pt, d1);
                        if (murphy.oct2 == 0) {
                              if (murphy.quad4 == 0) {
                                    pt.y++;
                              } else {

                                    pt.y--;
                              }
                        } else {
                              pt.x++;
                        }
                  }
            }
            dd += murphy.ku;
            d0 += murphy.kv;
      }

}

#endif //_GRAPHICS_H_IMPL_

#endif //GRAPHICS_IMPL

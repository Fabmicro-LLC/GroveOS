/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Modified version of NANOSVG

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	

/*
 * Copyright (c) 2013-14 Mikko Mononen memon@inside.org
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * The polygon rasterization is heavily based on stb_truetype rasterizer
 * by Sean Barrett - http://nothings.org/
 *
 */

#ifndef _NSVG_H_
#define _NSVG_H_

typedef struct NSVGfromEdgesRasterizer NSVGfromEdgesRasterizer;

enum NSVGpaintType {
        NSVG_PAINT_NONE = 0,
        NSVG_PAINT_COLOR = 1,
        NSVG_PAINT_LINEAR_GRADIENT = 2,
        NSVG_PAINT_RADIAL_GRADIENT = 3
};

enum NSVGfillRule {
        NSVG_FILLRULE_NONZERO = 0,
        NSVG_FILLRULE_EVENODD = 1
};

enum NSVGflags {
        NSVG_FLAGS_VISIBLE = 0x01
};

typedef struct NSVGgradientStop {
        unsigned int color;
        float offset;
} NSVGgradientStop;

typedef struct NSVGgradient {
        float xform[6];
        char spread;
        float fx, fy;
        int nstops;
        NSVGgradientStop stops[1];
} NSVGgradient;

typedef struct NSVGpaint {
        char type;
        union {
                unsigned int color;
                NSVGgradient* gradient;
        };
} NSVGpaint;

typedef struct NSVGedge {
        float x0,y0, x1,y1;
        int dir;
} NSVGedge;

typedef struct NSVGshape
{
        NSVGpaint fill;                         // Fill paint
        NSVGpaint stroke;                       // Stroke paint
        float opacity;                          // Opacity of the shape.
        float strokeWidth;                      // Stroke width (scaled).
        char fillRule;                          // Fill rule, see NSVGfillRule.
        unsigned char flags;            	// Logical or of NSVG_FLAGS_* flags

        int nedges_fill;
        NSVGedge* edges_fill;

        int nedges_stroke;
        NSVGedge* edges_stroke;
} NSVGshape;

/////////
/*
enum NSVGlineJoin {
        NSVG_JOIN_MITER = 0,
        NSVG_JOIN_ROUND = 1,
        NSVG_JOIN_BEVEL = 2
};


enum NSVGlineCap {
        NSVG_CAP_BUTT = 0,
        NSVG_CAP_ROUND = 1,
        NSVG_CAP_SQUARE = 2
};

typedef struct NSVGpath
{
        float* pts;                                     // Cubic bezier points: x0,y0, [cpx1,cpx1,cpx2,cpy2,x1,y1], ...
        int npts;                                       // Total number of bezier points.
        char closed;                            // Flag indicating if shapes should be treated as closed.
        float bounds[4];                        // Tight bounding box of the shape [minx,miny,maxx,maxy].
        struct NSVGpath* next;          // Pointer to next path, or NULL if last element.
} NSVGpath;


typedef struct NSVGshape
{
        char id[64];                            // Optional 'id' attr of the shape or its group
        NSVGpaint fill;                         // Fill paint
        NSVGpaint stroke;                       // Stroke paint
        float opacity;                          // Opacity of the shape.
        float strokeWidth;                      // Stroke width (scaled).
        float strokeDashOffset;         // Stroke dash offset (scaled).
        float strokeDashArray[8];                       // Stroke dash array (scaled).
        char strokeDashCount;                           // Number of dash values in dash array.
        char strokeLineJoin;            // Stroke join type.
        char strokeLineCap;                     // Stroke cap type.
        float miterLimit;                       // Miter limit
        char fillRule;                          // Fill rule, see NSVGfillRule.
        unsigned char flags;            // Logical or of NSVG_FLAGS_* flags
        float bounds[4];                        // Tight bounding box of the shape [minx,miny,maxx,maxy].
        NSVGpath* paths;                        // Linked list of paths in the image.

        int nedges_fill;
        const NSVGedge* edges_fill;

        int nedges_stroke;
        const NSVGedge* edges_stroke;

        struct NSVGshape* next;         // Pointer to next shape, or NULL if last element.
} NSVGshape;
*/
//////////

typedef struct NSVGimage
{
        float width;                            // Width of the image.
        float height;                           // Height of the image.

	int shapes_len;
        NSVGshape *shapes;               // list of shapes in the image.
	
} NSVGimage;


#endif // _NSVG_H_


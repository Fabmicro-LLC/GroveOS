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

#ifndef NSVG_PAINT_H
#define NSVG_PAINT_H

#include "nsvg.h"
#include "graphics.h"

typedef struct NSVGfromEdgesRasterizer NSVGfromEdgesRasterizer;

// Allocated rasterizer context.
NSVGfromEdgesRasterizer* nsvgCreateFromEdgesRasterizer();

// Deletes rasterizer context.
void nsvgDeleteFromEdgesRasterizer(NSVGfromEdgesRasterizer*);



void nsvgRasterizeFromEdges(NSVGfromEdgesRasterizer* r,
				   NSVGimage* image, float tx, float ty, float scale,
				   unsigned char* dst, int w, int h, int stride, Context* gc);


// Rasterizes SVG image, returns RGBA image (non-premultiplied alpha)
//   r - pointer to rasterizer context
//   image - pointer to image to rasterize
//   tx,ty - image offset (applied after scaling)
//   scale - image scale
//   dst - pointer to destination image data, 4 bytes per pixel (RGBA)
//   w - width of the image to render
//   h - height of the image to render
//   stride - number of bytes per scaleline in the destination buffer


#endif // NSVG_PAINT_H

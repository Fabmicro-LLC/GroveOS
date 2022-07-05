#ifndef _SVGLIB_H_
#define _SVGLIB_H_

#include "nsvg.h"
#include "graphics.h"
#include "wnd_common.h"

WNDCLASS* SVGIMAGE;
void svg_init();
void gc_draw_nsvgimage(Context* gc, NSVGimage* image, int x, int y);


#endif //_SVGLIB_H_

#ifdef SVGLIB_IMPL
#ifndef _SVGLIB_IMPL_H_
#define _SVGLIB_IMPL_H_

#include "wnd.h"

NSVGfromEdgesRasterizer *SVGRAST=NULL;
extern int svgimage_window_proc(WND* hwnd, int msg, int p1, int p2);

#ifdef SVC_CLIENT

#include "svc_uilib.h"

WNDCLASS* SVGIMAGE = NULL;

void svg_init() {
	SVGIMAGE = svc_wnd_get_class("svgimage");
}

#else

#include "nsvg_paint.h"

void svg_init() {
	SVGRAST = nsvgCreateFromEdgesRasterizer();
        WNDCLASS class_svgimage={
                .class_name = "svgimage",
                .wnd_proc = &svgimage_window_proc,
        };
        wnd_register_class(&class_svgimage);
}

void gc_draw_nsvgimage(Context* gc, NSVGimage* image, int x, int y) {
        if(SVGRAST && gc) {
                gc->translate.x+=x;
                gc->translate.y+=y;

                nsvgRasterizeFromEdges(SVGRAST, image, 0, 0, 1, gc->framebuffer, image->width, image->height, RECT_WIDTH(gc->framebuffer_rect), gc);

                gc->translate.x-=x;
                gc->translate.y-=y;
        }
}

#endif //SVC_CLIENT


#endif //_SVGLIB_IMPL_H_
#endif //SVGLIB_IMPL

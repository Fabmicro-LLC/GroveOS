/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "wnd.h"
#include "utils.h"
#include "svglib.h"

int svgimage_window_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_CREATE: {
		WINDOW->wnd_proc(hwnd, msg, p1, p2);
		hwnd->data = NULL;
		wnd_reset_flags(hwnd, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
        } break;

	case WM_DESTROY: {
		if(hwnd->data) {
			hwnd->data = NULL;
		}
		WINDOW->wnd_proc(hwnd, msg, p1, p2);
	} break;

	case WM_SET_IMAGE: {
		hwnd->data = (void*) p1;
	} break;

        case WM_SIZE_THAT_FITS: {
                if(p1==NULL) break;
                Size *size=(Size*) p1;

		NSVGimage* svgimage=(NSVGimage*) hwnd->data;
                if(svgimage) {
                        size->cx = svgimage->width;
                        size->cy = svgimage->height;
                }
        } break;


        case WM_DRAW_RECT: {
                Context * gc=(Context*)p1;
                if(gc == NULL) break;

                if(wnd_get_flags(hwnd, FLAG_FILLBG)) gc_fill_rect(gc, 0, 0, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame), hwnd->bgcolor);

		NSVGimage* svgimage=(NSVGimage*) hwnd->data;
                if(svgimage) gc_draw_nsvgimage(gc, svgimage, 0, 0);
        } break;


        default: {
                return WINDOW->wnd_proc(hwnd, msg, p1, p2);
        } break;

        }//switch

        return 0;
}


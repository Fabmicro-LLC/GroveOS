/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef _WND_H_
#define _WND_H_

#include "wnd_common.h"
#include "wnd_utils.h"

#ifdef SVC_CLIENT

#include "svc.h"
#include "svc_uilib.h"

#endif //SVC_CLIENT

int wnd_init();
void wnd_set_proc(WND* hwnd, WNDPROC addr);
WNDCLASS* wnd_register_class(WNDCLASS *wnd_class);
WNDCLASS* wnd_get_class(const char* class_name);
WND* wnd_create(WNDCLASS* wnd_class);
void wnd_destroy(WND* hwnd);

int wnd_default_proc(WND* hwnd, int msg, int p1, int p2);
int wnd_proc_call(WND* hwnd, int msg, int p1, int p2);
Size wnd_size_that_fits(WND* hwnd);
void wnd_set_flags(WND* hwnd, uint8_t flag_bits);
void wnd_reset_flags(WND* hwnd, uint8_t flag_bits);
uint8_t wnd_get_flags(WND* hwnd, uint8_t flag_bits);
void wnd_set_frame(WND* hwnd, int left, int top, int width, int height);
int wnd_set_text(WND* hwnd, const char* text);
char* wnd_get_text(WND* hwnd, char* buf, unsigned int buf_size);
void wnd_set_font(WND* hwnd, Font* font);
void wnd_set_text_color(WND* hwnd, Color color);
void wnd_set_highlighted_text_color(WND* hwnd, Color color);
void wnd_set_text_alignment(WND* hwnd, TextAlignment alignment);
void wnd_set_image(WND* hwnd, Bitmap* image, int highlighted);
void wnd_add_subview(WND* superview, WND* subview);
void wnd_remove_from_superview(WND* view);
WND* wnd_with_tag(WND* hwnd, int tag);

#endif //_WND_H_

#ifdef WND_IMPL
#ifndef _WND_IMPL_H_
#define _WND_IMPL_H_

int wnd_default_proc(WND* hwnd, int msg, int p1, int p2) {
	hwnd->wnd_class->wnd_proc(hwnd, msg, p1, p2);
}

Size wnd_size_that_fits(WND* hwnd) {
        Size size={0};
        wnd_proc_call(hwnd, WM_SIZE_THAT_FITS, (int)&size, 0);
        return size;
}

void wnd_set_flags(WND* hwnd, uint8_t flag_bits) {
        hwnd->flags |= flag_bits;
}

void wnd_reset_flags(WND* hwnd, uint8_t flag_bits) {
        hwnd->flags &= ~flag_bits;
}

uint8_t wnd_get_flags(WND* hwnd, uint8_t flag_bits) {
        return ((hwnd->flags & flag_bits)!=0);
}

void wnd_set_frame(WND* hwnd, int left, int top, int width, int height) {
        Rect rect=MakeRect(left, top, width, height);
        wnd_proc_call(hwnd, WM_SET_FRAME, (int) &rect, 0);
}

int wnd_set_text(WND* hwnd, const char* text) {
        return wnd_proc_call(hwnd, WM_SET_TEXT, (int) text, 0);
}

char* wnd_get_text(WND* hwnd, char* buf, unsigned int buf_size) {
        return (char*) wnd_proc_call(hwnd, WM_GET_TEXT, (int) buf, buf_size);
}

int wnd_proc_call(WND* hwnd, int msg, int p1, int p2) {
        if(hwnd && hwnd->wnd_proc) {
                return hwnd->wnd_proc(hwnd, msg, p1, p2);
        }
        return 0;
}

void wnd_set_font(WND* hwnd, Font* font) {
	wnd_proc_call(hwnd, WM_SET_FONT, (int) font, 0);
}

void wnd_set_text_color(WND* hwnd, Color color) {
	wnd_proc_call(hwnd, WM_SET_TEXT_COLOR, color, 0);
}

void wnd_set_highlighted_text_color(WND* hwnd, Color color) {
	wnd_proc_call(hwnd, WM_SET_TEXT_COLOR, color, HIGHLIGHTED);
}

void wnd_set_text_alignment(WND* hwnd, TextAlignment alignment) {
	wnd_proc_call(hwnd, WM_SET_TEXT_ALIGNMENT, alignment, 0);
}

void wnd_set_image(WND* hwnd, Bitmap* image, int highlighted) {
	wnd_proc_call(hwnd, WM_SET_IMAGE, (int)image, highlighted);
}

void wnd_add_subview(WND* superview, WND* subview) {
        if(superview == NULL || subview == NULL) return;

        if(subview->superview) {
                wnd_remove_from_superview(subview);
        }

        if(superview->subviews == NULL) {
                superview->subviews = subview;
        } else {
                WND *item=superview->subviews;
                while(item->next_subview) {
                        item = item->next_subview;
                }

                item->next_subview = subview;
        }

        subview->superview = superview;
}

void wnd_remove_from_superview(WND* view) {
        if(view->superview) {
                WND* item=view->superview->subviews;
                if(item == view) {
                        view->superview->subviews= view->next_subview;
                } else {
                        while(item) {
                                if(item->next_subview == view) {
                                        item->next_subview = view->next_subview;
                                        break;
                                }
                                item = item->next_subview;
                        }
                }

                view->next_subview = NULL;
                view->superview = NULL;
        }
}

WND* wnd_with_tag(WND* hwnd, int tag) {
        if(hwnd == NULL) return NULL;

        WND* subview=hwnd->subviews;
        while(subview) {
                if(subview->tag == tag) return subview;
                subview = subview->next_subview;
        }

        subview=hwnd->subviews;
        while(subview) {
                WND* result = wnd_with_tag(subview, tag);
                if(result) return result;
                subview = subview->next_subview;
        }       

        return NULL;

}

#ifdef SVC_CLIENT

int wnd_init() {
	WINDOW = svc_wnd_get_class("window");
        LABEL = svc_wnd_get_class("label");
        IMAGE = svc_wnd_get_class("image");
        BUTTON = svc_wnd_get_class("button");
        SCROLL= svc_wnd_get_class("scroll");
        FORM = svc_wnd_get_class("form");
        MENU_CELL = svc_wnd_get_class("menu_cell");

	font_small = svc_get_font(small);
        font_medium = svc_get_font(medium);
        font_large = svc_get_font(large);
}

void wnd_set_proc(WND* hwnd, WNDPROC addr) {
        hwnd->wnd_proc = addr + svc_get_text();
}

WNDCLASS* wnd_register_class(WNDCLASS *wnd_class) {
	return svc_wnd_register_class(wnd_class);
}
WNDCLASS* wnd_get_class(const char* class_name) {
	return svc_wnd_get_class(class_name);
}
WND* wnd_create(WNDCLASS* wnd_class) {
	return svc_wnd_create(wnd_class);
}
void wnd_destroy(WND* hwnd) {
	svc_wnd_destroy(hwnd);
}

#endif //SVC_CLIENT

#endif //_WND_IMPL_H_
#endif //WND_IMPL

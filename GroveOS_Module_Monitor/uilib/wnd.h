/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#ifndef _WND_H_
#define _WND_H_

#include "wnd_common.h"

#ifndef WND_SVC_CLIENT

int wnd_init();
WNDCLASS* wnd_register_class(WNDCLASS *wnd_class);
WNDCLASS* wnd_get_class(const char* class_name);
WND* wnd_create(WNDCLASS* wnd_class);
void wnd_destroy(WND* hwnd);
void wnd_add_subview(WND* superview, WND* subview);
void wnd_remove_from_superview(WND* view);
WND* wnd_with_tag(WND* hwnd, int tag);
int wnd_draw_text(Context* gc, const char* s, int len, Font *font, Color rgb, Rect* rect, int cursor_pos);
Size wnd_get_text_size(Font* font, char* text);
Point wnd_touch2screen_pt(int touch_x, int touch_y);

#else

#include "svc.h"

inline int wnd_init() {
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

inline void wnd_set_proc(WND* hwnd, WNDPROC addr) {
        hwnd->wnd_proc = addr + svc_get_text();
}

inline WNDCLASS* wnd_register_class(WNDCLASS *wnd_class) {
	return svc_wnd_register_class(wnd_class);
}
inline WNDCLASS* wnd_get_class(const char* class_name) {
	return svc_wnd_get_class(class_name);
}
inline WND* wnd_create(WNDCLASS* wnd_class) {
	return svc_wnd_create(wnd_class);
}
inline void wnd_destroy(WND* hwnd) {
	svc_wnd_destroy(hwnd);
}
inline void wnd_add_subview(WND* superview, WND* subview) {
	svc_wnd_add_subview(superview, subview);
}
inline void wnd_remove_from_superview(WND* hwnd) {
	svc_wnd_remove_from_superview(hwnd);	
}
	
inline WND* wnd_with_tag(WND* hwnd, int tag) {
	return svc_wnd_with_tag(hwnd, tag);
}

inline int wnd_draw_text(Context* gc, const char* s, int len, Font *font, Color rgb, Rect* rect, int cursor_pos) {
	return svc_wnd_draw_text(gc, s, len, font, rgb, rect, cursor_pos);
}
inline Size wnd_get_text_size(Font* font, char* text) {
	return svc_wnd_get_text_size(font, text);
}
inline Point wnd_touch2screen_pt(int touch_x, int touch_y) {
	return svc_wnd_touch2screen_pt(touch_x, touch_y);
}
#endif //WND_SVC_CLIENT

#endif//_WND_H_

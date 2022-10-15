/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#ifndef _WND_COMMON_H_
#define _WND_COMMON_H_

#include "pixel_font.h"
#include "graphics.h"
#include "bitmap.h"
#include <stdlib.h>

//WINDOW UI MESSAGES
#define WM_CREATE                               70
#define WM_LAYOUT                               71
#define WM_DRAW_RECT                            72
#define WM_TOUCH_PRESSED                        73
#define WM_TOUCH_RELEASED                       74
#define WM_DO_DRAW                              75
#define WM_HIT_TEST                             76
#define WM_SIZE_THAT_FITS                       77
#define WM_COMMAND                              78
#define WM_VSCROLL				79
#define WM_DESTROY				80
#define WM_SET_FRAME				81
#define WM_SET_TEXT				82
#define WM_GET_TEXT				83
#define WM_UPDATE				84
#define WM_SET_SEL				85
#define WM_TOUCH_CANCELLED			86

//flags
#define FLAG_HIDDEN				(1<<0)
#define FLAG_TOUCHES_ENABLED			(1<<1)
#define FLAG_FILLBG				(1<<2)
#define FLAG_HIGHLIGHTED			(1<<3)
#define FLAG_OPAQUE				(1<<4)
#define FLAG_CUSTOM				(1<<5)

#define FLAG_DESTROYED				(1<<7)

Font *font_large;
Font *font_medium;
Font *font_small;

typedef struct WND WND;
typedef struct WNDNODE WNDNODE;
typedef int (*WNDPROC)(WND* hwnd, int message, int p1, int p2);

typedef struct {
        char *class_name;
	WNDPROC wnd_proc;
} WNDCLASS;

WNDCLASS* WINDOW;
WNDCLASS* LABEL;
WNDCLASS* IMAGE;
WNDCLASS* BUTTON;
WNDCLASS* SCROLL;
WNDCLASS* FORM;
WNDCLASS* MENU_CELL;

typedef enum {
	left, right, center
} Alignment;

typedef struct {
	Color text_color;
	Color highlighted_text_color;
	Font *font;
	Alignment alignment;
	BOOL selected;
	int cursor;
} LABEL_DATA;

typedef struct {
	Bitmap* bitmap;
	Bitmap* highlighted_bitmap;
} IMAGE_DATA;

typedef struct {
	WND* background;
	WND* icon;
	WND* label;
	Rect margins;
	int inner_space;
	int state;
} BUTTON_DATA;

typedef struct {
	WND* content_view;
	WND* scrollbar;
	WND* up;
	WND* down;
	Point content_offset;
	Size content_size;
	int scroll_step;
} SCROLL_DATA;

typedef struct {
	WND* title;
	WND* view;
	WND* footer;
	WND* ok;
	WND* cancel;
} FORM_DATA;

typedef struct {
	WND* title;
	WND* details;
	BOOL highlighted;
	int param_id;
} MENU_CELL_DATA;
	
struct WND {
	union WNDDATA {
		LABEL_DATA label;
		IMAGE_DATA image;
		BUTTON_DATA button;
		SCROLL_DATA scroll;
		FORM_DATA form;
		MENU_CELL_DATA cell;
	} data;

        Rect frame;

	WNDCLASS* wnd_class;
	WNDPROC wnd_proc;

	//linked list of subviews, first item is stored in subviews, all next item is linked using next_subview
	WND* subviews;
	WND* next_subview;
	WND* superview;

        uint32_t tag;
	void *extra;

	char* text_buf;
	char* text;
        Color bgcolor;
	uint8_t flags;
};

typedef struct {
	WND* view;
	Point pt;
} HIT_TEST_INFO;

inline int wnd_proc_call(WND* hwnd, int msg, int p1, int p2) {
        if(hwnd && hwnd->wnd_proc) {
                return hwnd->wnd_proc(hwnd, msg, p1, p2);
        } 
        return 0;
}

inline Size wnd_size_that_fits(WND* hwnd) {
        Size size={0};
        wnd_proc_call(hwnd, WM_SIZE_THAT_FITS, (int)&size, 0);
        return size;
}

inline void wnd_set_flags(WND* hwnd, uint8_t flag_bits) {
        hwnd->flags |= flag_bits;
}

inline void wnd_reset_flags(WND* hwnd, uint8_t flag_bits) {
        hwnd->flags &= ~flag_bits;
}

inline uint8_t wnd_get_flags(WND* hwnd, uint8_t flag_bits) {
        return ((hwnd->flags & flag_bits)!=0);
}

inline void wnd_set_frame(WND* hwnd, int left, int top, int width, int height) {
        Rect rect=MakeRect(left, top, width, height);
        wnd_proc_call(hwnd, WM_SET_FRAME, (int) &rect, 0);
}

inline int wnd_set_text(WND* hwnd, const char* text) {
        return wnd_proc_call(hwnd, WM_SET_TEXT, (int) text, 0);
}

inline int wnd_get_text(WND* hwnd, char* buf, unsigned int buf_size) {
        return wnd_proc_call(hwnd, WM_GET_TEXT, (int) buf, buf_size);
}

#endif

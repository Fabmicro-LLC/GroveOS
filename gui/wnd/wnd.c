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
#include "msg.h"

#define CLASS_LIST_SIZE (10)
static struct { 
	WNDCLASS data[CLASS_LIST_SIZE];
	int len;
} class_list = {0};


extern Font calibri_large;
extern Font calibri_medium;
extern Font calibri_small;

extern int label_window_proc(WND* hwnd, int msg, int p1, int p2);
extern int image_window_proc(WND* hwnd, int msg, int p1, int p2);
extern int button_window_proc(WND* hwnd, int msg, int p1, int p2);
extern int scroll_window_proc(WND* hwnd, int msg, int p1, int p2);
extern int form_window_proc(WND* hwnd, int msg, int p1, int p2);
extern int menu_cell_window_proc(WND* hwnd, int msg, int p1, int p2);

int window_proc(WND* hwnd, int msg, int p1, int p2);

WNDCLASS* wnd_register_class(WNDCLASS *wnd_class) {
	if(wnd_class == NULL) {
		print("wnd_register_class error, wnd_class is NULL\n");
		return NULL;
	}

	if(class_list.len >= CLASS_LIST_SIZE) {
		print("wnd_register_class error, no space\n");
		return NULL;
	}

	for(int i=0; i<class_list.len; i++) {
		if(strcmp(class_list.data[i].class_name, wnd_class->class_name) == 0) {
			print("wnd_register_class error, already registered class_name=%s\n", wnd_class->class_name);
			return NULL;
		}
	}

	WNDCLASS* result= &class_list.data[class_list.len++];
	*result = *wnd_class;

	return result;
}

WNDCLASS* wnd_get_class(const char* class_name) {
	if(class_name == NULL) {
		print("wnd_get_class class_name is null\n");
		return NULL;
	}

	for(int i=0; i<class_list.len; i++) {
		if(strcmp(class_list.data[i].class_name, class_name) == 0) {
			return &class_list.data[i];
		}
	}

	return NULL;
}

WND* wnd_create(WNDCLASS* wnd_class) {
	if(wnd_class == NULL) {
		print("wnd_create error, wnd_class is null\n");
		return NULL;
	}

	WND* wnd = (WND*) malloc(sizeof(WND));
	if(wnd == NULL) {
		print("wnd_create error, no space available\n");
                return NULL;
        }

	memset(wnd, 0, sizeof(WND));

	wnd->wnd_class = wnd_class;
	wnd->wnd_proc = wnd_class->wnd_proc;

	wnd_proc_call(wnd, WM_CREATE, 0, 0);	

	return wnd;
}

void wnd_destroy(WND* hwnd) {
	if(hwnd == NULL) return;

	if(wnd_get_flags(hwnd, FLAG_DESTROYED) == 1) {
		print ("wnd_destroy failed:: hwnd=%p has been already destroyed\n", hwnd);
		return;
	}

	wnd_proc_call(hwnd, WM_DESTROY, 0, 0);
	wnd_set_flags(hwnd, FLAG_DESTROYED);

	free(hwnd);
}

int wnd_init() {
	font_large = &calibri_large;
	font_medium = &calibri_medium;
	font_small = &calibri_small;
	
	WNDCLASS class_window={
		.class_name = "window",
		.wnd_proc = &window_proc,
	};
	WINDOW = wnd_register_class(&class_window);

	WNDCLASS class_label={
                .class_name = "label",
                .wnd_proc = &label_window_proc,
        };
	LABEL = wnd_register_class(&class_label);	

	WNDCLASS class_image={
                .class_name = "image",
                .wnd_proc = &image_window_proc,
        };
	IMAGE = wnd_register_class(&class_image);

	WNDCLASS class_button={
                .class_name = "button",
                .wnd_proc = &button_window_proc,
        };
        BUTTON = wnd_register_class(&class_button);

	WNDCLASS class_scroll={
                .class_name = "scroll",
                .wnd_proc = &scroll_window_proc,
        };
        SCROLL= wnd_register_class(&class_scroll);	

	WNDCLASS class_form = {
		.class_name = "form",
		.wnd_proc = &form_window_proc,
	};
	FORM = wnd_register_class(&class_form);	


        WNDCLASS class_menu_cell={
                .class_name = "menu_cell",
                .wnd_proc = &menu_cell_window_proc,
        };
        MENU_CELL = wnd_register_class(&class_menu_cell);

	return 0;
}

void wnd_set_proc(WND* hwnd, WNDPROC addr) {
        hwnd->wnd_proc = addr;
}

int window_proc(WND* hwnd, int msg, int p1, int p2) {
	switch(msg) {
	case WM_CREATE: {
		wnd_set_flags(hwnd, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
	} break;

	case WM_DESTROY: {
		wnd_remove_from_superview(hwnd);

		//destroy subviews
                WND* subview=hwnd->subviews;
                while(subview) {
                        WND* next = subview->next_subview;
                        wnd_destroy(subview);
                        subview = next;
                }
        } break;

	case WM_LAYOUT: {
		//print("WINDOW::WM_LAYOUT\n");
	} break;

	case WM_SIZE_THAT_FITS: {
                Size *size=(Size*) p1;
		if(size == NULL) break;
	} break;

	case WM_SET_FRAME: {
		Rect *rect=(Rect*) p1;
		if( rect == NULL) break;
	
		hwnd->frame = *rect;
	} break;

	case WM_DRAW_RECT: {
                Context * gc=(Context*)p1;
                if(gc == NULL) break;
                if(wnd_get_flags(hwnd, FLAG_FILLBG)) gc_fill_rect(gc, 0, 0, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame), hwnd->bgcolor);
	} break;

	case WM_DO_DRAW: {
		//print("WINDOW::WM_DO_DRAW tag=%d\n", hwnd->tag);
		Context * super_gc=(Context*)p1;
		if(super_gc == NULL) break;

		wnd_proc_call(hwnd, WM_LAYOUT, 0, 0);

		Context gc=*super_gc;
		gc_translate(&gc, super_gc->translate.x + hwnd->frame.left, super_gc->translate.y + hwnd->frame.top);
		gc_clip(&gc, MakeRect(super_gc->translate.x + hwnd->frame.left, super_gc->translate.y + hwnd->frame.top, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame)));

		wnd_proc_call(hwnd, WM_DRAW_RECT, (int)&gc, 0);

		WND* start_subview;
		WND* subview;

		start_subview = subview = hwnd->subviews;
		while(subview) {
			if(wnd_get_flags(subview, FLAG_OPAQUE)==1) start_subview = subview;
			subview = subview->next_subview;
		}			

		subview=start_subview;

		while(subview) {
                	Rect subview_frame=subview->frame;
			OffsetRect(&subview_frame, super_gc->translate.x + hwnd->frame.left, super_gc->translate.y + hwnd->frame.top);

			//print("WINDOW::WM_DO_DRAW tag=%d, subview->tag=%d, intersect=%d\n", hwnd->tag, subview->tag, IntersectRect(NULL, &gc.clip_rect, &subview_frame));

                	if(!wnd_get_flags(subview, FLAG_HIDDEN) && IntersectRect(NULL, &gc.clip_rect, &subview_frame) ) {
				wnd_proc_call(subview, WM_DO_DRAW, (int)&gc, 0);
                	}

			subview = subview->next_subview;

        	}

	} break;

	case WM_HIT_TEST: {
		HIT_TEST_INFO *info= (HIT_TEST_INFO*) p1;
		if(info == NULL) break;

		info->view = NULL;

        	if(!PtInRect(&hwnd->frame, info->pt.x, info->pt.y) || wnd_get_flags(hwnd, FLAG_HIDDEN) || ! wnd_get_flags(hwnd, FLAG_TOUCHES_ENABLED)) return NULL;

		WND* subview_list[100];
		int subview_list_size=0;

		WND* subview=hwnd->subviews;

                while(subview && subview_list_size<SIZE(subview_list)) {
			subview_list[subview_list_size] = subview;
			subview_list_size++;
			subview = subview->next_subview;
		}

		info->pt.x  -= hwnd->frame.left;
		info->pt.y  -= hwnd->frame.top;

		for(int i=subview_list_size-1; i>=0; i--) {
			WND* subview=subview_list[i];
			wnd_proc_call(subview, WM_HIT_TEST, (int) info, 0);
                        if(info->view) return (int) info->view;
		}	

		info->view = hwnd;
		return (int)info->view;
	} break;

	case WM_COMMAND: {
		//print("WINDOW::WM_COMMAND cmd=%d, hwnd->tag=%d\n", p1, hwnd->tag);
                if(hwnd->superview) {
                        wnd_proc_call(hwnd->superview, WM_COMMAND,  p1, p2);
                }
        } break;

	case WM_TOUCH_PRESSED: {
		//print("WINDOW::WM_TOUCH_PRESSED hwnd->tag=%d\n", hwnd->tag);
	} break;
	
	case WM_TOUCH_RELEASED: {
		if(hwnd->superview && hwnd->tag) {
                	wnd_proc_call(hwnd->superview, WM_COMMAND,  hwnd->tag, (int)hwnd);
                }
		//print("WINDOW::WM_TOUCH_RELEASED hwnd->tag=%d\n", hwnd->tag);
	} break;

	case WM_TOUCH_CANCELLED: {
                //print("WINDOW::WM_TOUCH_CANCELLED hwnd->tag=%d\n", hwnd->tag);
        } break;



	default:
		print("WINDOW:: unknown message=%d\n", msg);
	}//switch

	return 0;
}



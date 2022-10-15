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

#define CMD_UP 121215
#define CMD_DOWN 9348348

#define SCROLL_W 50
#define SCROLL_H 35

extern Bitmap bitmap_arrow_up_black;
extern Bitmap bitmap_arrow_down_black;

int content_view_window_proc(WND* hwnd, int msg, int p1, int p2);

int scroll_window_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_CREATE: {
                //print("SCROLL::WM_CREATE\n");

		WINDOW->wnd_proc(hwnd, msg, p1, p2);
		hwnd->scroll = malloc(sizeof(SCROLL_DATA));
		memset(hwnd->scroll, 0, sizeof(SCROLL_DATA));		
		
		wnd_reset_flags(hwnd, FLAG_FILLBG);
		hwnd->scroll->scroll_step = 20;

		hwnd->scroll->content_view = wnd_create(WINDOW);
		hwnd->scroll->content_view->bgcolor = WHITE;//GREY_LIGHT2; //WHITE; //GREY_LIGHT;
		hwnd->scroll->content_view->wnd_proc = &content_view_window_proc;
		hwnd->scroll->content_view->extra = hwnd->scroll;
		wnd_add_subview(hwnd, hwnd->scroll->content_view);


		hwnd->scroll->scrollbar= wnd_create(WINDOW);
                hwnd->scroll->scrollbar->bgcolor = GREY_DARK;
                wnd_add_subview(hwnd, hwnd->scroll->scrollbar);


		hwnd->scroll->up = wnd_create(BUTTON);
		hwnd->scroll->up->tag = CMD_UP;
		hwnd->scroll->up->button->icon->image->bitmap=&bitmap_arrow_up_black;
		wnd_add_subview(hwnd->scroll->scrollbar, hwnd->scroll->up);

		hwnd->scroll->down = wnd_create(BUTTON);
		hwnd->scroll->down->tag = CMD_DOWN;
                hwnd->scroll->down->button->icon->image->bitmap=&bitmap_arrow_down_black;
                wnd_add_subview(hwnd->scroll->scrollbar, hwnd->scroll->down);
        } break;

	case WM_DESTROY: {
		if(hwnd->scroll) {
			free(hwnd->scroll);
			hwnd->scroll = NULL;
		}
		WINDOW->wnd_proc(hwnd, msg, p1, p2);

	} break;

	case WM_COMMAND: {
		//print("SCROLL::WM_COMMAND cmd=%d, hwnd->tag=%d\n", p1, hwnd->tag);
                switch(p1) {
		case CMD_DOWN:
			//print("CMD_DOWN::content_offset.y=%d\n", hwnd->scroll->content_offset.y);
			wnd_proc_call(hwnd, WM_VSCROLL, hwnd->scroll->scroll_step, 0);
			break;
		case CMD_UP:
			//print("CMD_UP::content_offset.y=%d\n", hwnd->scroll->content_offset.y);
			wnd_proc_call(hwnd, WM_VSCROLL, -hwnd->scroll->scroll_step, 0);
			break;
		default:
			WINDOW->wnd_proc(hwnd, msg, p1, p2);
                };

	} break;

	case WM_VSCROLL: {
		int dy = p1;
		hwnd->scroll->content_offset.y += dy;
        	if(hwnd->scroll->content_offset.y + RECT_HEIGHT(hwnd->frame) > hwnd->scroll->content_size.cy) {
                	hwnd->scroll->content_offset.y = hwnd->scroll->content_size.cy -  RECT_HEIGHT(hwnd->frame);
        	}
        	if(hwnd->scroll->content_offset.y <0) hwnd->scroll->content_offset.y = 0;
	} break;

	case WM_LAYOUT: {
		Size bounds= MakeSize( RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame));
        	if(bounds.cx <=0 || bounds.cy <= 0) {
			hwnd->scroll->content_view->frame = ZeroRect();
			hwnd->scroll->scrollbar->frame = ZeroRect();
                	hwnd->scroll->up->frame = ZeroRect();
                	hwnd->scroll->down->frame = ZeroRect();
                	return 0;
        	}

		Size scrollbarSize = MakeSize(SCROLL_W, bounds.cy);

		if(hwnd->scroll->content_size.cy <= bounds.cy) {
			wnd_set_flags(hwnd->scroll->scrollbar, FLAG_HIDDEN);
			scrollbarSize= MakeSize(0,0);
		} else {
			wnd_reset_flags(hwnd->scroll->scrollbar, FLAG_HIDDEN);
		}

		hwnd->scroll->content_view->frame = MakeRect(0,0,bounds.cx-scrollbarSize.cx, bounds.cy);
		hwnd->scroll->scrollbar->frame = MakeRect(bounds.cx - scrollbarSize.cx, 0, scrollbarSize.cx, scrollbarSize.cy);

		hwnd->scroll->up->frame = MakeRect(0, 0, SCROLL_W, SCROLL_H);
		hwnd->scroll->down->frame = MakeRect(0, scrollbarSize.cy-SCROLL_H,  SCROLL_W, SCROLL_H);
	} break;

        default: {
                return WINDOW->wnd_proc(hwnd, msg, p1, p2);
        } break;
        }//switch

        return 0;

}


int content_view_window_proc(WND* hwnd, int msg, int p1, int p2) {
	switch(msg) {

        case WM_DO_DRAW: {
                //print("SCROLL::WM_DO_DRAW tag=%d\n", hwnd->tag);
                Context * super_gc=(Context*)p1;
                if(super_gc == NULL) break;

		SCROLL_DATA* scroll_data=(SCROLL_DATA*) hwnd->extra;
		if(scroll_data == NULL) break;
		

                wnd_proc_call(hwnd, WM_LAYOUT, 0, 0);

                Context gc=*super_gc;

                gc_translate(&gc, super_gc->translate.x + hwnd->frame.left, super_gc->translate.y + hwnd->frame.top);
                gc_clip(&gc, MakeRect(super_gc->translate.x + hwnd->frame.left, super_gc->translate.y + hwnd->frame.top, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame)));
                wnd_proc_call(hwnd, WM_DRAW_RECT, (int)&gc, 0);

                gc_translate(&gc, super_gc->translate.x + hwnd->frame.left - scroll_data->content_offset.x, super_gc->translate.y + hwnd->frame.top - scroll_data->content_offset.y);

		WND* start_subview;
                WND* subview;

                start_subview = subview = hwnd->subviews;
                while(subview) {
                        if(wnd_get_flags(subview, FLAG_OPAQUE)==1) start_subview = subview;
                        subview = subview->next_subview;
                }
                subview=start_subview;

                while(subview) {
                        Rect subframe=subview->frame;
                        OffsetRect(&subframe, gc.translate.x, gc.translate.y);

                        //print("SCROLL::WM_DO_DRAW gc.translate.y=%d, subview->tag=%d, subview->frame.top=%d, subframe.top=%d, gc.clip_rect=(%d,%d,%d,%d), super_gc->translate=(%d,%d), hwnd->frame=(%d,%d,%d,%d), intersect=%d\n", gc.translate.y, subview->tag, subview->frame.top, subframe.top, gc.clip_rect.left, gc.clip_rect.top, gc.clip_rect.right, gc.clip_rect.bottom, super_gc->translate.x, super_gc->translate.y, hwnd->frame.left, hwnd->frame.top, hwnd->frame.right, hwnd->frame.bottom, IntersectRect(NULL, &gc.clip_rect, &subframe) );

                        if(!wnd_get_flags(subview, FLAG_HIDDEN) && IntersectRect(NULL, &gc.clip_rect, &subframe) ) {
                                wnd_proc_call(subview, WM_DO_DRAW, (int)&gc, 0);
                        }

                        subview = subview->next_subview;

                }

        } break;

	case WM_HIT_TEST: {
		//print("CONTENT_VIEW::WM_HIT_TEST\n");

		SCROLL_DATA* scroll_data=(SCROLL_DATA*) hwnd->extra;
                if(scroll_data == NULL) break;

                HIT_TEST_INFO *info= (HIT_TEST_INFO*) p1;
		if(info == NULL) break;

		info->view = NULL;

		//print("CONTENT_VIEW::WM_HIT_TEST::2 info->pt=(%d,%d), interaction_enabled=%d\n", info->pt.x, info->pt.y, hwnd->user_interaction_enabled);

                if(!PtInRect(&hwnd->frame, info->pt.x, info->pt.y) || wnd_get_flags(hwnd, FLAG_HIDDEN)|| ! wnd_get_flags(hwnd, FLAG_TOUCHES_ENABLED)) return NULL;


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

                info->pt.x += scroll_data->content_offset.x;
                info->pt.y += scroll_data->content_offset.y;

		//print("CONTENT_VIEW::WM_HIT_TEST::3 info->pt=(%d,%d), subview_list_size=%d\n", info->pt.x, info->pt.y, subview_list_size);

                for(int i=subview_list_size-1; i>=0; i--) {
                        WND* subview=subview_list[i];
			//print("CONTENT_VIEW::WM_HIT_TEST::LOOP i=%d, subview->tag=%d, subview->frame=(%d,%d,%d,%d)\n", i, subview->tag, subview->frame.left, subview->frame.top, subview->frame.right, subview->frame.bottom);
                        wnd_proc_call(subview, WM_HIT_TEST, (int) info, 0);
                        if(info->view) {
				//print("CONTENT_VIEW::WM_HIT_TEST::4\n");
				return (int) info->view;
			}
                }

                info->view = hwnd;
		//print("CONTENT_VIEW::WM_HIT_TEST::5\n");
                return (int)info->view;
        } break;


	
	default:
		return WINDOW->wnd_proc(hwnd, msg, p1, p2);
	}//switch

	return 0;
}


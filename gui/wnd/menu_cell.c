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

int menu_cell_window_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_CREATE: {
                //print("CELL::WM_CREATE\n");

		WINDOW->wnd_proc(hwnd, msg, p1, p2);
		hwnd->cell = malloc(sizeof(MENU_CELL_DATA));
		memset(hwnd->cell, 0, sizeof(MENU_CELL_DATA));

		wnd_reset_flags(hwnd, FLAG_FILLBG);
		wnd_set_flags(hwnd, FLAG_TOUCHES_ENABLED);

		hwnd->cell->title = wnd_create(LABEL);
		wnd_set_text(hwnd->cell->title, "");
                hwnd->cell->title->label->font  = font_large;
		hwnd->cell->title->label->text_color = BLACK;
                wnd_add_subview(hwnd, hwnd->cell->title);

		hwnd->cell->details = wnd_create(LABEL);
		wnd_set_text(hwnd->cell->details, "");
                hwnd->cell->details->label->font  = font_large;
                hwnd->cell->details->label->text_color = BLACK;
                wnd_add_subview(hwnd, hwnd->cell->details);
        } break;

	case WM_DESTROY: {
		if(hwnd->cell) {
			free(hwnd->cell);
			hwnd->cell = NULL;
		}
		WINDOW->wnd_proc(hwnd, msg, p1, p2);
	} break;


	case WM_LAYOUT: {
		Size bounds= MakeSize( RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame));
        	if(bounds.cx <=0 || bounds.cy <= 0) {
			hwnd->cell->title->frame = ZeroRect();
			hwnd->cell->details->frame = ZeroRect();
                	return 0;
        	}

		Size titleSize = wnd_size_that_fits(hwnd->cell->title);
		Size detailsSize = wnd_size_that_fits(hwnd->cell->details);

		if(titleSize.cx + detailsSize.cx>bounds.cx) {
			if(detailsSize.cx>100) {
				detailsSize.cx = 100;
				hwnd->cell->details->label->alignment = left;	
			} else {
				hwnd->cell->details->label->alignment = right;
			}

			if(titleSize.cx > bounds.cx - detailsSize.cx - 10) {
				titleSize.cx = bounds.cx - detailsSize.cx - 10;
			}
		} else {
			hwnd->cell->details->label->alignment = right;
		}

		hwnd->cell->title->frame = MakeRect(0,(bounds.cy - titleSize.cy)/2, titleSize.cx, titleSize.cy);
		hwnd->cell->details->frame = MakeRect(bounds.cx - detailsSize.cx,(bounds.cy - detailsSize.cy)/2, detailsSize.cx, detailsSize.cy);

	} break;

	case WM_DRAW_RECT: {
		if(hwnd->cell->highlighted) {
			hwnd->cell->title->label->text_color = WHITE;
			hwnd->cell->details->label->text_color = WHITE;
			wnd_set_flags(hwnd, FLAG_FILLBG);
			hwnd->bgcolor = BLUE;
		} else {
			hwnd->cell->title->label->text_color = BLACK;
			hwnd->cell->details->label->text_color = BLACK;
			wnd_reset_flags(hwnd, FLAG_FILLBG);
		}

		WINDOW->wnd_proc(hwnd, msg, p1, p2);
	} break;

	case WM_TOUCH_PRESSED: {
		hwnd->cell->highlighted = 1;
        } break;

        case WM_TOUCH_RELEASED: {
		hwnd->cell->highlighted = 0;
                if(hwnd->superview) {
                        if(hwnd->tag) {
                                wnd_proc_call(hwnd->superview, WM_COMMAND,  hwnd->tag, (int) hwnd);
                        }
                }
        } break;

	case WM_TOUCH_CANCELLED: {
		hwnd->cell->highlighted = 0;
	} break;

        default: {
                return WINDOW->wnd_proc(hwnd, msg, p1, p2);
        } break;
        }//switch

        return 0;

}

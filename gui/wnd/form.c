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

#define CMD_OK	3428341 
#define CMD_CANCEL 343843

int form_window_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_CREATE: {
                //print("FORM::WM_CREATE\n");
		WINDOW->wnd_proc(hwnd, msg, p1, p2);
		hwnd->form = malloc(sizeof(FORM_DATA));	
		memset(hwnd->form, 0, sizeof(FORM_DATA));

		wnd_set_flags(hwnd, FLAG_OPAQUE);

		hwnd->form->title = wnd_create(LABEL);
                hwnd->form->title->label->font  = font_large;
		hwnd->form->title->label->text_color = WHITE;
		hwnd->form->title->label->alignment = center;
		hwnd->form->title->bgcolor=GREY_DARK;
		wnd_set_flags(hwnd->form->title, FLAG_FILLBG);
                wnd_add_subview(hwnd, hwnd->form->title);

		hwnd->form->view= wnd_create(SCROLL);
		wnd_add_subview(hwnd, hwnd->form->view);

		hwnd->form->footer= wnd_create(WINDOW);
		hwnd->form->footer->bgcolor = GREY_DARK;
                wnd_add_subview(hwnd, hwnd->form->footer);

		hwnd->form->ok= wnd_create(BUTTON);
		hwnd->form->ok->tag = CMD_OK;
		wnd_set_text(hwnd->form->ok, "ОК");
		wnd_add_subview(hwnd->form->footer, hwnd->form->ok);

		hwnd->form->cancel = wnd_create(BUTTON);
		hwnd->form->cancel->tag = CMD_CANCEL;
                wnd_set_text(hwnd->form->cancel, "Отмена");
                wnd_add_subview(hwnd->form->footer, hwnd->form->cancel);
        } break;

	case WM_DESTROY: {
		if(hwnd->form) {
			free(hwnd->form);
			hwnd->form = NULL;
		}
		WINDOW->wnd_proc(hwnd, msg, p1, p2);
	} break;

	case WM_SET_TEXT: {
		wnd_proc_call(hwnd->form->title, msg, p1, p2);
        } break;

	case WM_GET_TEXT: {
                return wnd_proc_call(hwnd->form->title, msg, p1, p2);
        } break;

	case WM_COMMAND: {
		print("FORM::WM_COMMAND cmd=%d\n", p1);
		switch(p1) {
		case CMD_OK:
		case CMD_CANCEL:
			print("FORM::WM_COMMAND wnd_destroy\n");
			wnd_destroy(hwnd);
			return 0;
		}
	} break;


	case WM_LAYOUT: {
		Size bounds= MakeSize( RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame));
        	if(bounds.cx <=0 || bounds.cy <= 0) {
			hwnd->form->title->frame = ZeroRect();
			hwnd->form->view->frame = ZeroRect();
			hwnd->form->footer->frame = ZeroRect();
                	hwnd->form->ok->frame = ZeroRect();
                	hwnd->form->cancel->frame = ZeroRect();
                	return 0;
        	}

		Size titleSize = wnd_size_that_fits(hwnd->form->title);
		Size cancelSize = wnd_size_that_fits(hwnd->form->cancel);
		Size okSize = wnd_size_that_fits(hwnd->form->ok);
		Size footerSize  = MakeSize(bounds.cx, 50);

		if(titleSize.cy < 40) titleSize.cy = 40;

		if(cancelSize.cy < 80) cancelSize.cx  = 80;
		cancelSize.cy = 40;

		if(okSize.cx < 80) okSize.cx  = 80;
		okSize.cy = 40;

		if(wnd_get_flags(hwnd->form->footer, FLAG_HIDDEN)) {
			footerSize = MakeSize(0,0);
			hwnd->form->ok->frame = ZeroRect();
                        hwnd->form->cancel->frame = ZeroRect();
		}

		hwnd->form->title->frame = MakeRect(0,0, bounds.cx, titleSize.cy);
                hwnd->form->view->frame = MakeRect(0, titleSize.cy, bounds.cx, bounds.cy - titleSize.cy - footerSize.cy);
		hwnd->form->footer->frame = MakeRect(0, bounds.cy - footerSize.cy, footerSize.cx, footerSize.cy);

                hwnd->form->cancel->frame = MakeRect(10, (footerSize.cy - cancelSize.cy)/2, cancelSize.cx, cancelSize.cy);
                hwnd->form->ok->frame = MakeRect(footerSize.cx - okSize.cx-10, (footerSize.cy - okSize.cy)/2, okSize.cx, okSize.cy);

	} break;

        default: {
                return WINDOW->wnd_proc(hwnd, msg, p1, p2);
        } break;
        }//switch

        return 0;

}

/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "wnd.h"
#include "utf8.h"
#include "utils.h"

#define TEXT_BLOCK_SIZE (1)

int label_window_proc(WND* hwnd, int msg, int p1, int p2) {
	switch(msg) {

        case WM_CREATE: {
		//print("LABEL::WM_CREATE\r\n");
		WINDOW->wnd_proc(hwnd, msg, p1, p2);
		hwnd->label = malloc(sizeof(LABEL_DATA));
		memset(hwnd->label, 0, sizeof(LABEL_DATA));

		hwnd->label->text_color = 0x0;
        	hwnd->label->font = font_medium;
		hwnd->label->cursor = -1;
		wnd_reset_flags(hwnd, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
        } break;

	case WM_DESTROY: {
		if(hwnd->label) {
			if(hwnd->label->text_buf) {
                		free(hwnd->label->text_buf);
                		hwnd->label->text_buf = NULL;
        		}

			free(hwnd->label);
			hwnd->label = NULL;
		}
		WINDOW->wnd_proc(hwnd, msg, p1, p2);
	} break;

	case WM_SET_FONT: {
                hwnd->label->font = (Font*) p1;
        } break;

	case WM_SET_TEXT_ALIGNMENT: {
                hwnd->label->alignment = (TextAlignment) p1;
        } break;

        case WM_SET_TEXT_COLOR: {
		switch(p2) {
                case HIGHLIGHTED:
			hwnd->label->highlighted_text_color = (Color) p1;
                        break;
                case NORMAL:
                	hwnd->label->text_color = (Color) p1;
                        break;
                }
        } break;

	case WM_SIZE_THAT_FITS: {
		if(p1==NULL) break;
		Size *size=(Size*) p1;

		Rect bounds=MakeRect(0,0,size->cx, size->cy);
		gc_draw_wnd_text(NULL, hwnd->label->text_buf, strlen(hwnd->label->text_buf), hwnd->label->font, hwnd->label->text_color, &bounds, 0, 0);
		size->cx = RECT_WIDTH(bounds);
		size->cy = RECT_HEIGHT(bounds);
        } break;


	case WM_DRAW_RECT: {
                Context * gc=(Context*)p1;
                if(gc == NULL) break;


		Size bounds=MakeSize(RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame));
		Rect textRect=MakeRect(0, 0, bounds.cx, bounds.cy);

		Size textSize=wnd_size_that_fits(hwnd);

		if(hwnd->label->alignment == center)  {
			textRect = MakeRect((bounds.cx - textSize.cx)/2, 0, textSize.cx, bounds.cy);
		} else if(hwnd->label->alignment == right) {
                        textRect = MakeRect(bounds.cx - textSize.cx, 0, textSize.cx, bounds.cy);
		}

		if(hwnd->label->selected) {
			gc_fill_rect(gc, 0, 0, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame), BLUE);
			gc_draw_wnd_text(gc, hwnd->label->text_buf, strlen(hwnd->label->text_buf), hwnd->label->font, WHITE, &textRect, hwnd->label->cursor+1,  hwnd->label->alignment);
		} else {
			if(wnd_get_flags(hwnd, FLAG_FILLBG)) gc_fill_rect(gc, 0, 0, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame), hwnd->bgcolor);
			Color color = (wnd_get_flags(hwnd, FLAG_HIGHLIGHTED) ? hwnd->label->highlighted_text_color : hwnd->label->text_color);

			gc_draw_wnd_text(gc, hwnd->label->text_buf, strlen(hwnd->label->text_buf), hwnd->label->font, color, &textRect, hwnd->label->cursor+1,  hwnd->label->alignment);
		}

        } break;

	case WM_SET_SEL: {
		hwnd->label->selected = (p1 != 0);
	} break;

	case WM_SET_TEXT: {
		hwnd->label->selected = 0;

		char* s = (char*) p1;
                if( s == NULL) break;
                int slen = strlen(s);

		if(slen+1 > hwnd->label->text_buf_size) {
			hwnd->label->text_buf_size = slen+1;
			if(hwnd->label->text_buf_size < TEXT_BLOCK_SIZE) hwnd->label->text_buf_size = TEXT_BLOCK_SIZE;

			if(hwnd->label->text_buf) free(hwnd->label->text_buf);
			hwnd->label->text_buf  = (char*) malloc(hwnd->label->text_buf_size);

			if(hwnd->label->text_buf == NULL) {
				print("LABEL::WM_SET_TEXT out of memory\r\n");
				break;
			}
		}

                strncpy(hwnd->label->text_buf, s, slen);
                hwnd->label->text_buf[slen] = 0;

	} break;

	case WM_GET_TEXT: {
                char* buf = (char*) p1;
                int buf_size = p2;

                if(buf == NULL || buf_size<=0) return (int) hwnd->label->text_buf;

                if(hwnd->label->text_buf == NULL) {
                        buf[0] = 0;
                        return (int) hwnd->label->text_buf;
                }

                int textlen = strlen(hwnd->label->text_buf);
                if(textlen>buf_size-1) textlen = buf_size-1;


                strncpy(buf, hwnd->label->text_buf, textlen);
                buf[textlen] = 0;


                return (int) hwnd->label->text_buf;
        } break;


	default: {
		return WINDOW->wnd_proc(hwnd, msg, p1, p2);
	} break;
	}

	return 0;
}


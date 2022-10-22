/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "uilib.h"
#include "utf8.h"
#include <string.h>
#include "lcd-ft800.h"
#include "utils.h"
#include <stdio.h>


#define TAG_INPUT_VTEXT         384832421

#define CMD_BACK		4
#define CMD_OK			8

#define CMD_KEY_ARROWUP		812
#define CMD_KEY_ARROWDOWN 	813

extern WND* SCREEN;
extern Bitmap bitmap_arrow_up_black;
extern Bitmap bitmap_arrow_down_black;

static INPUT_TEXT_PARAM INPUT_PARAM;
static char** LIST;
static int LIST_SIZE;
static int LIST_INDEX;

static int select_list_wnd_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_COMMAND: {
                int cmd=p1;
                WND* cmdWnd = (WND*)p2;

                switch(cmd) {
		case CMD_KEY_ARROWUP: {
			if(++LIST_INDEX>=LIST_SIZE) LIST_INDEX = 0; //LIST_SIZE-1;
			print("select_list_wnd_proc::CMD_KEY_ARROWUP index=%d, size=%d\r\n", LIST_INDEX, LIST_SIZE);
			wnd_set_text(wnd_with_tag(hwnd, TAG_INPUT_VTEXT), LIST[LIST_INDEX]);
		} break;


		case CMD_KEY_ARROWDOWN: {
                        if(--LIST_INDEX<0) LIST_INDEX = LIST_SIZE-1; //0;
			print("select_list_wnd_proc::CMD_KEY_ARROWDOWN index=%d, size=%d\r\n", LIST_INDEX, LIST_SIZE);
                        wnd_set_text(wnd_with_tag(hwnd, TAG_INPUT_VTEXT), LIST[LIST_INDEX]);
                } break;


		case CMD_OK: {
                        print("select_list_wnd_proc::CMD_OK::0\r\n");
			sprintf(INPUT_PARAM.text, "%d", LIST_INDEX);
			wnd_proc_call(INPUT_PARAM.hwnd, INPUT_PARAM.msg, INPUT_PARAM.param_id, (int)INPUT_PARAM.text);
                        wnd_destroy(hwnd);

                        print("select_list_wnd_proc::CMD_OK::end \n");

                        return 0;

                } break;

		case CMD_BACK: {
                        wnd_destroy(hwnd);
                } break;

                default:
                        wnd_default_proc(hwnd, msg, p1, p2);
                };//switch

        } break;

        default:
                wnd_default_proc(hwnd, msg, p1, p2);
        }//
}


WND* show_select_list(INPUT_TEXT_PARAM *param, char** list, unsigned int list_len, unsigned int selected_index) {
        print("show_select_list list_len=%d\r\n", list_len);

	INPUT_PARAM = *param;
	LIST = list;
	LIST_SIZE = list_len;
	LIST_INDEX = selected_index;

        WND* hwnd=wnd_create(FORM);
        wnd_set_frame(hwnd, 0, 0, LCD_WIDTH, LCD_HEIGHT);
        wnd_set_proc(hwnd, &select_list_wnd_proc);
        wnd_set_text(hwnd, param->title);
        wnd_set_text(hwnd->form->ok, "Сохранить");
        hwnd->form->ok->tag = CMD_OK;
        wnd_set_text(hwnd->form->cancel, "Назад");
	hwnd->form->cancel->tag = CMD_BACK;
        wnd_add_subview(SCREEN, hwnd);

        WND* label = wnd_create(LABEL);
        wnd_set_font(label, font_large);
        wnd_set_text(label, "Значение: ");
        wnd_add_subview(hwnd, label);

        WND* vtext= wnd_create(LABEL);
        vtext->tag = TAG_INPUT_VTEXT;
        wnd_set_font(vtext, font_large);
        wnd_set_text(vtext, LIST[LIST_INDEX]);
        wnd_add_subview(hwnd,vtext);

        Size labelSize = wnd_size_that_fits(label);
        wnd_set_frame(label, 20,90, labelSize.cx, labelSize.cy);
        wnd_set_frame(vtext, label->frame.right, 90, LCD_WIDTH - label->frame.right - 20 - 60, labelSize.cy);

        WND* btn;

        btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_ARROWUP;
	wnd_set_image(btn, &bitmap_arrow_up_black, 0);
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, LCD_WIDTH - 20 -60, vtext->frame.top+labelSize.cy/2-40, 60, 40);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_ARROWDOWN;
	wnd_set_image(btn, &bitmap_arrow_down_black, 0);
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, LCD_WIDTH - 20 -60, vtext->frame.top+labelSize.cy/2, 60, 40);

	return hwnd;
}

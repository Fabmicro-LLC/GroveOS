/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "uilib.h"

#define CMD_ALERT_CANCEL 32489823

extern WND* SCREEN;

int alert_wnd_proc(WND* hwnd, int msg, int p1, int p2) {
        //print("main_wnd_proc msg=%d\n",msg);
        switch(msg) {

        case WM_DRAW_RECT: {
                Context * gc=(Context*)p1;
                if(gc == NULL) break;

                wnd_default_proc(hwnd, msg, p1, p2);
                gc_draw_rect(gc, 0, 0, RECT_WIDTH(hwnd->frame), RECT_HEIGHT(hwnd->frame), 2, WHITE);
        } break;


        case WM_COMMAND: {
                int cmd=p1;

                switch(cmd) {
                case CMD_ALERT_CANCEL: {
                        wnd_destroy(hwnd);
                        return 0;
                } break;

                default:
                        wnd_default_proc(hwnd, msg, p1, p2);

                }
        }

        default:
                wnd_default_proc(hwnd, msg, p1, p2);
        }//

}

WND* show_alert(const char* text) {
        WND* hwnd=wnd_create(WINDOW);
        wnd_set_proc(hwnd, &alert_wnd_proc);
        hwnd->bgcolor = COLOR332(191, 0, 0);
        Size frmSize = MakeSize(320, 120);
        wnd_set_frame(hwnd, (RECT_WIDTH(SCREEN->frame)- frmSize.cx)/2, (RECT_HEIGHT(SCREEN->frame)-frmSize.cy)/2, frmSize.cx, frmSize.cy);
        wnd_add_subview(SCREEN, hwnd);

        WND* btn=wnd_create(BUTTON);
        btn->tag = CMD_ALERT_CANCEL;
        wnd_set_text(btn, "Отмена");
        wnd_set_text_color(btn, WHITE);
        Size btnSize= wnd_size_that_fits(btn);
        if(btnSize.cx<80) btnSize.cx=80;
        if(btnSize.cy<30) btnSize.cy=30;
        wnd_set_frame(btn, (frmSize.cx - btnSize.cx)/2, (frmSize.cy -btnSize.cy) -10, btnSize.cx, btnSize.cy);
        wnd_add_subview(hwnd, btn);

        WND* label = wnd_create(LABEL);
        wnd_set_text(label, text);
        wnd_set_font(label, font_large);
        wnd_set_text_color(label, WHITE);
        wnd_set_text_alignment(label, center);
        Size labelSize=wnd_size_that_fits(label);
        if(labelSize.cx>frmSize.cx-10) labelSize.cx = frmSize.cx;
        if(labelSize.cy>frmSize.cy-10-10) labelSize.cy = frmSize.cy-10-10;
        wnd_set_frame(label, (frmSize.cx - labelSize.cx)/2, 5, labelSize.cx, labelSize.cy);
        wnd_add_subview(hwnd, label);
	return hwnd;
}


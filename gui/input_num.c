/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
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

#define TAG_INPUT_VAL 		374278472

#define CMD_BACK		4
#define CMD_OK			8

#define CMD_KEY0		800
#define CMD_KEY1		801
#define CMD_KEY2		802
#define CMD_KEY3		803
#define CMD_KEY4		804
#define CMD_KEY5		805
#define CMD_KEY6		806
#define CMD_KEY7		807
#define CMD_KEY8		808
#define CMD_KEY9		809
#define CMD_KEY_DEL		810
#define CMD_KEY_DOT		811
#define CMD_KEY_ARROWUP		812
#define CMD_KEY_ARROWDOWN 	813
#define CMD_KEY_MINUS		814

extern WND* SCREEN;
extern Bitmap bitmap_arrow_up_black;
extern Bitmap bitmap_arrow_down_black;

static INPUT_TEXT_PARAM INPUT_PARAM;
static char FORMAT[16];

static int input_num_wnd_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_COMMAND: {
                int cmd=p1;
		WND* cmdWnd = (WND*)p2;

                print("input_num_wnd_proc::WM_COMMAND cmd=%d, cmdWnd=%p\n", cmd, cmdWnd);
                
                switch(cmd) {
		case CMD_KEY_ARROWUP: {
                        print("input_num_wnd_proc::CMD_KEY_ARROWUP\n");
                        WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        char val_text[64];
                        wnd_get_text(val, val_text, SIZE(val_text));
                        float fval = atof(val_text);

                        fval += INPUT_PARAM.step;
			if(fval<INPUT_PARAM.lower_limit) fval = INPUT_PARAM.lower_limit;
                        if(fval>INPUT_PARAM.upper_limit) fval = INPUT_PARAM.upper_limit;
                        snprintf(val_text, SIZE(val_text), FORMAT, fval);
                        wnd_set_text(val,val_text);
			val->label->selected = TRUE;
                } break;


                case CMD_KEY_ARROWDOWN: {
                        print("input_num_wnd_proc::CMD_KEY_ARROWDOWN\n");
                        WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        char val_text[64];
                        wnd_get_text(val, val_text, SIZE(val_text));
                        float fval = atof(val_text);

                        fval -= INPUT_PARAM.step;
                        if(fval<INPUT_PARAM.lower_limit) fval = INPUT_PARAM.lower_limit;
			if(fval>INPUT_PARAM.upper_limit) fval = INPUT_PARAM.upper_limit;
                        snprintf(val_text, SIZE(val_text), FORMAT, fval);
                        wnd_set_text(val,val_text);
			val->label->selected = TRUE;
                } break;

		case CMD_KEY0 ... CMD_KEY9: {
			char key_text[2];
			wnd_get_text(cmdWnd, key_text, SIZE(key_text));
			print("input_num_wnd_proc::WM_COMMAND key=%c pressed\n", key_text[0]);

			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
			char val_text[64];

			if(val->label->selected) {
				val_text[0]=0;
			} else {
				wnd_get_text(val, val_text, SIZE(val_text));
			}
			print("input_num_wnd_proc::WM_COMMAND cmd=%d, selected=%d, val_text=%s\n", cmd, val->label->selected, val_text);
			if(strlen(val_text)<19) strcat(val_text, key_text);
			print("input_num_wnd_proc::WM_COMMAND val_new_text=%s\n", val_text);
			
			wnd_set_text(val, val_text);
			val->label->selected = FALSE;
		} break;


		case CMD_KEY_DEL: {
			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        char val_text[64];

			if(val->label->selected) {
                                val_text[0]=0;
                        } else {
                        	wnd_get_text(val, val_text, SIZE(val_text));
			}
                        print("input_num_wnd_proc::CMD_KEY_DEL selected=%d, val_text=%s\n", val->label->selected, val_text);
			int len = strlen(val_text);
			if(len>0) val_text[len-1] = 0;
                        print("input_num_wnd_proc::CMD_KEY_DEL val_new_text=%s\n", val_text);
                        wnd_set_text(val, val_text);
			val->label->selected = FALSE;
		} break;


		case CMD_KEY_DOT: {
			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        char val_text[64];

                        wnd_get_text(val, val_text, SIZE(val_text));
                        print("input_num_wnd_proc::CMD_KEY_DOT val_text=%s\n", val_text);
			if(strlen(val_text)>0 && strchr(val_text, '.') == NULL) {
				if(strlen(val_text)<19) strcat(val_text, ".");
			}
                        print("input_num_wnd_proc::CMD_KEY_DOT val_new_text=%s\n", val_text);
                        wnd_set_text(val, val_text);
		} break;

		case CMD_KEY_MINUS: {
                        WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        char val_text[64];

                        wnd_get_text(val, val_text, SIZE(val_text));
                        print("input_num_wnd_proc::CMD_KEY_MINUS val_text=%s\n", val_text);
			float fval = atof(val_text);
                        snprintf(val_text, SIZE(val_text), FORMAT, -fval);

                        print("input_num_wnd_proc::CMD_KEY_MINUS val_new_text=%s\n", val_text);

			//set_param(INPUT_PARAM.param_id, val_text);
                        wnd_set_text(val, val_text);
                } break;


		case CMD_OK: {
			print("input_num_wnd_proc::CMD_OK::0\r\n");
			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        wnd_get_text(val, INPUT_PARAM.text, sizeof(INPUT_PARAM.text));

			print("input_num_wnd_proc::CMD_OK::2\r\n");
			float fval = atof(INPUT_PARAM.text);
                        if(fval<INPUT_PARAM.lower_limit) fval = INPUT_PARAM.lower_limit;
                        if(fval>INPUT_PARAM.upper_limit) fval = INPUT_PARAM.upper_limit;
                        snprintf(INPUT_PARAM.text, sizeof(INPUT_PARAM.text), FORMAT, fval);


                        wnd_proc_call(INPUT_PARAM.hwnd, INPUT_PARAM.msg, INPUT_PARAM.param_id, (int)INPUT_PARAM.text);
                        wnd_destroy(hwnd);

                        print("input_num_wnd_proc::CMD_OK::end \n");
			DelayLoopMicro(200);

                        return 0;

		} break;

		case CMD_BACK: {
			print("input_num_wnd_proc::CMD_BACK \r\n");
                        //wnd_proc_call(hwnd->superview, WM_UPDATE, 0, 0);
                        wnd_destroy(hwnd);

			return 0;

                } break;
		
		default:
			wnd_default_proc(hwnd, msg, p1, p2);
                };//switch

        } break;

        default:
                wnd_default_proc(hwnd, msg, p1, p2);
        }//
}


WND* show_input_num_format(INPUT_TEXT_PARAM *param, char* format) {
	print("show_input_num\r\n");

	INPUT_PARAM = *param;
	snprintf(FORMAT, sizeof(FORMAT), "%s", format);

	print("show_input_num::2 FORMAT=%s, format=%s\r\n",FORMAT,format);

        WND* hwnd=wnd_create(FORM);

        wnd_set_frame(hwnd, 0, 0, LCD_WIDTH, LCD_HEIGHT);
        wnd_set_proc(hwnd, &input_num_wnd_proc);
        wnd_set_text(hwnd, param->title);
	//wnd_set_text(hwnd->form->ok, "Сохранить");
	if(param->ok_text) wnd_set_text(hwnd->form->ok, param->ok_text);
	hwnd->form->ok->tag = CMD_OK;
        //wnd_set_text(hwnd->form->cancel, "Назад");
	if(param->cancel_text) wnd_set_text(hwnd->form->cancel, param->cancel_text);
	hwnd->form->cancel->tag = CMD_BACK;
        wnd_add_subview(SCREEN, hwnd);

	WND* label = wnd_create(LABEL);
        wnd_set_font(label, font_large);
	wnd_set_text(label, "Значение: ");
        wnd_add_subview(hwnd, label);

	WND* valuebg = wnd_create(WINDOW);
	valuebg->bgcolor = WHITE;
	wnd_add_subview(hwnd,valuebg);

	char buf[64];
	WND* value= wnd_create(LABEL);
	value->tag = TAG_INPUT_VAL;
        wnd_set_font(value, font_large);
	wnd_set_text_alignment(value, right);
        //wnd_set_text(value, get_param(param->param_id, buf, SIZE(buf)));
	wnd_set_text(value, param->text);
	value->label->selected = TRUE;
        wnd_add_subview(hwnd,value);

	Size labelSize = wnd_size_that_fits(label);
                
	wnd_set_frame(label, LCD_WIDTH/2-labelSize.cx,60, labelSize.cx, labelSize.cy);
	wnd_set_frame(valuebg, label->frame.right, 55, 140+10, labelSize.cy+10);
        wnd_set_frame(value, label->frame.right+5, 60, 140, labelSize.cy);
	
	Size btnSize = MakeSize(60, 40);
	int bw = btnSize.cx + 0;
	int dx = (LCD_WIDTH - bw*7)/2;
	int dy1 = LCD_HEIGHT - 60 - btnSize.cy - btnSize.cy;
	int dy2 = LCD_HEIGHT - 60 - btnSize.cy;
	
	WND* btn;
        btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_ARROWUP;
        wnd_set_image(btn, &bitmap_arrow_up_black, 0);
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, value->frame.right+10, value->frame.top+labelSize.cy/2-30, 50, 30);

        btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_ARROWDOWN;
        wnd_set_image(btn, &bitmap_arrow_down_black, 0);
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, value->frame.right+10, value->frame.top+labelSize.cy/2, 50, 30);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_MINUS;
        wnd_set_text(btn, "+/-");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+0*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY1;
        wnd_set_text(btn, "1");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+1*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY2;
        wnd_set_text(btn, "2");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+2*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY3;
        wnd_set_text(btn, "3");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+3*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY4;
        wnd_set_text(btn, "4");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+4*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY5;
        wnd_set_text(btn, "5");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+5*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_DEL;
        wnd_set_text(btn, "DEL");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+6*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY6;
        wnd_set_text(btn, "6");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+1*bw, dy2, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY7;
        wnd_set_text(btn, "7");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+2*bw, dy2, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY8;
        wnd_set_text(btn, "8");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+3*bw, dy2, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY9;
        wnd_set_text(btn, "9");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+4*bw, dy2, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY0;
        wnd_set_text(btn, "0");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+5*bw, dy2, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_DOT;
        wnd_set_text(btn, ".");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+6*bw, dy2, btnSize.cx, btnSize.cy);

	print("show_input_num::end \r\n");

        return hwnd;
}

WND* show_input_num(INPUT_TEXT_PARAM *param) {
	return show_input_num_format(param, "%.3f");
}


#include "uilib.h"
#include "utf8.h"
#include <string.h>
#include "lcd-ft800.h"
#include "utils.h"
#include <stdio.h>

#define TAG_INPUT_VAL 		374278472

#define CMD_BACK		4
#define CMD_OK			8

#define CMD_KEY0		800
#define CMD_KEY1		801
#define CMD_KEY2		802
#define CMD_KEY3		803
#define CMD_KEY4		804
#define CMD_KEY5		805
#define CMD_KEY6		806
#define CMD_KEY7		807
#define CMD_KEY8		808
#define CMD_KEY9		809
#define CMD_KEY_DEL		810
#define CMD_KEY_DOT		811
#define CMD_KEY_ARROWUP		812
#define CMD_KEY_ARROWDOWN 	813
#define CMD_KEY_MINUS		814

extern WND* SCREEN;
extern Bitmap bitmap_arrow_up_black;
extern Bitmap bitmap_arrow_down_black;

static INPUT_TEXT_PARAM INPUT_PARAM;
static char FORMAT[16];

static int input_num_wnd_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_COMMAND: {
                int cmd=p1;
		WND* cmdWnd = (WND*)p2;

                print("input_num_wnd_proc::WM_COMMAND cmd=%d, cmdWnd=%p\n", cmd, cmdWnd);
                
                switch(cmd) {
		case CMD_KEY_ARROWUP: {
                        print("input_num_wnd_proc::CMD_KEY_ARROWUP\n");
                        WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        char val_text[64];
                        wnd_get_text(val, val_text, SIZE(val_text));
                        float fval = atof(val_text);

                        fval += INPUT_PARAM.step;
			if(fval<INPUT_PARAM.lower_limit) fval = INPUT_PARAM.lower_limit;
                        if(fval>INPUT_PARAM.upper_limit) fval = INPUT_PARAM.upper_limit;
                        snprintf(val_text, SIZE(val_text), FORMAT, fval);
                        wnd_set_text(val,val_text);
			val->label->selected = TRUE;
                } break;


                case CMD_KEY_ARROWDOWN: {
                        print("input_num_wnd_proc::CMD_KEY_ARROWDOWN\n");
                        WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        char val_text[64];
                        wnd_get_text(val, val_text, SIZE(val_text));
                        float fval = atof(val_text);

                        fval -= INPUT_PARAM.step;
                        if(fval<INPUT_PARAM.lower_limit) fval = INPUT_PARAM.lower_limit;
			if(fval>INPUT_PARAM.upper_limit) fval = INPUT_PARAM.upper_limit;
                        snprintf(val_text, SIZE(val_text), FORMAT, fval);
                        wnd_set_text(val,val_text);
			val->label->selected = TRUE;
                } break;

		case CMD_KEY0 ... CMD_KEY9: {
			char key_text[2];
			wnd_get_text(cmdWnd, key_text, SIZE(key_text));
			print("input_num_wnd_proc::WM_COMMAND key=%c pressed\n", key_text[0]);

			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
			char val_text[64];

			if(val->label->selected) {
				val_text[0]=0;
			} else {
				wnd_get_text(val, val_text, SIZE(val_text));
			}
			print("input_num_wnd_proc::WM_COMMAND cmd=%d, selected=%d, val_text=%s\n", cmd, val->label->selected, val_text);
			if(strlen(val_text)<19) strcat(val_text, key_text);
			print("input_num_wnd_proc::WM_COMMAND val_new_text=%s\n", val_text);
			
			wnd_set_text(val, val_text);
			val->label->selected = FALSE;
		} break;


		case CMD_KEY_DEL: {
			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        char val_text[64];

			if(val->label->selected) {
                                val_text[0]=0;
                        } else {
                        	wnd_get_text(val, val_text, SIZE(val_text));
			}
                        print("input_num_wnd_proc::CMD_KEY_DEL selected=%d, val_text=%s\n", val->label->selected, val_text);
			int len = strlen(val_text);
			if(len>0) val_text[len-1] = 0;
                        print("input_num_wnd_proc::CMD_KEY_DEL val_new_text=%s\n", val_text);
                        wnd_set_text(val, val_text);
			val->label->selected = FALSE;
		} break;


		case CMD_KEY_DOT: {
			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        char val_text[64];

                        wnd_get_text(val, val_text, SIZE(val_text));
                        print("input_num_wnd_proc::CMD_KEY_DOT val_text=%s\n", val_text);
			if(strlen(val_text)>0 && strchr(val_text, '.') == NULL) {
				if(strlen(val_text)<19) strcat(val_text, ".");
			}
                        print("input_num_wnd_proc::CMD_KEY_DOT val_new_text=%s\n", val_text);
                        wnd_set_text(val, val_text);
		} break;

		case CMD_KEY_MINUS: {
                        WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        char val_text[64];

                        wnd_get_text(val, val_text, SIZE(val_text));
                        print("input_num_wnd_proc::CMD_KEY_MINUS val_text=%s\n", val_text);
			float fval = atof(val_text);
                        snprintf(val_text, SIZE(val_text), FORMAT, -fval);

                        print("input_num_wnd_proc::CMD_KEY_MINUS val_new_text=%s\n", val_text);

			//set_param(INPUT_PARAM.param_id, val_text);
                        wnd_set_text(val, val_text);
                } break;


		case CMD_OK: {
			print("input_num_wnd_proc::CMD_OK::0\r\n");
			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        wnd_get_text(val, INPUT_PARAM.text, sizeof(INPUT_PARAM.text));

			print("input_num_wnd_proc::CMD_OK::2\r\n");
			float fval = atof(INPUT_PARAM.text);
                        if(fval<INPUT_PARAM.lower_limit) fval = INPUT_PARAM.lower_limit;
                        if(fval>INPUT_PARAM.upper_limit) fval = INPUT_PARAM.upper_limit;
                        snprintf(INPUT_PARAM.text, sizeof(INPUT_PARAM.text), FORMAT, fval);


                        wnd_proc_call(INPUT_PARAM.hwnd, INPUT_PARAM.msg, INPUT_PARAM.param_id, (int)INPUT_PARAM.text);
                        wnd_destroy(hwnd);

                        print("input_num_wnd_proc::CMD_OK::end \n");
			DelayLoopMicro(200);

                        return 0;

		} break;

		case CMD_BACK: {
			print("input_num_wnd_proc::CMD_BACK \r\n");
                        //wnd_proc_call(hwnd->superview, WM_UPDATE, 0, 0);
                        wnd_destroy(hwnd);

			return 0;

                } break;
		
		default:
			wnd_default_proc(hwnd, msg, p1, p2);
                };//switch

        } break;

        default:
                wnd_default_proc(hwnd, msg, p1, p2);
        }//
}


WND* show_input_num_format(INPUT_TEXT_PARAM *param, char* format) {
	print("show_input_num\r\n");

	INPUT_PARAM = *param;
	snprintf(FORMAT, sizeof(FORMAT), "%s", format);

	print("show_input_num::2 FORMAT=%s, format=%s\r\n",FORMAT,format);

        WND* hwnd=wnd_create(FORM);

        wnd_set_frame(hwnd, 0, 0, LCD_WIDTH, LCD_HEIGHT);
        wnd_set_proc(hwnd, &input_num_wnd_proc);
        wnd_set_text(hwnd, param->title);
	//wnd_set_text(hwnd->form->ok, "Сохранить");
	if(param->ok_text) wnd_set_text(hwnd->form->ok, param->ok_text);
	hwnd->form->ok->tag = CMD_OK;
        //wnd_set_text(hwnd->form->cancel, "Назад");
	if(param->cancel_text) wnd_set_text(hwnd->form->cancel, param->cancel_text);
	hwnd->form->cancel->tag = CMD_BACK;
        wnd_add_subview(SCREEN, hwnd);

	WND* label = wnd_create(LABEL);
        wnd_set_font(label, font_large);
	wnd_set_text(label, "Значение: ");
        wnd_add_subview(hwnd, label);

	WND* valuebg = wnd_create(WINDOW);
	valuebg->bgcolor = WHITE;
	wnd_add_subview(hwnd,valuebg);

	char buf[64];
	WND* value= wnd_create(LABEL);
	value->tag = TAG_INPUT_VAL;
        wnd_set_font(value, font_large);
	wnd_set_text_alignment(value, right);
        //wnd_set_text(value, get_param(param->param_id, buf, SIZE(buf)));
	wnd_set_text(value, param->text);
	value->label->selected = TRUE;
        wnd_add_subview(hwnd,value);

	Size labelSize = wnd_size_that_fits(label);
                
	wnd_set_frame(label, LCD_WIDTH/2-labelSize.cx,60, labelSize.cx, labelSize.cy);
	wnd_set_frame(valuebg, label->frame.right, 55, 140+10, labelSize.cy+10);
        wnd_set_frame(value, label->frame.right+5, 60, 140, labelSize.cy);
	
	Size btnSize = MakeSize(60, 40);
	int bw = btnSize.cx + 0;
	int dx = (LCD_WIDTH - bw*7)/2;
	int dy1 = LCD_HEIGHT - 60 - btnSize.cy - btnSize.cy;
	int dy2 = LCD_HEIGHT - 60 - btnSize.cy;
	
	WND* btn;
        btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_ARROWUP;
        wnd_set_image(btn, &bitmap_arrow_up_black, 0);
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, value->frame.right+10, value->frame.top+labelSize.cy/2-30, 50, 30);

        btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_ARROWDOWN;
        wnd_set_image(btn, &bitmap_arrow_down_black, 0);
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, value->frame.right+10, value->frame.top+labelSize.cy/2, 50, 30);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_MINUS;
        wnd_set_text(btn, "+/-");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+0*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY1;
        wnd_set_text(btn, "1");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+1*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY2;
        wnd_set_text(btn, "2");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+2*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY3;
        wnd_set_text(btn, "3");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+3*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY4;
        wnd_set_text(btn, "4");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+4*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY5;
        wnd_set_text(btn, "5");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+5*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_DEL;
        wnd_set_text(btn, "DEL");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+6*bw, dy1, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY6;
        wnd_set_text(btn, "6");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+1*bw, dy2, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY7;
        wnd_set_text(btn, "7");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+2*bw, dy2, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY8;
        wnd_set_text(btn, "8");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+3*bw, dy2, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY9;
        wnd_set_text(btn, "9");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+4*bw, dy2, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY0;
        wnd_set_text(btn, "0");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+5*bw, dy2, btnSize.cx, btnSize.cy);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_DOT;
        wnd_set_text(btn, ".");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx+6*bw, dy2, btnSize.cx, btnSize.cy);

	print("show_input_num::end \r\n");

        return hwnd;
}

WND* show_input_num(INPUT_TEXT_PARAM *param) {
	return show_input_num_format(param, "%.3f");
}


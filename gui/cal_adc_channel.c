#include "uilib.h"
#include "lcd-ft800.h"
#include "utils.h"
#include <stdio.h>
#include <stm32f4xx.h>
#include "adc.h"
#include "listener.h"
#include "msg.h"
#include "config.h"


#define ADC_VALS_SIZE	100
#define TAG_INFO	7223232

#define CMD_OK          8
#define CMD_CANCEL	7

extern WND* SCREEN;
extern WND* show_input_num(INPUT_TEXT_PARAM* p);
extern WND* show_alert(char* text);

static INPUT_TEXT_PARAM INPUT_PARAM;
static int adc_channel;
static int state;
static WND* hwnd_parent;

static void update_info(WND* hwnd);
static void set_state(WND* hwnd, int new_state);
static float adc_val_avg;
static float adc_val_sum;
static int adc_val_count;
static int adc_val_enable;

static float config_adc_offset;
static float config_adc_coeff;

static void adc_event_proc(int msg, int p1, int p2) {
	if(adc_val_enable) {
		adc_val_sum += ADC_DATA.adc_avg[adc_channel];
		adc_val_count++;
		adc_val_avg =  (adc_val_sum / (float) adc_val_count);
		print("adc_event_proc value = %f, sum = %f, count  =%d, avg = %f\r\n",  ADC_DATA.adc_avg[adc_channel], adc_val_sum, adc_val_count, adc_val_avg);
	}
}

static INPUT_TEXT_PARAM MakeTextParam(WND* hwnd, int param_id) {

        INPUT_TEXT_PARAM param;
        memset(&param, 0, sizeof(INPUT_TEXT_PARAM));
        param.param_id = param_id;
        param.hwnd = hwnd;
        param.msg = WM_INPUT_TEXT;
        param.cancel_text = "Назад";
        param.ok_text = "Сохранить";


        param.step = 1;

	param.title = "Секунды";
	param.lower_limit = 0;
        param.upper_limit = 59;

        return param;
}


static int cal_adc_channel_wnd_proc(WND* hwnd, int msg, int p1, int p2) {
	switch(msg) {

	case WM_INPUT_TEXT: {
		int param_id = p1;
                char* text = (char*) p2;

		if(param_id == 0 || text == 0) break;

		/*
		WND* val = wnd_with_tag(hwnd, param_id);
		int n = abs(atoi(text));

		char buf[64];

		switch(param_id) {
        	case TAG_SECOND:
			if(n>59) n=59;
			snprintf(buf, sizeof(buf), "%02d", n);
                	break;
        	}

        	wnd_set_text(val, buf);
		*/
	} break;

	case WM_COMMAND: {
                int cmd=p1;
                WND* cmdWnd = (WND*)p2;

                print("cal_adc_channel_wnd_proc::WM_COMMAND cmd=%d, cmdWnd=%p\r\n", cmd, cmdWnd);

                switch(cmd) {

		//case TAG_SECOND: {
		//	INPUT_PARAM= MakeTextParam(hwnd, cmd);
                //        show_input_num(&INPUT_PARAM);
                //} break;

		case CMD_CANCEL: {
			listener_remove(ADC1_EVENT, &adc_event_proc);
                	config_active.adc.offset[adc_channel] = config_adc_offset;
                	config_active.adc.coeff[adc_channel] = config_adc_coeff;
                	save_config();
                	wnd_destroy(hwnd);
		} break;

		case CMD_OK: {
			set_state(hwnd, state+1);
		} break;

		default:
			wnd_default_proc(hwnd, msg, p1, p2);
		}
	} break;

	case WM_LAYOUT: {
		update_info(hwnd);
		wnd_default_proc(hwnd, msg, p1, p2);

		Size contentSize = MakeSize(RECT_WIDTH(hwnd->form->view->frame), RECT_HEIGHT(hwnd->form->view->frame));
		WND* info = wnd_with_tag(hwnd, TAG_INFO);
		Size info_size = wnd_size_that_fits(info);
		info->frame = MakeRect((contentSize.cx - info_size.cx)/2, (contentSize.cy - info_size.cy)/2, info_size.cx, info_size.cy);
	} break;

	default:
		wnd_default_proc(hwnd, msg, p1, p2);
	}
}

WND* show_cal_adc_channel(WND* parent, int index) {
	adc_channel = index;
	state = 0;
	hwnd_parent = parent;

	WND* hwnd = wnd_create(FORM);

	char buf[64];

	wnd_set_frame(hwnd, 0, 0, LCD_WIDTH, LCD_HEIGHT);
        wnd_set_proc(hwnd, &cal_adc_channel_wnd_proc);
	snprintf(buf, sizeof(buf), "Калибровка АЦП канала #%02d", adc_channel);
        wnd_set_text(hwnd, buf);
        wnd_set_text(hwnd->form->ok, "Сохранить");
        hwnd->form->ok->tag = CMD_OK;
        wnd_set_text(hwnd->form->cancel, "Отмена");
	hwnd->form->cancel->tag = CMD_CANCEL;
        wnd_add_subview(SCREEN, hwnd);

        WND* info= wnd_create(LABEL);
	info->tag = TAG_INFO;
        wnd_set_font(info, font_large);
	wnd_set_text_alignment(info, center);
        wnd_add_subview(hwnd->form->view, info);

	config_adc_offset = config_active.adc.offset[adc_channel];
	config_adc_coeff =  config_active.adc.coeff[adc_channel];

	set_state(hwnd, 0);
	listener_set(ADC1_EVENT, &adc_event_proc);
	return hwnd;
}

void update_info(WND* hwnd) {
	char buf[256];

	switch(state) {

	case 0: {
		wnd_set_text(wnd_with_tag(hwnd, CMD_OK), "Далее");
		snprintf(buf, sizeof(buf), "Отсоедините все провода\nот АЦП канала #%02d\nи нажмите <Далее>", adc_channel);
		wnd_set_text(wnd_with_tag(hwnd, TAG_INFO), buf);

	} break;

	case 1: {
                wnd_set_text(wnd_with_tag(hwnd, CMD_OK), "Далее");
                snprintf(buf, sizeof(buf), "Подождите 10 секунд\nи нажмите <Далее>\n\navg = %f", adc_channel, adc_val_avg);
                wnd_set_text(wnd_with_tag(hwnd, TAG_INFO), buf);
        } break;	

	case 2: {
                wnd_set_text(wnd_with_tag(hwnd, CMD_OK), "Далее");
                snprintf(buf, sizeof(buf), "Подключите источник референсного\nнапряжения 5В к АЦП канала #%02d\nи нажмите <Далее>", adc_channel);
                wnd_set_text(wnd_with_tag(hwnd, TAG_INFO), buf);

        } break;

	case 3: {
		wnd_set_text(wnd_with_tag(hwnd, CMD_OK), "Далее");
		snprintf(buf, sizeof(buf), "Подождите 10 секунд\nи нажмите <Далее>\n\navg = %f", adc_channel, adc_val_avg);
		wnd_set_text(wnd_with_tag(hwnd, TAG_INFO), buf);

        } break;

	case 4: {
		wnd_set_text(wnd_with_tag(hwnd, CMD_OK), "Сохранить");
		snprintf(buf, sizeof(buf), "Калибровка закончена\n\nсмещение: %f\nкоэффициент: %f", config_active.adc.offset[adc_channel],
										config_active.adc.coeff[adc_channel]);
		wnd_set_text(wnd_with_tag(hwnd, TAG_INFO), buf);
	} break;

	}//switch
}

void set_state(WND* hwnd, int new_state) {
	state = new_state;
	print("next_state state=%d\r\n", state);

	switch(state) {

        case 0: 
                adc_val_count = 0;
                adc_val_sum  = 0;
                adc_val_avg = ADC_DATA.adc_avg[adc_channel];
		adc_val_enable = 0;
		config_active.adc.offset[adc_channel] = 0;
		config_active.adc.coeff[adc_channel] = 1;
		print("::0 adc_channel offset-%f, coeff=%f\r\n", config_active.adc.offset[adc_channel], config_active.adc.coeff[adc_channel]);
		break;
	case 1:
		adc_val_enable = 1;
		print("::1 adc_channel offset-%f, coeff=%f\r\n", config_active.adc.offset[adc_channel], config_active.adc.coeff[adc_channel]);
                break;

	case 2:
		config_active.adc.offset[adc_channel] = adc_val_avg;
		
		adc_val_count = 0;
                adc_val_sum  = 0;
                adc_val_avg = ADC_DATA.adc_avg[adc_channel];
                adc_val_enable = 0;
                break;

	case 3:
		adc_val_enable = 1;
		break;

	case 4:
                if(fabs(adc_val_avg) >0.00001) {
			config_active.adc.coeff[adc_channel] = 5.0 / adc_val_avg;
		} else  {
			config_active.adc.coeff[adc_channel] = 0;
		}
		
		adc_val_count = 0;
                adc_val_sum  = 0;
                adc_val_avg = ADC_DATA.adc_avg[adc_channel];
                adc_val_enable = 0;
                break;

	case 5:
		listener_remove(ADC1_EVENT, &adc_event_proc);
                save_config();

		//DelayLoopMicro(500000);
		//print("cal_adc_channel::end, call WM_UPDATE for hwnd=%p\r\n", hwnd_parent);
		if(hwnd_parent) wnd_proc_call(hwnd_parent, WM_UPDATE, 0, 0);
		//DelayLoopMicro(500000);

                wnd_destroy(hwnd);

	};

	update_info(hwnd);

}

#include "uilib.h"
#include "config.h"
#include "lcd-ft800.h"
#include "utils.h"
#include <stdio.h>
#include "adc.h"

#include <math.h>

#define CMD_BACK		4
#define CMD_SAVE_PARAM		8

#define NUM_OF_BLOCK_PARAMS	16

#define CMD_BLOCK_ADC		10000
#define CMD_BLOCK_ADC_OFFSET	11100
#define CMD_BLOCK_ADC_COEFF	11200
#define CMD_BLOCK_ADC_CAL	11300
#define CMD_BLOCK_ADC_AVG	11400
#define CMD_BLOCK_ADC_RMS	11500

extern WND* SCREEN;
extern uint32_t millisec_timestamp;
extern void show_input_num(INPUT_TEXT_PARAM* p);
extern void show_input_num_format(INPUT_TEXT_PARAM* p, char* format);
extern void show_cal_adc_channel(WND* parent, int index);

typedef struct {
	char title[64];
	int cmd;
	int param;
} MENU_ITEM;

static uint32_t last_update_timestamp;

static INPUT_TEXT_PARAM INPUT_PARAM;
static WND* show_menu(WND* menu_parent, char* title, MENU_ITEM *menu_list, int menu_list_size);
static int menu_wnd_proc(WND* hwnd, int msg, int p1, int p2);
static INPUT_TEXT_PARAM MakeNumParam(WND* hwnd, char* title, int param_id, float lower_limit, float upper_limit , float step);

static uint32_t channel_format = 0;

//static void adc_event_proc(int msg, int p1, int p2) {
//}


INPUT_TEXT_PARAM MakeNumParam(WND* hwnd, char* title, int param_id, float lower_limit, float upper_limit , float step) {
	INPUT_TEXT_PARAM param;
	memset(&param, 0, sizeof(INPUT_TEXT_PARAM));
        param.title = title;
	param.param_id = param_id;
        param.hwnd = hwnd;
        param.msg = WM_INPUT_TEXT;
        param.cancel_text = "Назад";
        param.ok_text = "Сохранить";

        param.lower_limit = lower_limit;
        param.upper_limit = upper_limit;
        param.step = step;

	switch(param_id) {
        	case CMD_BLOCK_ADC_OFFSET ... (CMD_BLOCK_ADC_OFFSET+NUM_OF_BLOCK_PARAMS): {
                	int index = param_id - CMD_BLOCK_ADC_OFFSET;
                        snprintf(param.text, sizeof(param.text), "%.6f", config_active.adc.offset[index]);
                } break;
                case CMD_BLOCK_ADC_COEFF ... (CMD_BLOCK_ADC_COEFF+NUM_OF_BLOCK_PARAMS): {
                        int index = param_id - CMD_BLOCK_ADC_COEFF;
                        snprintf(param.text, sizeof(param.text), "%.6f", config_active.adc.coeff[index]);
                } break;

        }//switch

	return param;
}

int menu_wnd_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {
	
	case WM_DO_DRAW: {
		if(millisec_timestamp - last_update_timestamp >= 500) {
			last_update_timestamp = millisec_timestamp;
			wnd_proc_call(hwnd, WM_UPDATE, 0, 0);
		}
		
		wnd_default_proc(hwnd, msg, p1, p2);
        } break;

	case WM_UPDATE: {
		//print("adc_setup::WM_UPDATE hwnd=%p\r\n", hwnd);
		char buf[64];
		WND* subview=hwnd->form->view->scroll->content_view->subviews;
                while(subview) {
			if(subview->wnd_class == MENU_CELL) {
				int param_id = subview->cell->param_id;
				switch(param_id) {
				case CMD_BLOCK_ADC_OFFSET ... (CMD_BLOCK_ADC_OFFSET+NUM_OF_BLOCK_PARAMS): {
                        		int index = subview->cell->param_id - CMD_BLOCK_ADC_OFFSET;
					snprintf(buf, sizeof(buf), "%.6f", config_active.adc.offset[index]);
					wnd_set_text(subview->cell->details, buf);
				} break;
				case CMD_BLOCK_ADC_COEFF ... (CMD_BLOCK_ADC_COEFF+NUM_OF_BLOCK_PARAMS): {
                                        int index = subview->cell->param_id - CMD_BLOCK_ADC_COEFF;
                                        snprintf(buf, sizeof(buf), "%.6f", config_active.adc.coeff[index]);
                                        wnd_set_text(subview->cell->details, buf);
                                } break;

				case CMD_BLOCK_ADC_AVG ... (CMD_BLOCK_ADC_AVG+NUM_OF_BLOCK_PARAMS): {
					int index = subview->cell->param_id - CMD_BLOCK_ADC_AVG;
                                        snprintf(buf, sizeof(buf), "%.6f", ADC_DATA.adc_avg[index]);
                                        wnd_set_text(subview->cell->details, buf);
				} break;

				case CMD_BLOCK_ADC_RMS ... (CMD_BLOCK_ADC_RMS+NUM_OF_BLOCK_PARAMS): {
                                        int index = subview->cell->param_id - CMD_BLOCK_ADC_RMS;
                                        snprintf(buf, sizeof(buf), "%.6f", ADC_DATA.adc_rms[index]);
                                        wnd_set_text(subview->cell->details, buf);
                                } break;


				}//switch

			}
                        subview = subview->next_subview;
                }
		//print("WM_UPDATE::end\r\n");
	} break;

	case WM_INPUT_TEXT: {
		int param_id = p1;
                char* text = (char*) p2;

		if(param_id == 0 || text == 0) break;

		float fval = atof(text);
		switch(param_id) {
                case CMD_BLOCK_ADC_OFFSET ... (CMD_BLOCK_ADC_OFFSET+NUM_OF_BLOCK_PARAMS): {
                	int index = param_id - CMD_BLOCK_ADC_OFFSET;
                        config_active.adc.offset[index] = fval;
			save_config();
                 } break;
                 case CMD_BLOCK_ADC_COEFF ... (CMD_BLOCK_ADC_COEFF+NUM_OF_BLOCK_PARAMS): {
                 	int index = param_id - CMD_BLOCK_ADC_COEFF;
                        config_active.adc.coeff[index] = fval;
			save_config();
                 } break;

		}//switch

                wnd_proc_call(hwnd, WM_UPDATE, 0, 0);
	} break;

        case WM_COMMAND: {
                int cmd=p1;

                switch(cmd) {

		case CMD_BACK: {
			if(hwnd->extra) {
				wnd_proc_call((WND*) hwnd->extra,WM_UPDATE, 0, 0);
			}
			wnd_destroy(hwnd);
			return 0;
		} break;

		case CMD_BLOCK_ADC ... (CMD_BLOCK_ADC + NUM_OF_BLOCK_PARAMS): {
			int index = cmd - CMD_BLOCK_ADC;

                        MENU_ITEM menu[] = {
                                {"Смещение АЦП", CMD_BLOCK_ADC_OFFSET + index, CMD_BLOCK_ADC_OFFSET + index},
                                {"Коеффициент АЦП", CMD_BLOCK_ADC_COEFF + index, CMD_BLOCK_ADC_COEFF + index},
				{"Vavg", 0, CMD_BLOCK_ADC_AVG + index},
				{"Vrms", 0, CMD_BLOCK_ADC_RMS + index},
				{"Калибровка канала", CMD_BLOCK_ADC_CAL + index, 0},
                        };

                        char buf[64];
			if(channel_format==1) {
				sprintf(buf, "АЦП канал X%02d", index+10+1);
			} else {
                        	sprintf(buf, "АЦП канал #%02d", index);
			}
                        show_menu(hwnd, buf, menu, SIZE(menu));
		} break;

		case CMD_BLOCK_ADC_OFFSET ... (CMD_BLOCK_ADC_OFFSET+NUM_OF_BLOCK_PARAMS): {
                        int index = cmd - CMD_BLOCK_ADC_OFFSET;

			INPUT_PARAM = MakeNumParam(hwnd, "Смещение АЦП", CMD_BLOCK_ADC_OFFSET + index, -4096, +4096, 1);
                        show_input_num_format(&INPUT_PARAM,"%0.5f");
                } break;

                case CMD_BLOCK_ADC_COEFF ... (CMD_BLOCK_ADC_COEFF+NUM_OF_BLOCK_PARAMS): {
                        int index = cmd - CMD_BLOCK_ADC_COEFF;

			INPUT_PARAM = MakeNumParam(hwnd, "Коэффициент АЦП", CMD_BLOCK_ADC_COEFF + index, -10, +10, 0.01);
                        show_input_num_format(&INPUT_PARAM,"%0.5f");

                } break;

		case CMD_BLOCK_ADC_CAL ... (CMD_BLOCK_ADC_CAL+NUM_OF_BLOCK_PARAMS): {
			int index = cmd - CMD_BLOCK_ADC_CAL;

			show_cal_adc_channel(hwnd, index);	
		} break;


		default:
			wnd_default_proc(hwnd, msg, p1, p2);
                };//switch
        } break;

        default:
                wnd_default_proc(hwnd, msg, p1, p2);
        }//switch
}


WND* show_menu(WND* hwnd_menu_parent, char* title, MENU_ITEM *menu_list, int menu_list_size) {
        WND* hwnd=wnd_create(FORM);
        wnd_set_frame(hwnd, 0, 0, LCD_WIDTH, LCD_HEIGHT);
        wnd_set_proc(hwnd, &menu_wnd_proc);
        wnd_set_text(hwnd, title);

        int row_height = font_large->height;
        for(int i=0; i<menu_list_size; i++) {
                WND* item = wnd_create(MENU_CELL);
                item->tag= menu_list[i].cmd;
		item->cell->param_id = menu_list[i].param;
                wnd_set_frame(item, 10, i*row_height, LCD_WIDTH - 10-50-20, row_height);
                wnd_set_text(item->cell->title, menu_list[i].title);
                wnd_add_subview(hwnd->form->view->scroll->content_view, item);
        }

        hwnd->form->view->scroll->scroll_step = row_height;
        hwnd->form->view->scroll->content_size = MakeSize(LCD_WIDTH, menu_list_size*row_height);
        wnd_set_flags(hwnd->form->ok, FLAG_HIDDEN);
	wnd_set_text(hwnd->form->cancel, "Назад");
	hwnd->form->cancel->tag = CMD_BACK;

	wnd_add_subview(SCREEN, hwnd);
	hwnd->extra = hwnd_menu_parent;
	wnd_proc_call(hwnd, WM_UPDATE, 0, 0);

	return hwnd;
}


WND* show_adc_setup(int format) {

	last_update_timestamp = millisec_timestamp;
	channel_format = format;

	/*
	MENU_ITEM menu[] = {
                {"АЦП канал #00",  CMD_BLOCK_ADC+0, CMD_BLOCK_ADC_RMS + 0},
		{"АЦП канал #01",  CMD_BLOCK_ADC+1, CMD_BLOCK_ADC_RMS + 1},
		{"АЦП канал #02",  CMD_BLOCK_ADC+2, CMD_BLOCK_ADC_RMS + 2},
		{"АЦП канал #03",  CMD_BLOCK_ADC+3, CMD_BLOCK_ADC_RMS + 3},
		{"АЦП канал #04",  CMD_BLOCK_ADC+4, CMD_BLOCK_ADC_RMS + 4},
		{"АЦП канал #05",  CMD_BLOCK_ADC+5, CMD_BLOCK_ADC_RMS + 5},
		{"АЦП канал #06",  CMD_BLOCK_ADC+6, CMD_BLOCK_ADC_RMS + 6},
		{"АЦП канал #07",  CMD_BLOCK_ADC+7, CMD_BLOCK_ADC_RMS + 7},
		{"АЦП канал #08",  CMD_BLOCK_ADC+8, CMD_BLOCK_ADC_RMS + 8},
		{"АЦП канал #09",  CMD_BLOCK_ADC+9, CMD_BLOCK_ADC_RMS + 9},
		{"АЦП канал #10",  CMD_BLOCK_ADC+10, CMD_BLOCK_ADC_RMS + 10},
		{"АЦП канал #11",  CMD_BLOCK_ADC+11, CMD_BLOCK_ADC_RMS + 11},
		{"АЦП канал #12",  CMD_BLOCK_ADC+12, CMD_BLOCK_ADC_RMS + 12},
		{"АЦП канал #13",  CMD_BLOCK_ADC+13, CMD_BLOCK_ADC_RMS + 13},
		{"АЦП канал #14",  CMD_BLOCK_ADC+14, CMD_BLOCK_ADC_RMS + 14},
		{"АЦП канал #15",  CMD_BLOCK_ADC+15, CMD_BLOCK_ADC_RMS + 15},
        };
	*/

	MENU_ITEM menu[16];
	int size = SIZE(menu);
	if(ADC_NUM_OF_CHANNELS < size) size = ADC_NUM_OF_CHANNELS;	

	for(int i=0; i<size; i++) {
		if(channel_format==1) {
			snprintf(menu[i].title, sizeof(menu[i].title), "АЦП канал X%02d", 10+1+i);
		} else {
			snprintf(menu[i].title, sizeof(menu[i].title), "АЦП канал #%02d", i);
		}
		menu[i].cmd = CMD_BLOCK_ADC+i;
		menu[i].param = CMD_BLOCK_ADC_RMS+i;
	}

        return show_menu(NULL, "Калибровка АЦП", menu, size);
}

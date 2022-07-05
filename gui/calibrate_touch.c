#include "uilib.h"
#include "lcd-ft800.h"
#include "config.h"
#include "utils.h"
#include <stdio.h>

#define CMD_BACK                4
#define CMD_SAVE_PARAM          8

#define CALIBRATE_MAX_POINTS    5

extern WND* SCREEN;
extern HIT_TEST_INFO TOUCH_INFO;


typedef struct {
        int step;
        WND* msg;
        Point screen_pt;

        Point sample_points[CALIBRATE_MAX_POINTS];
        Point reference_points[CALIBRATE_MAX_POINTS];

        float KX1;
        float KX2;
        float KX3;
        float KY1;
        float KY2;
        float KY3;

} CALIBRATE_ITEM;

static CALIBRATE_ITEM CALIBRATE = {0};

static void calc_calibration_coeff(CALIBRATE_ITEM* cal) {
	if(cal==NULL) return;

	print("calc_calibration_coeff\n");

        int n=CALIBRATE_MAX_POINTS;
        if(n<3) {
		print("calc_calibration_coeff: need 3 points or morei (n=%d)\n", n);
                return ;
	}

 	int i;
        double a[3],b[3],c[3],d[3],k;

        if(n==3) {
                        for(i=0; i<n; i++) {
                                a[i]=(double)(cal->sample_points[i].x);
                                b[i]=(double)(cal->sample_points[i].y);
                                c[i]=(double)(cal->reference_points[i].x);
                                d[i]=(double)(cal->reference_points[i].y);
                        }
        } else if(n>3) {
                        for(i=0; i<3; i++) {
                                a[i]=0;
                                b[i]=0;
                                c[i]=0;
                                d[i]=0;
                        }

                        for(i=0; i<n; i++) {
                                a[2]=a[2]+(double)(cal->sample_points[i].x);
                                b[2]=b[2]+(double)(cal->sample_points[i].y);
                                c[2]=c[2]+(double)(cal->reference_points[i].x);
                                d[2]=d[2]+(double)(cal->reference_points[i].y);
                                a[0]=a[0]+(double)(cal->sample_points[i].x)*(double)(cal->sample_points[i].x);
                                a[1]=a[1]+(double)(cal->sample_points[i].x)*(double)(cal->sample_points[i].y);
                                b[0]=a[1];
                                b[1]=b[1]+(double)(cal->sample_points[i].y)*(double)(cal->sample_points[i].y);
                                c[0]=c[0]+(double)(cal->sample_points[i].x)*(double)(cal->reference_points[i].x);
                                c[1]=c[1]+(double)(cal->sample_points[i].y)*(double)(cal->reference_points[i].x);
                                d[0]=d[0]+(double)(cal->sample_points[i].x)*(double)(cal->reference_points[i].y);
                                d[1]=d[1]+(double)(cal->sample_points[i].y)*(double)(cal->reference_points[i].y);
                        }

                        a[0]=a[0]/a[2];
                        a[1]=a[1]/b[2];
                        b[0]=b[0]/a[2];
                        b[1]=b[1]/b[2];
                        c[0]=c[0]/a[2];
                        c[1]=c[1]/b[2];
                        d[0]=d[0]/a[2];
                        d[1]=d[1]/b[2];
                        a[2]=a[2]/n;
                        b[2]=b[2]/n;
                        c[2]=c[2]/n;
                        d[2]=d[2]/n;
        }

	k=(a[0]-a[2])*(b[1]-b[2])-(a[1]-a[2])*(b[0]-b[2]);
	cal->KX1=((c[0]-c[2])*(b[1]-b[2])-(c[1]-c[2])*(b[0]-b[2]))/k;
	cal->KX2=((c[1]-c[2])*(a[0]-a[2])-(c[0]-c[2])*(a[1]-a[2]))/k;
	cal->KX3=(b[0]*(a[2]*c[1]-a[1]*c[2])+b[1]*(a[0]*c[2]-a[2]*c[0])+b[2]*(a[1]*c[0]-a[0]*c[1]))/k;
	cal->KY1=((d[0]-d[2])*(b[1]-b[2])-(d[1]-d[2])*(b[0]-b[2]))/k;
	cal->KY2=((d[1]-d[2])*(a[0]-a[2])-(d[0]-d[2])*(a[1]-a[2]))/k;
	cal->KY3=(b[0]*(a[2]*d[1]-a[1]*d[2])+b[1]*(a[0]*d[2]-a[2]*d[0])+b[2]*(a[1]*d[0]-a[0]*d[1]))/k;

	print("calc_calibration_coeff: KX1=%f, KX2=%f, KX3=%f, KY1=%f, KY2=%f, KY3=%f\n", cal->KX1, cal->KX2, cal->KX3, cal->KY1, cal->KY2, cal->KY3);
}


static int calibrate_touch_wnd_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_COMMAND: {
                int cmd=p1;

                print("calibrate_touch_wnd_proc::WM_COMMAND cmd=%d\n", cmd);

                switch(cmd) {

		case CMD_BACK: {
			wnd_destroy(hwnd);
			return 0;
		} break;

                case CMD_SAVE_PARAM: {
			config_active.lcd.KX1 = CALIBRATE.KX1;
        		config_active.lcd.KX2 = CALIBRATE.KX2;
        		config_active.lcd.KX3 = CALIBRATE.KX3;
        		config_active.lcd.KY1 = CALIBRATE.KY1;
        		config_active.lcd.KY2 = CALIBRATE.KY2;
        		config_active.lcd.KY3 = CALIBRATE.KY3;

			int rc=save_config();
        		print("calibrate_touch_wnd_proc CMD_SAVE_PARAM save_config_rc=%d\n", rc);

                        wnd_destroy(hwnd);
                        return 0;
                } break;

                default:
                        wnd_default_proc(hwnd, msg, p1, p2);
                };//switch
	} break;

	case WM_DO_DRAW: {
		Context * gc=(Context*)p1;
                if(gc == NULL) break;

		wnd_default_proc(hwnd, msg, p1, p2);

		///////
		Rect rect={0};
        	char txt[64];
        	sprintf(txt,"LCD_TOUCH x=%d, y=%d", lcd_touch_x, lcd_touch_y);
		Font* font = font_small;
		gc_set_font(gc, font);
		Size sz=wnd_get_text_size(font, txt);
        	gc_draw_text(gc, txt, strlen(txt), (LCD_WIDTH-sz.cx)/2, 0, WHITE);
		//////

		gc_draw_rect(gc, 30, 30, LCD_WIDTH-2*30, LCD_HEIGHT-2*30, 1, GREY);

		if(CALIBRATE.step<CALIBRATE_MAX_POINTS) {
			gc_draw_line(gc, CALIBRATE.screen_pt.x-4, CALIBRATE.screen_pt.y, CALIBRATE.screen_pt.x+5, CALIBRATE.screen_pt.y, WHITE);
                	gc_draw_line(gc, CALIBRATE.screen_pt.x, CALIBRATE.screen_pt.y-4, CALIBRATE.screen_pt.x, CALIBRATE.screen_pt.y+5, WHITE);
		}

		if(TOUCH_INFO.view == hwnd) {
			int scr_touch_x=(int16_t)(CALIBRATE.KX1*(lcd_touch_x)+CALIBRATE.KX2*(lcd_touch_y)+CALIBRATE.KX3+0.5);
                	int scr_touch_y=(int16_t)(CALIBRATE.KY1*(lcd_touch_x)+CALIBRATE.KY2*(lcd_touch_y)+CALIBRATE.KY3+0.5);
			gc_fill_circle(gc, scr_touch_x, scr_touch_y, 2, GREEN);
		}




	} break;

	case WM_LAYOUT: {
		switch(CALIBRATE.step) {
        	case 0: 
			wnd_set_text(CALIBRATE.msg, "Нажмите крест вверху-слева");
                	CALIBRATE.screen_pt=MakePoint(30, 30);
			break;
        	case 1: 
			wnd_set_text(CALIBRATE.msg, "Нажмите крест вверху-справа");
                        CALIBRATE.screen_pt=MakePoint(LCD_WIDTH - 30, 30);
			break;
		case 2: 
                        wnd_set_text(CALIBRATE.msg, "Нажмите крест внизу-справа");
                        CALIBRATE.screen_pt=MakePoint(LCD_WIDTH - 30, LCD_HEIGHT - 30);
                        break;
		case 3:
                        wnd_set_text(CALIBRATE.msg, "Нажмите крест внизу-слева");
                        CALIBRATE.screen_pt=MakePoint(30, LCD_HEIGHT - 30);
                        break;
		case 4:
                        wnd_set_text(CALIBRATE.msg, "Нажмите крест в центре");
                        CALIBRATE.screen_pt=MakePoint(LCD_WIDTH/2, LCD_HEIGHT/2);
                        break;
		case 5:
			wnd_reset_flags(hwnd->form->footer, FLAG_HIDDEN);
			wnd_set_text(CALIBRATE.msg, "Калибровка закончена");
			WND* btn=wnd_with_tag(hwnd, CMD_BACK);
			if(btn) wnd_set_flags(btn, FLAG_HIDDEN);

			break;
		case 6:
			wnd_set_text(CALIBRATE.msg, "Тест сенс. экрана");
			break;
		}//switch

		Size msgSize = wnd_size_that_fits(CALIBRATE.msg);
        	wnd_set_frame(CALIBRATE.msg, (LCD_WIDTH - msgSize.cx)/2, LCD_HEIGHT/2+20, msgSize.cx, msgSize.cy);

		wnd_default_proc(hwnd, msg, p1, p2);
        } break;

        case WM_TOUCH_RELEASED: {
                //print("CALIBRATE::WM_TOUCH_RELEASED hwnd->tag=%d, CALIBRATE.step=%d lcd_touch_x=%d, lcd_touch_y=%d\n", hwnd->tag, CALIBRATE.step, lcd_touch_x, lcd_touch_y);
		if(CALIBRATE.step<CALIBRATE_MAX_POINTS) {
			CALIBRATE.sample_points[CALIBRATE.step] = MakePoint(lcd_touch_x, lcd_touch_y);
                	CALIBRATE.reference_points[CALIBRATE.step] = CALIBRATE.screen_pt;

			print("CALIBRATE.sample_points[%d](%d,%d), CALIBRATE.reference_points[%d](%d,%d)\n", CALIBRATE.step, CALIBRATE.sample_points[CALIBRATE.step].x, CALIBRATE.sample_points[CALIBRATE.step].y, CALIBRATE.step, CALIBRATE.reference_points[CALIBRATE.step].x, CALIBRATE.reference_points[CALIBRATE.step].y);

			CALIBRATE.step++;
			if(CALIBRATE.step == CALIBRATE_MAX_POINTS) calc_calibration_coeff(&CALIBRATE);
		} else {
                        CALIBRATE.screen_pt.x=(int16_t)(CALIBRATE.KX1*(lcd_touch_x)+CALIBRATE.KX2*(lcd_touch_y)+CALIBRATE.KX3+0.5);
                        CALIBRATE.screen_pt.y=(int16_t)(CALIBRATE.KY1*(lcd_touch_x)+CALIBRATE.KY2*(lcd_touch_y)+CALIBRATE.KY3+0.5);
		}
        } break;


	default:
        	wnd_default_proc(hwnd, msg, p1, p2);
	}//switch

	return 0;
}


	

WND* show_calibrate_touch(BOOL test) {
	print("show_calibrate_touch\n");

	memset(&CALIBRATE, 0, sizeof(CALIBRATE));

	CALIBRATE.KX1 = config_active.lcd.KX1;
	CALIBRATE.KX2 = config_active.lcd.KX2;
	CALIBRATE.KX3 = config_active.lcd.KX3;
	CALIBRATE.KY1 = config_active.lcd.KY1;
        CALIBRATE.KY2 = config_active.lcd.KY2;
        CALIBRATE.KY3 = config_active.lcd.KY3;

	WND* hwnd=wnd_create(FORM);
        wnd_set_frame(hwnd, 0, 0, LCD_WIDTH, LCD_HEIGHT);
        hwnd->wnd_proc = &calibrate_touch_wnd_proc;
        wnd_set_text(hwnd, "Калибровка сенс. экрана");


	WND* msg = wnd_create(LABEL);
	CALIBRATE.msg = msg;
	wnd_set_font(msg, font_large);
	wnd_set_text_color(msg, WHITE);
	wnd_add_subview(hwnd, msg);


	WND* btn = wnd_create(BUTTON);
	wnd_set_text(btn, "Отмена");
        wnd_add_subview(hwnd, btn);
	Size btnSize=wnd_size_that_fits(btn);
	wnd_set_frame(btn, (LCD_WIDTH-btnSize.cx)/2, LCD_HEIGHT/2+60, btnSize.cx+20, btnSize.cy+10);
	btn->tag = CMD_BACK;

	wnd_set_flags(hwnd->form->footer, FLAG_HIDDEN);
	wnd_set_flags(hwnd->form->title, FLAG_HIDDEN);
	wnd_set_flags(hwnd->form->view, FLAG_HIDDEN);

	wnd_set_text(hwnd->form->cancel, "Отмена");
	wnd_set_text(hwnd->form->ok, "Сохранить");
	hwnd->form->ok->tag = CMD_SAVE_PARAM;

	if(test) {
		CALIBRATE.step = 6;
		wnd_set_flags(hwnd->form->ok, FLAG_HIDDEN);

	}

	wnd_add_subview(SCREEN, hwnd);
	return hwnd;
}


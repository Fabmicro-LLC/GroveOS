#include "uilib.h"
#include "lcd-ft800.h"
#include "utils.h"
#include <stdio.h>
#include <stm32f4xx.h>


#define TAG_YEAR	72223001
#define TAG_MONTH	72223002
#define TAG_DAY		72223003
#define TAG_HOUR	72223004
#define TAG_MINUTE	72223005
#define TAG_SECOND	72223006


#define CMD_OK          8

extern WND* SCREEN;
extern uint32_t millisec_timestamp;
extern WND* show_input_num(INPUT_TEXT_PARAM* p);
extern WND* show_alert(char* text);

static INPUT_TEXT_PARAM INPUT_PARAM;

static void time_update(WND* hwnd);

static INPUT_TEXT_PARAM MakeTextParam(WND* hwnd, int param_id) {
        INPUT_TEXT_PARAM param;
        memset(&param, 0, sizeof(INPUT_TEXT_PARAM));
        param.param_id = param_id;
        param.hwnd = hwnd;
        param.msg = WM_INPUT_TEXT;
        param.cancel_text = "Назад";
        param.ok_text = "Сохранить";


        param.step = 1;


	WND* val = wnd_with_tag(hwnd, param_id);
        wnd_get_text(val, param.text, sizeof(param.text));

	switch(param_id) {
	case TAG_YEAR: 
		param.title = "Год";
        	param.lower_limit = 1900;
        	param.upper_limit = 2100;
		break;
        case TAG_MONTH:
		param.title = "Месяц";
		param.lower_limit = 1;
                param.upper_limit = 12;
		break;
        case TAG_DAY:
		param.title = "День";
		param.lower_limit = 1;
                param.upper_limit = 31;
		break;
        case TAG_HOUR:
		param.title = "Час";
		param.lower_limit = 0;
                param.upper_limit = 23;
		break;
        case TAG_MINUTE:
		param.title = "Минуты";
		param.lower_limit = 0;
                param.upper_limit = 59;
		break;
        case TAG_SECOND:
		param.title = "Секунды";
		param.lower_limit = 0;
                param.upper_limit = 59;
		break;
	}

        return param;
}


static int set_time_wnd_proc(WND* hwnd, int msg, int p1, int p2) {
	switch(msg) {

	case WM_INPUT_TEXT: {
		int param_id = p1;
                char* text = (char*) p2;

		if(param_id == 0 || text == 0) break;

		WND* val = wnd_with_tag(hwnd, param_id);
		int n = abs(atoi(text));

		char buf[64];

		switch(param_id) {
		case TAG_YEAR:
			if(n<1900) n=1900;
			snprintf(buf, sizeof(buf), "%04d", n);
                	break;
        	case TAG_MONTH:
			if(n<1) n=1;
			if(n>12) n=12;
			snprintf(buf, sizeof(buf), "%02d", n);
                	break;
        	case TAG_DAY:
			if(n<1) n=1;
			if(n>31) n=31;
			snprintf(buf, sizeof(buf), "%02d", n);
                	break;
        	case TAG_HOUR:
			if(n>23) n=23;
			snprintf(buf, sizeof(buf), "%02d", n);
                	break;
        	case TAG_MINUTE:
        	case TAG_SECOND:
			if(n>59) n=59;
			snprintf(buf, sizeof(buf), "%02d", n);
                	break;
        	}

        	wnd_set_text(val, buf);
	} break;

	case WM_COMMAND: {
                int cmd=p1;
                WND* cmdWnd = (WND*)p2;

                print("set_time_wnd_proc::WM_COMMAND cmd=%d, cmdWnd=%p\r\n", cmd, cmdWnd);

                switch(cmd) {

                case TAG_YEAR:
		case TAG_MONTH:
		case TAG_DAY:
		case TAG_HOUR:
		case TAG_MINUTE:
		case TAG_SECOND: {
			INPUT_PARAM= MakeTextParam(hwnd, cmd);
                        show_input_num(&INPUT_PARAM);
                } break;

		case CMD_OK: {
                        char buf[64];
			wnd_get_text(wnd_with_tag(hwnd, TAG_DAY), buf, sizeof(buf));
			int day = atoi(buf);
			wnd_get_text(wnd_with_tag(hwnd, TAG_MONTH), buf, sizeof(buf));
                        int month = atoi(buf);
			wnd_get_text(wnd_with_tag(hwnd, TAG_YEAR), buf, sizeof(buf));
                        int year = atoi(buf);
			wnd_get_text(wnd_with_tag(hwnd, TAG_HOUR), buf, sizeof(buf));
                        int hour= atoi(buf);
                        wnd_get_text(wnd_with_tag(hwnd, TAG_MINUTE), buf, sizeof(buf));
                        int minute = atoi(buf);
                        wnd_get_text(wnd_with_tag(hwnd, TAG_SECOND), buf, sizeof(buf));
                        int second= atoi(buf);

			int max_day = 31;
			switch(month) {
			case 1:
				max_day=31;
				break;
			case 2:
				max_day=28;
				if((year % 400 == 0)  || ((year % 4 == 0) && (year % 100 != 0))) {
					max_day = 29;
				}
				break;
			case 3:
				max_day=31;
				break;
			case 4:
				max_day=30;
				break;
			case 5:
				max_day=31;
				break;
			case 6:
				max_day=30;
				break;
			case 7:
				max_day=31;
				break;
			case 8:
				max_day=31;
				break;
			case 9:
				max_day=30;
				break;
			case 10:
				max_day=31;
				break;
			case 11:
				max_day=30;
				break;
			case 12:
				max_day=31;
				break;
			}//

			if(day>max_day) {
				show_alert("Неверно указан день");
				break;
			}



                        RTC_DateTypeDef d;
                        d.RTC_Year = year - 2000;
                        d.RTC_Month = month;
                        d.RTC_Date = day;
                        d.RTC_WeekDay = RTC_Weekday_Tuesday;

			RTC_TimeTypeDef t;
                        t.RTC_Hours = hour;
                        t.RTC_Minutes = minute;
                        t.RTC_Seconds = second;

                        PWR_BackupAccessCmd(ENABLE);
                        RTC_WriteProtectionCmd(DISABLE);
                        RTC_SetDate(RTC_Format_BIN, &d);
                        RTC_SetTime(RTC_Format_BIN, &t);
                        RTC_WriteProtectionCmd(ENABLE);
                        PWR_BackupAccessCmd(DISABLE);

			wnd_destroy(hwnd);
		} break;

		default:
			wnd_default_proc(hwnd, msg, p1, p2);
		}
	} break;

	default:
		wnd_default_proc(hwnd, msg, p1, p2);
	}
}

WND* show_set_time() {
	WND* hwnd = wnd_create(FORM);

	wnd_set_frame(hwnd, 0, 0, LCD_WIDTH, LCD_HEIGHT);
        wnd_set_proc(hwnd, &set_time_wnd_proc);
        wnd_set_text(hwnd, "Установка даты и времени");
        wnd_set_text(hwnd->form->ok, "Сохранить");
        hwnd->form->ok->tag = CMD_OK;
        wnd_set_text(hwnd->form->cancel, "Отмена");
        wnd_add_subview(SCREEN, hwnd);

        WND* date_label = wnd_create(LABEL);
        wnd_set_font(date_label, font_large);
        wnd_set_text(date_label, "Дата: ");
        wnd_add_subview(hwnd, date_label);

        WND* year= wnd_create(LABEL);
        year->tag = TAG_YEAR;
        wnd_set_font(year, font_large);
        wnd_set_text_alignment(year, left);
        wnd_set_text(year, "2000");
        wnd_add_subview(hwnd,year);
	wnd_set_flags(year, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
	year->bgcolor = WHITE;

	WND* month= wnd_create(LABEL);
        month->tag = TAG_MONTH;
        wnd_set_font(month, font_large);
        wnd_set_text_alignment(month, left);
        wnd_set_text(month, "01");
        wnd_add_subview(hwnd, month);
 	wnd_set_flags(month, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
        month->bgcolor = WHITE;


	WND* day=wnd_create(LABEL);
        day->tag = TAG_DAY;
        wnd_set_font(day, font_large);
        wnd_set_text_alignment(day, left);
        wnd_set_text(day, "01");
        wnd_add_subview(hwnd, day);
	wnd_set_flags(day, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
        day->bgcolor = WHITE;

	WND* time_label = wnd_create(LABEL);
        wnd_set_font(time_label, font_large);
        wnd_set_text(time_label, "Время: ");
        wnd_add_subview(hwnd, time_label);
	wnd_set_flags(day, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
        day->bgcolor = WHITE;


        WND* hour= wnd_create(LABEL);
        hour->tag = TAG_HOUR;
        wnd_set_font(hour, font_large);
        wnd_set_text_alignment(hour, left);
        wnd_set_text(hour, "01");
        wnd_add_subview(hwnd,hour);
	wnd_set_flags(hour, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
        hour->bgcolor = WHITE;


	WND* minute= wnd_create(LABEL);
        minute->tag = TAG_MINUTE;
        wnd_set_font(minute, font_large);
        wnd_set_text_alignment(minute, left);
        wnd_set_text(minute, "01");
        wnd_add_subview(hwnd, minute);
	wnd_set_flags(minute, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
        minute->bgcolor = WHITE;


	WND* second = wnd_create(LABEL);
        second->tag = TAG_SECOND;
        wnd_set_font(second, font_large);
        wnd_set_text_alignment(second, left);
        wnd_set_text(second, "01");
        wnd_add_subview(hwnd, second);
	wnd_set_flags(second, FLAG_FILLBG | FLAG_TOUCHES_ENABLED);
        second->bgcolor = WHITE;


	time_update(hwnd);
	
        Size label_size = wnd_size_that_fits(time_label);
        wnd_set_frame(date_label, LCD_WIDTH/2-label_size.cx, 60, label_size.cx, label_size.cy);
	wnd_set_frame(time_label, LCD_WIDTH/2-label_size.cx, 60+15+label_size.cy, label_size.cx, label_size.cy);

	Size year_size = wnd_size_that_fits(year);
	Size month_size = wnd_size_that_fits(month);
	Size day_size = wnd_size_that_fits(day);

	wnd_set_frame(day, date_label->frame.right+5, date_label->frame.top, day_size.cx, label_size.cy);
	wnd_set_frame(month, day->frame.right+10, date_label->frame.top, month_size.cx, label_size.cy);
        wnd_set_frame(year, month->frame.right+10, date_label->frame.top, year_size.cx, label_size.cy);

	Size hour_size = wnd_size_that_fits(hour);
        Size minute_size = wnd_size_that_fits(minute);
        Size second_size = wnd_size_that_fits(second);

	wnd_set_frame(hour, time_label->frame.right+5, time_label->frame.top, hour_size.cx, label_size.cy);
        wnd_set_frame(minute, hour->frame.right+10, time_label->frame.top, minute_size.cx, label_size.cy);
	wnd_set_frame(second, minute->frame.right+10, time_label->frame.top, second_size.cx, label_size.cy);

	return hwnd;
	
}

void time_update(WND* hwnd) {
        char buf[64];
	RTC_TimeTypeDef t;
        RTC_GetTime(RTC_Format_BIN, &t);
        snprintf(buf, sizeof(buf), "%0.2d", t.RTC_Hours);
        wnd_set_text(wnd_with_tag(hwnd, TAG_HOUR), buf);

        snprintf(buf, sizeof(buf), "%0.2d", t.RTC_Minutes);
        wnd_set_text(wnd_with_tag(hwnd, TAG_MINUTE), buf);

        snprintf(buf, sizeof(buf), "%0.2d", t.RTC_Seconds);
        wnd_set_text(wnd_with_tag(hwnd, TAG_SECOND), buf);

        RTC_DateTypeDef d;
        RTC_GetDate(RTC_Format_BIN, &d);

        snprintf(buf, sizeof(buf), "%0.4d", d.RTC_Year+2000);
        wnd_set_text(wnd_with_tag(hwnd, TAG_YEAR), buf);

        snprintf(buf, sizeof(buf), "%0.2d", d.RTC_Month);
        wnd_set_text(wnd_with_tag(hwnd, TAG_MONTH), buf);

        snprintf(buf, sizeof(buf), "%0.2d", d.RTC_Date);
        wnd_set_text(wnd_with_tag(hwnd, TAG_DAY), buf);


}

#include "uilib.h"
#include "utf8.h"
#include <string.h>
#include "lcd-ft800.h"
#include "utils.h"
#include <stdio.h>


#define TAG_INPUT_VAL 		374278472

#define CMD_BACK		4
#define CMD_OK			8
#define CMD_KEY_DEL		810
#define CMD_KEY_ALPHA		815
#define CMD_KEY_CAPS		816
#define CMD_KEY_LANG		817
#define CMD_KEY_SYM		818
#define CMD_KEY_RIGHT		819
#define CMD_KEY_LEFT		820

#define KEYBOARD_ALPHA	0
#define KEYBOARD_SYM	1
#define LANG_RUS	0
#define LANG_ENG	1

extern WND* SCREEN;
extern Bitmap bitmap_key_caps_black;
extern Bitmap bitmap_key_caps_green;
extern Bitmap bitmap_key_caps_white;

static INPUT_TEXT_PARAM INPUT_PARAM;

static BOOL KEYBOARD_CAPS = FALSE;
static int  KEYBOARD_TYPE = KEYBOARD_ALPHA;
static int KEYBOARD_LANG  = LANG_RUS;

static char BUF[128];

static WND* show_input_text_and_cursor(INPUT_TEXT_PARAM *param, char* text, int cursor_pos);

static int input_text_wnd_proc(WND* hwnd, int msg, int p1, int p2) {
        switch(msg) {

        case WM_COMMAND: {
                int cmd=p1;
		WND* cmdWnd = (WND*)p2;

                switch(cmd) {
		case CMD_KEY_ALPHA: {
			char* key_text = wnd_get_text(cmdWnd, 0, 0);

			print("key_text = %s\r\n", key_text);

			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
			if(val->label->selected) {
				BUF[0]=0;
				val->label->cursor = 0;
			} else {
				wnd_get_text(val, BUF, sizeof(BUF));
			}

			int max_size = sizeof(BUF);
			if(INPUT_PARAM.upper_limit>0 && INPUT_PARAM.upper_limit<max_size ) max_size = INPUT_PARAM.upper_limit;

			if(strlen(BUF) + strlen(key_text) < max_size) {
				u8_insert(BUF, sizeof(BUF), val->label->cursor, key_text);
				val->label->cursor ++;
		
			}
			wnd_set_text(val, BUF);
			val->label->selected = FALSE;
			
			wnd_set_text_alignment(val, (wnd_size_that_fits(val).cx > RECT_WIDTH(val->frame) ? right : left));

			print("input_text::CMD_KEY_ALPHA key_text=%s, cursor = %d, text=%s, clen = %d\r\n", key_text, val->label->cursor, BUF, strlen(BUF));

		} break;

		case CMD_KEY_CAPS: {
			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
			KEYBOARD_CAPS = !KEYBOARD_CAPS;
			show_input_text_and_cursor(&INPUT_PARAM, wnd_get_text(val, 0, 0), val->label->cursor);

			wnd_destroy(hwnd);
			return 0;

		} break;

		case CMD_KEY_LANG: {
                        WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);

			if(KEYBOARD_TYPE==KEYBOARD_ALPHA) {
                        	KEYBOARD_LANG = (KEYBOARD_LANG == LANG_RUS ? LANG_ENG : LANG_RUS );
			} else {
				KEYBOARD_TYPE = KEYBOARD_ALPHA;
			}

                        show_input_text_and_cursor(&INPUT_PARAM, wnd_get_text(val, 0, 0), val->label->cursor);
                        wnd_destroy(hwnd);

                        return 0;

                } break;

		case CMD_KEY_SYM: {
                        WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        KEYBOARD_TYPE = KEYBOARD_SYM;
                        show_input_text_and_cursor(&INPUT_PARAM, wnd_get_text(val, 0, 0), val->label->cursor);
                        wnd_destroy(hwnd);

			return 0;
                } break;

		case CMD_KEY_RIGHT: {
                        WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
			int len = u8_strlen(wnd_get_text(val, 0, 0));
                        if(val->label->cursor < len) {
                                val->label->cursor++;
                        } else {
				val->label->cursor = len;
			}

                        return 0;
                } break;


		case CMD_KEY_LEFT: {
                        WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
                        if(val->label->cursor > 0 ) {
                                val->label->cursor--;
                        } else {
                                val->label->cursor = 0;
                        }

                        return 0;
                } break;

		case CMD_KEY_DEL: {
			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);

			char* text = wnd_get_text(val, 0, 0);
			if(val->label->selected) {
                                //BUF[0]=0;
				if(text) text[0] = 0;
                        //} else {
                        	//wnd_get_text(val, BUF, sizeof(BUF));
			}

			if(val->label->cursor>0) {
				u8_erase(text,val->label->cursor-1,val->label->cursor);
				val->label->cursor --;
			}

                        print("input_text_wnd_proc::CMD_KEY_DEL new_text=%s, clen=%d\n", text, strlen(text));
                        //wnd_set_text(val, BUF);
			val->label->selected = FALSE;
			
			wnd_set_text_alignment(val, (wnd_size_that_fits(val).cx > RECT_WIDTH(val->frame) ? right : left));
		} break;

		case CMD_OK: {
			WND* val = wnd_with_tag(hwnd, TAG_INPUT_VAL);
			char* text = wnd_get_text(val, 0, 0);
			print("input_text_wnd_proc::CMD_OK text=%s\n", text);
			wnd_proc_call(INPUT_PARAM.hwnd, INPUT_PARAM.msg, INPUT_PARAM.param_id, (int)text);
			wnd_destroy(hwnd);

			return 0;
		} break;

		case CMD_BACK: {
                        //wnd_proc_call(hwnd->superview, WM_UPDATE, 0, 0);
                        wnd_destroy(hwnd);
			print("input_text_wnd_proc::CMD_BACK \n");
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



WND* show_input_text_and_cursor(INPUT_TEXT_PARAM *param, char* text, int cursor_pos) {
	print("show_input_text\n");
	INPUT_PARAM = *param;

        WND* hwnd=wnd_create(FORM);
        wnd_set_frame(hwnd, 0, 0, LCD_WIDTH, LCD_HEIGHT);
        wnd_set_proc(hwnd, &input_text_wnd_proc);
        wnd_set_text(hwnd, param->title);
	if(param->ok_text) wnd_set_text(hwnd->form->ok, param->ok_text);
	hwnd->form->ok->tag = CMD_OK;
        if(param->cancel_text) wnd_set_text(hwnd->form->cancel, param->cancel_text);
	hwnd->form->cancel->tag = CMD_BACK;
        wnd_add_subview(SCREEN, hwnd);

	WND* valuebg = wnd_create(WINDOW);
        valuebg->bgcolor = WHITE;
        wnd_add_subview(hwnd,valuebg);

        WND* value= wnd_create(LABEL);
        value->tag = TAG_INPUT_VAL;
        wnd_set_font(value, font_large);
        wnd_set_text(value, text);
        value->label->selected = FALSE;
	value->label->cursor = cursor_pos;
        wnd_add_subview(hwnd,value);

        wnd_set_frame(valuebg, 0, 40, LCD_WIDTH-55, 30+10);
        wnd_set_frame(value, 5, 45, LCD_WIDTH-55-10, 30);

	wnd_set_text_alignment(value, (wnd_size_that_fits(value).cx > RECT_WIDTH(value->frame) ? right : left));

        Size bs= MakeSize(43, 35);
        int dy = 82;
	int dx = 0;

	WND* btn;

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_DEL;
        wnd_set_text(btn, "DEL");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, LCD_WIDTH - 55, 40, 55, 40);


	if (KEYBOARD_TYPE == KEYBOARD_ALPHA && KEYBOARD_LANG == LANG_RUS) {
		char* t1u[]={"Й","Ц","У","К","Е","Н","Г","Ш","Щ","З","Х"};
		char* t1l[]={"й","ц","у","к","е","н","г","ш","щ","з","х"};

        	dx = (LCD_WIDTH - bs.cx*SIZE(t1u))/2;
		for(int i=0; i<SIZE(t1u); i++) {
			btn= wnd_create(BUTTON);
        		btn->tag = CMD_KEY_ALPHA;
        		wnd_set_text(btn, (KEYBOARD_CAPS ? t1u[i] : t1l[i]));
        		wnd_add_subview(hwnd, btn);
        		wnd_set_frame(btn, dx+i*bs.cx, dy, bs.cx, bs.cy);
		}

		dy+=bs.cy;

		char* t2u[]={"Ф","Ы","В","А","П","Р","О","Л","Д","Ж","Э"};
		char* t2l[]={"ф","ы","в","а","п","р","о","л","д","ж","э"};

		dx = (LCD_WIDTH - bs.cx*SIZE(t2u))/2;
        	for(int i=0; i<SIZE(t2u); i++) {
                	btn= wnd_create(BUTTON);
                	btn->tag = CMD_KEY_ALPHA;
                	wnd_set_text(btn, (KEYBOARD_CAPS ? t2u[i] : t2l[i]));
                	wnd_add_subview(hwnd, btn);
                	wnd_set_frame(btn, dx+i*bs.cx, dy, bs.cx, bs.cy);
        	}

		dy+=bs.cy;

		char* t3u[]={"Я","Ч","С","М","И","Т","Ь","Б","Ю","Ъ"};
		char* t3l[]={"я","ч","с","м","и","т","ь","б","ю","ъ"};

		dx = (LCD_WIDTH - bs.cx*SIZE(t3u))/2;
        	for(int i=0; i<SIZE(t3u); i++) {
                	btn= wnd_create(BUTTON);
                	btn->tag = CMD_KEY_ALPHA;
                	wnd_set_text(btn, (KEYBOARD_CAPS ? t3u[i] : t3l[i]));
                	wnd_add_subview(hwnd, btn);
                	wnd_set_frame(btn, dx+i*bs.cx, dy, bs.cx, bs.cy);
        	}

		dy+=bs.cy;

	} else if(KEYBOARD_TYPE == KEYBOARD_ALPHA && KEYBOARD_LANG == LANG_ENG) {

		char* t1u[]={"Q","W","E","R","T","Y","U","I","O","P"};
        	char* t1l[]={"q","w","e","r","t","y","u","i","o","p"};

		dx = (LCD_WIDTH - bs.cx*SIZE(t1u))/2;
		for(int i=0; i<SIZE(t1u); i++) {
                        btn= wnd_create(BUTTON);
                        btn->tag = CMD_KEY_ALPHA;
                        wnd_set_text(btn, (KEYBOARD_CAPS ? t1u[i] : t1l[i]));
                        wnd_add_subview(hwnd, btn);
                        wnd_set_frame(btn, dx+i*bs.cx, dy, bs.cx, bs.cy);
                }

                dy+=bs.cy;

        	char* t2u[]={"A","S","D","F","G","H","J","K","L"};
		char* t2l[]={"a","s","d","f","g","h","j","k","l"};

		dx = (LCD_WIDTH - bs.cx*SIZE(t2u))/2;
		for(int i=0; i<SIZE(t2u); i++) {
                        btn= wnd_create(BUTTON);
                        btn->tag = CMD_KEY_ALPHA;
                        wnd_set_text(btn, (KEYBOARD_CAPS ? t2u[i] : t2l[i]));
                        wnd_add_subview(hwnd, btn);
                        wnd_set_frame(btn, dx+i*bs.cx, dy, bs.cx, bs.cy);
                }

                dy+=bs.cy;


		char* t3u[]={"Z","X","C","V","B","N","M"};
                char* t3l[]={"z","x","c","v","b","n","m"};

		dx = (LCD_WIDTH - bs.cx*SIZE(t3u))/2;
                for(int i=0; i<SIZE(t3u); i++) {
                        btn= wnd_create(BUTTON);
                        btn->tag = CMD_KEY_ALPHA;
                        wnd_set_text(btn, (KEYBOARD_CAPS ? t3u[i] : t3l[i]));
                        wnd_add_subview(hwnd, btn);
                        wnd_set_frame(btn, dx+i*bs.cx, dy, bs.cx, bs.cy);
                }

                dy+=bs.cy;

	} else { //if(KEYBOARD_TYPE == KEYBOARD_SYM) {
		
		char* t1[]={"1","2","3","4","5","6","7","8","9","0"};
		dx = (LCD_WIDTH - bs.cx*SIZE(t1))/2;
        	for(int i=0; i<SIZE(t1); i++) {
                	btn= wnd_create(BUTTON);
                	btn->tag = CMD_KEY_ALPHA;
                	wnd_set_text(btn, t1[i]);
                	wnd_add_subview(hwnd, btn);
                	wnd_set_frame(btn, dx+i*bs.cx, dy, bs.cx, bs.cy);
        	}

		dy+=bs.cy;

		char* t2[]={"!","@","#","$","%","^","*","(",")","?"};
		dx = (LCD_WIDTH - bs.cx*SIZE(t2))/2;
                for(int i=0; i<SIZE(t2); i++) {
                        btn= wnd_create(BUTTON);
                        btn->tag = CMD_KEY_ALPHA;
                        wnd_set_text(btn, t2[i]);
                        wnd_add_subview(hwnd, btn);
                        wnd_set_frame(btn, dx+i*bs.cx, dy, bs.cx, bs.cy);
                }

		dy+=bs.cy;

		char* t3[]={"_","=","+",":",";","[","]","'","|","\"", "/"};
		dx = (LCD_WIDTH - bs.cx*SIZE(t3))/2;
                for(int i=0; i<SIZE(t3); i++) {
                        btn= wnd_create(BUTTON);
                        btn->tag = CMD_KEY_ALPHA;
                        wnd_set_text(btn, t3[i]);
                        wnd_add_subview(hwnd, btn);
                        wnd_set_frame(btn, dx+i*bs.cx, dy, bs.cx, bs.cy);
                }

		dy+=bs.cy;

	}

	//last row
	dx = (LCD_WIDTH - bs.cx*11)/2;

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_CAPS;
	btn->button->icon->image->bitmap = (KEYBOARD_CAPS ? &bitmap_key_caps_green : &bitmap_key_caps_black);
	btn->button->icon->image->highlighted_bitmap = &bitmap_key_caps_white;
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx, dy, bs.cx, bs.cy);

	dx+=RECT_WIDTH(btn->frame);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_LANG;
        wnd_set_text(btn, "ENG/РУС");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx, dy, 2*bs.cx, bs.cy);

        dx+=RECT_WIDTH(btn->frame);


	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_ALPHA;
        wnd_set_text(btn, " ");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx, dy, 2*bs.cx, bs.cy);

	dx+=RECT_WIDTH(btn->frame);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_SYM;
        wnd_set_text(btn, "123");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx, dy, bs.cx, bs.cy);	

        dx+=RECT_WIDTH(btn->frame);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_ALPHA;
        wnd_set_text(btn, ".");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx, dy, bs.cx, bs.cy);

	dx+=RECT_WIDTH(btn->frame);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_ALPHA;
        wnd_set_text(btn, ",");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx, dy, bs.cx, bs.cy);

        dx+=RECT_WIDTH(btn->frame);

	btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_ALPHA;
        wnd_set_text(btn, "-");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx, dy, bs.cx, bs.cy);

	dx+=RECT_WIDTH(btn->frame);

        btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_LEFT;
        wnd_set_text(btn, "<");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx, dy, bs.cx, bs.cy);

	dx+=RECT_WIDTH(btn->frame);

        btn= wnd_create(BUTTON);
        btn->tag = CMD_KEY_RIGHT;
        wnd_set_text(btn, ">");
        wnd_add_subview(hwnd, btn);
        wnd_set_frame(btn, dx, dy, bs.cx, bs.cy);

	memset(BUF, 0, sizeof(BUF));

	print("show_input_text::end \n");
	return hwnd;

}

WND* show_input_text(INPUT_TEXT_PARAM *param, char* text) {
	return show_input_text_and_cursor(param, text, u8_strlen(text));
}

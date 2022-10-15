/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef _WND_UTILS_H_
#define _WND_UTILS_H_

#include "wnd_common.h"

int gc_draw_wnd_text(Context* gc, const char* s, int len, Font *font, Color rgb, Rect* rect, int cursor_pos, TextAlignment alignment);
Size wnd_get_text_size(Font* font, char* text);
Point wnd_touch2screen_pt(TOUCH2SCREEN_PARAM *param);

#endif //_WND_UTILS_H_

#ifdef WND_IMPL
#ifndef _WND_UTILS_H_IMPL_
#define _WND_UTILS_H_IMPL_

int gc_draw_wnd_text(Context* gc, const char* s, int len, Font *font, Color rgb, Rect* rect, int cursor_pos, TextAlignment alignment) {
	// gc is optional
	// rect is IN and OUT param
	// cursor_pos (optional) IN = real cursor pos +1

	if(s==NULL || len==0 || font==NULL || rect==NULL) {
		//print("wnd_draw_u16text failed: string or font or rect is null\n");
		return 0;
	}

	int width;
	int height;

	if(RECT_WIDTH(*rect)>0) {
		width = RECT_WIDTH(*rect);
	} else {
		width = 20000;
		alignment = left;
	}

	if(RECT_HEIGHT(*rect)>0) {
                height = RECT_HEIGHT(*rect);
        } else {
                height = 20000;
        }

	int max_line_width=0;
	int line_width  = 0;
	int pos_start = 0;

	int x = rect->left;
	int y = rect->top;
	int i=0;
	int prev_i=0;

	BOOL find_cursor = (cursor_pos-1 >= 0);
	int count = 0;

	if(gc) gc_set_font(gc, font);

	pixel_font_symbol symbol;

	while(i<len) {
		prev_i = i;
		int c = u8_nextchar(s, &i);
                if(c==0) break;

		count++;
		if(find_cursor && count > cursor_pos-1) {
			find_cursor = FALSE;

			if(gc) gc_fill_rect(gc, x+line_width-1, y, 2, font->height, GREEN);
		}	

		symbol=get_pixel_font_symbol(font, c);

		if(c=='\n') {
			if(alignment == right) x += width - line_width;
			else if(alignment == center) x += (width - line_width)/2;

			if(gc) gc_draw_text(gc, (char*)s+pos_start, (i - pos_start), x, y, rgb);
                        if(line_width>max_line_width) max_line_width = line_width;
                        pos_start = i;
                        line_width = 0;
                        y += font->height;
                        x = rect->left;

                        if(y>height) {
                                pos_start = len;
                                break;
                        }

		} else if(line_width + symbol.symbol_width > width) {
			if(alignment == right) x += width - line_width;
                        else if(alignment == center) x += (width - line_width)/2;

			if(gc) gc_draw_text(gc, (char*)s+pos_start, (prev_i - pos_start), x, y, rgb);
			if(line_width>max_line_width) max_line_width = line_width;
			pos_start = prev_i;
			line_width = symbol.symbol_width;
			y += font->height;
			x = rect->left;

			if(y>height) {
				pos_start = len;
				break;
			}
		} else {
			line_width += symbol.symbol_width;
		}
	}

	
	if(pos_start < len) {
		if(alignment == right) x += width - line_width;
                else if(alignment == center) x += (width - line_width)/2;

		if(gc) gc_draw_text(gc, (char*)s+pos_start, (len - pos_start), x, y, rgb);
		if(line_width>max_line_width) max_line_width = line_width;
	}


	SetRect(rect, rect->left, rect->top, rect->left+max_line_width, y+font->height);

	if(find_cursor) {
		if(gc) gc_fill_rect(gc, x+line_width-1, y, 2, font->height, GREEN);
        }

	return 0;
}

Size wnd_get_text_size(Font* font, char* text) {
        Rect rect=ZeroRect();
        gc_draw_wnd_text(NULL, text, strlen(text), font, BLACK, &rect, 0, 0);
        return MakeSize(RECT_WIDTH(rect), RECT_HEIGHT(rect));
}


Point wnd_touch2screen_pt(TOUCH2SCREEN_PARAM *p) {
        int x=(int)(p->KX1 * p->touch_x + p->KX2 * p->touch_y + p->KX3+0.5);
        int y=(int)(p->KY1 * p->touch_x + p->KY2 * p->touch_y + p->KY3+0.5);

        if(x<0) x=0; else if (x > p->screen_width) x=p->screen_width;
        if(y<0) y=0; else if (y > p->screen_height) y=p->screen_height;

        return MakePoint(x,y);
}

#endif //_WND_UTILS_H_IMPL_
#endif //WND_IMPL

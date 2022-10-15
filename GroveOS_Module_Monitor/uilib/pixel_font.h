/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#ifndef _PIXEL_FONT_H_
#define _PIXEL_FONT_H_

typedef struct {
        int symbol;
        int symbol_width;
        int symbol_index;
} pixel_font_symbol ;

typedef struct {
        char* name;
        int height;
	int ascent;
        unsigned char* symbols_data;
	int symbols_data_len;
        pixel_font_symbol* symbols_info;
	int symbols_info_len;
} Font;

pixel_font_symbol get_pixel_font_symbol(Font* font, int c);

typedef enum {
        medium,
        small,
        large,
} FontSize;

#endif //_PIXEL_FONT_H_

#ifdef PIXEL_FONT_IMPL

#ifndef _PIXEL_FONT_H_IMPL_
#define _PIXEL_FONT_H_IMPL_

pixel_font_symbol get_pixel_font_symbol(Font* font, int c) {

        pixel_font_symbol result;

	if(c>=0x20 && c<=0x7f) {
		result = font->symbols_info[c-0x20];
	} else if(c>=0x410&& c<=0x44f) {
		result = font->symbols_info[(c-0x410)+(0x7f-0x20+1)];
	} else { 
		result = font->symbols_info['?'-0x20]; 
	}

        return result;
}

#endif //_PIXEL_FONT_H_IMPL_
#endif //PIXEL_FONT_IMPL

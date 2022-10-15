/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef ___FBQUEUE_H___
#define ___FBQUEUE_H___

#include "stm32f4xx.h"
#include "lcd-ft800.h"

#define FRAMEBUFFER_WIDTH (LCD_WIDTH/2)
#define FRAMEBUFFER_HEIGHT (LCD_HEIGHT/2)

#define FRAMEBUFFER_SIZE_BYTES  (FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * 1)

typedef struct {
        unsigned char framebuffer[FRAMEBUFFER_WIDTH*FRAMEBUFFER_HEIGHT];
        int img_index;
} Fbinfo;

void fbqueue_init();
int fbqueue_len();
int fbqueue_enqueue(Fbinfo *info);
Fbinfo* fbqueue_dequeue();
int fbqueue_isempty();
int fbqueue_isfull();
Fbinfo* fbqueue_get_head();
Fbinfo* fbqueue_get_tail();
void fbqueue_clear();


#endif

/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef __OLED_H__
#define __OleD_H__

#define OLED_LINE_WIDTH  100             // WEG010032 has 4 lines each of 100 pixels width
#define OLED_FONT_WIDTH  6               // Built-in font has 6 pixels width


#define	OLED_RW(X) gpio_set(GPIOB, GPIO_Pin_10, X);
#define	OLED_RS(X) gpio_set(GPIOE, GPIO_Pin_15, X);
#define	OLED_E(X) gpio_set(GPIOD, GPIO_Pin_8, X);
#define	OLED_CS1(X) gpio_set(GPIOD, GPIO_Pin_11, X);
#define	OLED_CS2(X) gpio_set(GPIOD, GPIO_Pin_10, X);



int oled_init(void);
void OLED_WEG010032_BLIT(uint16_t size, char *buf, int16_t x, int16_t y);
void OLED_WEG010032_PRINT(uint16_t size, char *buf, int16_t x, int16_t y);
void OLED_WEG010032_CLEAR(void);

#endif // __OLED_H__

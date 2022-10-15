/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#ifndef _BITMAP_H_
#define _BITMAP_H_

typedef struct {
	int width;
	int height;
	unsigned char* data;//rgb
	int data_size;
	unsigned char* alpha_data;
	int alpha_size;
} Bitmap;

#endif

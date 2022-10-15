/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef __EXT_GPIO_H__
#define __EXT_GPIO_H__


typedef struct _EXT_GPIO {
	unsigned int periph;
	void* port;
	unsigned int pin;
	int mode; 
	int irq_port_src; 
	int irq_pin_src; 
	int irq_line; 
	int irq_chan; 
	int irq_raisfall; 
	char *descr;
} EXT_GPIO;

extern EXT_GPIO ext_gpios[];

int ext_gpio_init(void);
int ext_gpio_get(unsigned int gpio_num);
int ext_gpio_set(unsigned int gpio_num, int value);
int ext_gpio_irq(unsigned int gpio_num, int onoff);
	
#endif

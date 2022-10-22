/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "ext_irq.h"
#include <stm32f4xx_exti.h>
#include <string.h>
#include "utils.h"


struct EXTIRQ_PROC_DATA irq_proc_list[IRQ_LIST_SIZE];


extern uint8_t event_logging;

static int irq_line_index(int irq_line) {
	switch(irq_line) {
	case EXTI_Line0: return 0;
	case EXTI_Line1: return 1;
	case EXTI_Line2: return 2;
	case EXTI_Line3: return 3;
	case EXTI_Line4: return 4;
	case EXTI_Line5: return 5;
	case EXTI_Line6: return 6;
	case EXTI_Line7: return 7;
	case EXTI_Line8: return 8;
	case EXTI_Line9: return 9;
	case EXTI_Line10: return 10;
	case EXTI_Line11: return 11;
	case EXTI_Line12: return 12;
	case EXTI_Line13: return 13;
	case EXTI_Line14: return 14;
	case EXTI_Line15: return 15;
	default:
		return -1;
	}
}

void ext_irq_init(void) {
	ext_irq_clear_all(NULL);
}
		
int ext_irq_set(int irq_line, EXTIRQ_PROC proc, void* owner) {
	int index = irq_line_index(irq_line);
	if(index<0) return -1;

	if(irq_proc_list[index].owner != owner && owner != NULL && irq_proc_list[index].owner != NULL) {
		return -2; // IRQ is taken by some application
	}

	irq_proc_list[index].proc = proc;
	irq_proc_list[index].owner = owner;
	return 0;
}

int ext_irq_clear(int irq_line, void* owner) {
        int index = irq_line_index(irq_line);
        if(index<0) return -1;

	if(owner && owner != irq_proc_list[index].owner) {
		return -2; // already taken
	}

        irq_proc_list[index].proc = NULL;
        irq_proc_list[index].owner = NULL;

	return 0;
}

void ext_irq_clear_all(void* owner) {
	for(int i=0; i<IRQ_LIST_SIZE; i++) {
		if(irq_proc_list[i].owner == owner || owner == NULL) {
			irq_proc_list[i].owner = NULL;
			irq_proc_list[i].proc = NULL;
		}
	}
}

void ext_irq_call_proc(int exti_line)
{
	if(event_logging > 1)
		_print("ext_irq_call_proc() IRQ EXTI_Line_%d\r\n", irq_line_index(exti_line));

	EXTIRQ_PROC proc=ext_irq_get(exti_line);
	if(proc) {
		if(event_logging >1)
			_print("ext_irq_call_proc() Calling proc = %p\r\n", irq_line_index(exti_line));
		proc(exti_line);
	}
}

EXTIRQ_PROC ext_irq_get(int irq_line) {
	int index = irq_line_index(irq_line);
        if(index<0) return NULL;

	return irq_proc_list[index].proc;
}

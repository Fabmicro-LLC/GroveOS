/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "listener.h"
#include <string.h>
#include "utils.h"

#define LISTENERS_SIZE 10

typedef struct {
	int msg;
	LISTENER_PROC proc;
} LISTENER_INFO;

static LISTENER_INFO listeners[LISTENERS_SIZE];

void listener_init() {
	for(int i=0; i<LISTENERS_SIZE; i++) {
                listeners[i].msg = 0;
                listeners[i].proc = 0;
        }
}
		
int listener_set(int msg, LISTENER_PROC proc) {
	print("listener::listener_set proc=%p, msg=%d\r\n", proc, msg);
	int free_slot=-1;
	for(int i=0; i<LISTENERS_SIZE; i++) {
		if(listeners[i].msg == msg && listeners[i].proc == proc) {
			return 0;
		}
		if(listeners[i].proc == 0) free_slot = i;
	}

	if(free_slot == -1) {
		print("listener_set failed, no free slots found!, msg=%d\r\n", msg);
		return -1;
	}

	listeners[free_slot].msg = msg;
	listeners[free_slot].proc = proc;

	print("listener::listener_set::end msg=%d, proc=%p, free_slot=%d\r\n", msg, proc, free_slot);

	return 0;
}

void listener_remove(int msg, LISTENER_PROC proc) {
	print("listener::listener_remove proc=%p\r\n", proc);
	for(int i=0; i<LISTENERS_SIZE; i++) {
                if(listeners[i].msg == msg && listeners[i].proc == proc) {
			listeners[i].msg = 0;
			listeners[i].proc = 0;
			break;
		}
	}
}


void listener_update(int msg, int p1, int p2) {
	//print("listener::listener_update msg=%d\r\n", msg);
	for(int i=0; i<LISTENERS_SIZE; i++) {

		//if(listeners[i].msg!=0) print("listener::listener_update i=%d, msg=%d, proc=%p\r\n", i, listeners[i].msg, listeners[i].proc);

                if(listeners[i].msg == msg && listeners[i].proc != 0) {
			listeners[i].proc(msg, p1, p2);
                }
        }
}

void listener_clear_all() {
	for(int i=0; i<LISTENERS_SIZE; i++) {
                listeners[i].msg = 0;
                listeners[i].proc = 0;
        }
}	

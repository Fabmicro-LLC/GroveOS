/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "fbqueue.h"
#include "utils.h"

#define FBQUEUE_MAX_SIZE 2

static Fbinfo fbqueue_array[FBQUEUE_MAX_SIZE];
static Fbinfo* fbqueue[FBQUEUE_MAX_SIZE];
static int fbqueue_head;
static int fbqueue_tail;


void fbqueue_init() {
	fbqueue_tail = fbqueue_head = 0;
}


int fbqueue_enqueue(Fbinfo* msg) {
        if(fbqueue_isfull()) {
                return -1;
        }

        fbqueue[fbqueue_tail % FBQUEUE_MAX_SIZE] = msg;
        fbqueue_tail++;
        return 0;
}

Fbinfo* fbqueue_dequeue() {
        if(fbqueue_isempty()) {
                return NULL;
        } else {
                Fbinfo* result=fbqueue[fbqueue_head % FBQUEUE_MAX_SIZE];
                fbqueue_head++;
                return result;
        }
}

void fbqueue_clear() {
        fbqueue_head = fbqueue_tail = 0;
}

Fbinfo* fbqueue_get_head() {
        if(fbqueue_isempty()) {
                return NULL;
        } else {
                return &fbqueue_array[fbqueue_head % FBQUEUE_MAX_SIZE];
        }
}

Fbinfo* fbqueue_get_tail() {
        if(fbqueue_isfull()) {
                return NULL;
        } else {
                return &fbqueue_array[fbqueue_tail % FBQUEUE_MAX_SIZE];
        }
}
int fbqueue_isempty() {
        return (fbqueue_head == fbqueue_tail);
}
int fbqueue_isfull() {
        return ((fbqueue_tail - FBQUEUE_MAX_SIZE) == fbqueue_head);
}

int fbqueue_len() {
	return (fbqueue_tail - fbqueue_head);
}

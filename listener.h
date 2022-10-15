/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef _LISTENER_H_
#define _LISTENER_H_

typedef void (*LISTENER_PROC)(int msg, int p1, int p2);

#ifndef SVC_CLIENT

void listener_init();
int listener_set(int msg, LISTENER_PROC proc);
void listener_remove(int msg, LISTENER_PROC proc);
void listener_update(int msg, int p1, int p2);
void listener_clear_all();
#else

int svc_listener_set(int msg, LISTENER_PROC proc);
void svc_listener_remove(int msg, LISTENER_PROC proc);

#endif //SVC_CLIENT

#endif //_LISTENER_H_

#ifdef SVC_CLIENT_IMPL

#ifndef _LISTENER_H_IMPL_
#define _LISTENER_H_IMPL_

#include "svc.h"

__attribute__ ((noinline)) int svc_listener_set(int msg, LISTENER_PROC proc) {
        svc(SVC_LISTENER_SET);
}

__attribute__ ((noinline)) void svc_listener_remove(int msg, LISTENER_PROC proc) {
        svc(SVC_LISTENER_REMOVE);
}

#endif //_LISTENER_H_IMPL_
#endif //SVC_CLIENT_IMPL

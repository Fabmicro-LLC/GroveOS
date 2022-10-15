/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef _EXT_PWM_H_
#define _EXT_PWM_H_

#define PWM_PORT_1               0
#define PWM_PORT_2               1
#define PWM_PORT_3               2
#define PWM_PORT_4               3
#define PWM_PORT_5               4
#define PWM_PORT_6               5
#define PWM_PORT_7               6
#define PWM_PORT_8               7


#ifndef SVC_CLIENT

int ext_pwm_set_clock(int port, int clock);
float ext_pwm_get(int port);
int ext_pwm_set(int port, float pwm);

#else

int svc_pwm_set_clock(int port, int clock);

#endif //SVC_CLIENT

#endif //_EXT_PWM_H_

#ifdef SVC_CLIENT_IMPL

#ifndef _EXT_PWM_H_IMPL_
#define _EXT_PWM_H_IMPL_

#include "svc.h"

// Set any port PWM value
__attribute__ ((noinline)) int svc_pwm_set(int port_num, float pwm)
{
        svc(SVC_SET_PWM);
}

// Set DC port PWM value
__attribute__ ((noinline)) int svc_pwm_set_clock(int port_num, int clock)
{
        svc(SVC_SET_PWM_CLOCK);
}

// Read any port PWM value
__attribute__ ((noinline)) float svc_pwm_get(int port_num)
{
        svc(SVC_READ_PWM);
        asm volatile ("vmov s0, r0\n");
}



#endif //_EXT_PWM_H_IMPL_
#endif //SVC_CLIENT_IMPL

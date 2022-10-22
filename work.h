/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef ___WORK_H___
#define ___WORK_H___

void init();
void prompt(void);
void process_message(MSG* msg);
void stop_application(void* application_text);

#define	MONITOR_INVOKE		5	// microsecons to hold UP and DOWN keys to run zmodem
#define	APPLICATION_MAX_MEM	16	// Max numbe rof memory blocks available for application. Each block is one malloc()
#define	MODBUS_SECURITY_KEY	0x01234567	// Use this key when initiating Modbus TFS requests


#endif

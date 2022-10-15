/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef __VAULT_H__
#define __VAULT_H__


// returns number of variables deleted or error
int vault_del_var(char* app_name, char *var_name);

// return number of variables deleted or error
int vault_del_var_by_app(char* app_name);

// return number of bytes read or error
int vault_get_var(char* app_name, char *var_name, void **data);

// return number of bytes written or error
int vault_set_var(char* app_name, char *var_name, void *data, int size, int offset);

int vault_enum(int idx, char *app_name, char *var_name, int* size);

void vault_erase(void);

#endif // __VAULT_H__

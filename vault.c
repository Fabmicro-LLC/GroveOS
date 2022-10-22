/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "utils.h"
#include "vault.h"
#include "config.h"

int vault_del_var_by_app(char* app_name)
{
	if(app_name == NULL)
		return -1;

	int del = 0;

	for(int i = 0; i < VAULT_INDEX_SIZE; i++) {
		if(config_active.vault_index[i].size != 0) {
			if(strncasecmp(config_active.vault_index[i].app_name, app_name, VAULT_VAR_NAME_LEN) == 0) {
				// variable with this name has been found
				config_active.vault_index[i].size = 0;
				del++;
			}
		}
	}

	if(del) {
		save_config();
		return del;
	}

	return -4; // variable not found
}



int vault_del_var(char* app_name, char *var_name)
{
	if(app_name == NULL || var_name == NULL)
		return -1;

	
	int del = 0;

	for(int i = 0; i < VAULT_INDEX_SIZE; i++) {
		if(config_active.vault_index[i].size != 0) {
			if(strncasecmp(config_active.vault_index[i].app_name, app_name, VAULT_VAR_NAME_LEN) == 0
			&& strncasecmp(config_active.vault_index[i].var_name, var_name, VAULT_VAR_NAME_LEN) == 0) {
				// variable with this name has been found
				config_active.vault_index[i].size = 0;
				del++;
			}
		}
	}

	if(del) {
		save_config();
		return del;
	}

	return -4; // variable not found
}



void vault_erase(void)
{
	memset(config_active.vault_index, 0, sizeof(config_active.vault_index));
	save_config();
}



int vault_get_var(char* app_name, char *var_name, void **data)
{
	if(app_name == NULL || var_name == NULL)
		return -1;

	

	for(int i = 0; i < VAULT_INDEX_SIZE; i++) {
		if(config_active.vault_index[i].size != 0) {
			if(strncasecmp(config_active.vault_index[i].app_name, app_name, VAULT_VAR_NAME_LEN) == 0
			&& strncasecmp(config_active.vault_index[i].var_name, var_name, VAULT_VAR_NAME_LEN) == 0) {
				// variable with this name has been found
				if(config_active.vault_index[i].backup_sram_offset + config_active.vault_index[i].size > 4096) {
					// bogus variable, should be deleted
					return -2; 
				}

				*data = get_backup_sram_ptr(config_active.vault_index[i].backup_sram_offset);
				return config_active.vault_index[i].size; // OK
				
			}
		}
	}

	return -4; // variable not found
}


int vault_set_var(char* app_name, char *var_name, void *data, int size, int offset)
{
	if(app_name == NULL || var_name == NULL)
		return -1;


	for(int i = 0; i < VAULT_INDEX_SIZE; i++) {
		if(config_active.vault_index[i].size != 0) {
			if(strncasecmp(config_active.vault_index[i].app_name, app_name, VAULT_VAR_NAME_LEN) == 0
			&& strncasecmp(config_active.vault_index[i].var_name, var_name, VAULT_VAR_NAME_LEN) == 0) {
				// variable with this name has been found
				if(config_active.vault_index[i].backup_sram_offset + config_active.vault_index[i].size > 4096 || config_active.vault_index[i].backup_sram_offset + offset > 4096) {

					print("vault_set_var() bogus variable: backup_sram_offset = %d, sram_size = %d, offset = %d\r\n", config_active.vault_index[i].backup_sram_offset, config_active.vault_index[i].size, offset);
					// bogus variable, should be deleted
					return -2; 
				}

				if(size > config_active.vault_index[i].size) {
					// too much data to write
					return -5;
				}

				if(write_to_backup_sram((char*)data, size, config_active.vault_index[i].backup_sram_offset + offset ) != size) {
					// failed to write to backup sram
					return -3;
				} 

				return size; // OK
			}
		}
	}

	// Variable with this name does not exist, try to create one

	int i,j, k, idx;

	idx = -1;

	for(j = 0; j < 4096; j++) { // iterate through backup sram address space
		// find if this addr belongs to anyone

		for(k = 0; k < size+offset; k++) {
			if(k+j >= 4096) {
				// no space available for data
				return -6;
			}

			for(i = 0; i < VAULT_INDEX_SIZE; i++) {
				if(
					(j+k >= config_active.vault_index[i].backup_sram_offset) && 
					(j+k < config_active.vault_index[i].backup_sram_offset+config_active.vault_index[i].size)) {
					// this space is taken
					goto try_next_space;
				}

				// this byte is free

				if(idx == -1 && config_active.vault_index[i].size == 0) 
					idx = i; // will use this index
			}
		}

		// we have found room for this data

		if(idx == -1) {
			// something went wrong, free index was not found
			return -8;
		}

		if(write_to_backup_sram((char*)data, size, j+offset) != size) {
			// failed to write to backup sram
			print("vault_set_var() failed to write to backup sram, data = %p, size = %d, j = %d, offset = %d\r\n", data, size, j, offset);
			return -9;
		} 

		memset(config_active.vault_index[idx].app_name, 0, VAULT_VAR_NAME_LEN);
		strncpy(config_active.vault_index[idx].app_name, app_name, VAULT_VAR_NAME_LEN);

		memset(config_active.vault_index[idx].var_name, 0, VAULT_VAR_NAME_LEN);
		strncpy(config_active.vault_index[idx].var_name, var_name, VAULT_VAR_NAME_LEN);

		config_active.vault_index[idx].backup_sram_offset = j;
		config_active.vault_index[idx].size = size+offset;

		save_config();

		return size;

		try_next_space:
		;
	}

	return -7; // no free space found
}


int vault_enum(int idx, char *app_name, char *var_name, int* size)
{

	if(app_name == NULL || var_name == NULL)
		return -1; // bogus parameter

	if(idx >= VAULT_INDEX_SIZE)
		return -2; // out of range

	//print("vault_enum() %p %p %d\r\n", app_name, config_active.vault_index[idx].app_name, idx);

	strncpy(app_name, config_active.vault_index[idx].app_name, VAULT_VAR_NAME_LEN); 
	strncpy(var_name, config_active.vault_index[idx].var_name, VAULT_VAR_NAME_LEN); 

	app_name[VAULT_VAR_NAME_LEN] = 0;
	var_name[VAULT_VAR_NAME_LEN] = 0;

	*size = config_active.vault_index[idx].size;

	return idx;
}


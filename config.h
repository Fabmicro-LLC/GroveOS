/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef ___CONFIG_H___
#define ___CONFIG_H___

#include <stdint.h>
#include "hardware.h"

#define	BAUD_RATES	6
#define	STOP_BITS	4

#pragma pack(1)

extern int baud_rates[];

#define	VAULT_INDEX_SIZE	(16*16)
#define	VAULT_VAR_NAME_LEN	14

typedef struct __VAULT_INDEX {
	char app_name[VAULT_VAR_NAME_LEN];
	char var_name[VAULT_VAR_NAME_LEN];
	int backup_sram_offset;
	int size;
} VAULT_INDEX;

typedef struct __CONFIG {

	struct {
        	float KX1;
        	float KX2;
        	float KX3;
        	float KY1;
        	float KY2;
        	float KY3;
        	uint8_t enabled;
		uint8_t reserve1;
		uint8_t reserve2;
		uint8_t reserve3;
	} lcd;

	struct {
        	uint8_t enabled;
		uint8_t	mode;
		uint8_t reserve1;
		uint8_t reserve2;
        	uint32_t tim_period;
        	uint32_t samples;
        	float coeff[16];
        	float offset[16];
	} adc;

	struct {
		uint8_t enabled;
		uint8_t prescaler;
		uint8_t mode;
		uint8_t flags;
	} spi;

	struct {
		uint8_t mode;
		uint8_t short_addr;
		uint8_t txrx_pol;
		uint8_t manchester_pol;
	} dali1;

	struct {
		uint8_t mode;
		uint8_t short_addr;
		uint8_t txrx_pol;
		uint8_t manchester_pol;
	} dali2;

	uint8_t gpios_enabled;
	uint8_t logger_enabled;
	uint8_t	microsec_timer_enabled;
	uint8_t ext1_port_baud_rate;
	uint8_t ext1_port_parity;
	uint8_t ext1_port_stop_bits;
	uint8_t	ext1_port_modbus_mode;
	uint8_t	ext1_port_modbus_addr;
	uint8_t ext2_port_baud_rate;
	uint8_t ext2_port_parity;
	uint8_t ext2_port_stop_bits;
	uint8_t	ext2_port_modbus_mode;
	uint8_t	ext2_port_modbus_addr;
	uint8_t use_rtc;
	uint8_t reserve1;
	uint8_t reserve2;
	uint32_t ext2_port_modbus_key;
	uint32_t ext1_port_modbus_key;
	
	char default_application_file_name[256];
	
	VAULT_INDEX vault_index[VAULT_INDEX_SIZE];

	char user_data[2048];

	uint16_t crc16;
} CONFIG;

#pragma pack()


CONFIG config_active;

int load_config();
int save_config();
int verify_config();



#define CONF_END        0
#define CONF_BYTE       1
#define CONF_WORD       2
#define CONF_SUBITEM    3
#define CONF_DWORD      4
#define CONF_FLOAT      5

typedef struct _CONF_ITEM {
        unsigned char   item_type;              // 0 - end of list, 1 - byte, 2 - word, 3 - sub_item, 4 - dword, 5 - float, N - array of N bytes
        char            *item_name;             // NULL
        char            *item_descr;            // Some text string
        void*           data;                   // Pointer to data or sub_item if type = 3
} CONF_ITEM;

extern CONF_ITEM config_map[];

int config_map_find(char* conf_str, int* item_type, void** item_data);
int config_map_enum(CONF_ITEM* conf_item, char* prefix);



#endif

/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "svc.h"
#include "msg.h"
#include "modbus_common.h"

#define	MENU_ITEMS		10	// max depth of menu
#define	MONITOR_QUIT_TIMEOUT	15000	// quit in 15 secs of inactivity
#define	STR_BUF_SIZE		128	// Size of bffer for debug print 
#define OLED_LINE_WIDTH		100	// OLED is of 100 pixels width
#define	OLED_CHAR_WIDTH		6	// Each character on OLED display is of 6 pixels width

#define	MAX_DC_PORT_NUM		3	// 0..3
#define	MAX_AC_PORT_NUM		3	// 0..3
#define	MAX_SENSOR_PORT_NUM	3	// 0..3
#define	MAX_INPUT_PORT_NUM	9	// 0..9

#define	SENSOR_L1P1		4	// Sensor #4 is for Line1 Phase1
#define	SENSOR_L1P2		5	// Sensor #5 is for Line1 Phase2
#define	SENSOR_L1P3		6	// Sensor #6 is for Line1 Phase3
#define	SENSOR_L2P1		7	// Sensor #7 is for Line2 Phase1
#define	SENSOR_L2P2		8	// Sensor #8 is for Line2 Phase2
#define	SENSOR_L2P3		9	// Sensor #9 is for Line2 Phase3

#define	MONITOR_QUIT_TIMER	1001
#define	MONITOR_REFRESH_OLED	1002
#define	MONITOR_MODBUS_COMPLETE	1003
#define	MONITOR_MODBUS_TIMEOUT	1004

#define	MENU_CMD_MAIN		2001
#define	MENU_CMD_IN_PORTS	2002
#define	MENU_CMD_SENSOR_PORTS	2003
#define	MENU_CMD_DC_PORTS	2004
#define	MENU_CMD_AC_PORTS	2005
#define	MENU_CMD_FILE		2007
#define	MENU_CMD_SETTINGS	2009
#define	MENU_CMD_DEFAULT_APP	2010
#define	MENU_CMD_EXT1_BAUD	2011
#define	MENU_CMD_EXT1_PARITY	2012
#define	MENU_CMD_EXT1_STOP	2013
#define	MENU_CMD_EXT1_ADDR	2014
#define	MENU_CMD_EXT1_MODE	2015
#define	MENU_CMD_FORMAT		2016
#define	MENU_CMD_FORMAT_CANCEL	2017
#define	MENU_CMD_FORMAT_OK	2018
#define	MENU_CMD_EXEC		2019
#define	MENU_CMD_EXEC_CANCEL	2020
#define	MENU_CMD_EXEC_OK	2021
#define	MENU_CMD_380V_PORTS	2022
#define	MENU_CMD_380V_1		2023
#define	MENU_CMD_380V_2		2024
#define	MENU_CMD_DC_SET		2025
#define	MENU_CMD_AC_SET		2026
#define	MENU_CMD_DEFAULT_APP_CONFIRM	2027
#define	MENU_CMD_DEFAULT_APP_OK		2027
#define	MENU_CMD_EXT1_BAUD_SET		2028
#define	MENU_CMD_EXT1_PARITY_SET	2029
#define	MENU_CMD_EXT1_STOP_SET		2030
#define	MENU_CMD_EXT1_MODE_SET		2031
#define	MENU_CMD_EXT1_ADDR_SET		2032
#define	MENU_CMD_MODBUS_REQ		2033
#define	MENU_CMD_MODBUS_REQ_ADDR	2034
#define	MENU_CMD_MODBUS_REQ_REG		2036
#define	MENU_CMD_MODBUS_REQ_RESPONSE	2037
#define	MENU_CMD_LAST			3000

#define	NO_PROC			(void*)0xffffffff

#define	NUM_OF_MENUS	(sizeof(menus) / sizeof(MENU))

int application_text_addr = 0;

void monitor_show_menu(void);
void monitor_navigate_menu(int cmd, int p1, int p2);
void monitor_show_file(void);
void monitor_navigate_file(int cmd, int p1, int p2);
void monitor_show_exec(void);
void monitor_show_inputs(void);
void monitor_show_sensors(void);
void monitor_navigate_ports(int cmd, int p1, int p2);
void monitor_show_380v_1(void);
void monitor_show_380v_2(void);
void monitor_navigate_380v(int cmd, int p1, int p2);
void monitor_show_dc_ports(void);
void monitor_show_ac_ports(void);
void monitor_show_dc_set(void);
void monitor_show_ac_set(void);
void monitor_navigate_default_app(int cmd, int p1, int p2);
void monitor_show_default_app_ok(void);
void monitor_show_ext1_baud_set(void);
void monitor_show_ext1_parity_set(void);
void monitor_show_ext1_stop_set(void);
void monitor_show_ext1_mode_set(void);
void monitor_show_ext1_addr(void);
void monitor_show_ext1_addr_set(void);
void monitor_show_modbus_req_addr(void);
void monitor_show_modbus_req_reg(void);
void monitor_show_modbus_req_response(void);

char previous_app_name[256];


int baud_rates[] = { 9600, 19200, 38400, 57600, 115200, 250000 };
char *stop_bits[] = { "1", "0.5", "2" , "1.5" };


typedef struct _MENU_ITEM {
	char* menu_name;
	short item_cmd;
} MENU_ITEM; 

// Main menu

typedef struct _MENU {
	short menu_cmd;
	short parent_cmd;
	void* navi_proc;
	void* show_proc;
	char *title;
	MENU_ITEM menu_items[MENU_ITEMS];
} MENU;

MENU menus[] = {
	{MENU_CMD_MAIN, 0, NO_PROC, NO_PROC, "** Монитор **",
		{
		 { "Вх дискр порты", MENU_CMD_IN_PORTS},
		 { "Вх сенс. порты", MENU_CMD_SENSOR_PORTS},
		 { "Вх 380V порты", MENU_CMD_380V_PORTS},
		 { "Вых DC порты", MENU_CMD_DC_PORTS},
		 { "Вых AC порты", MENU_CMD_AC_PORTS},
		 { "Настройки", MENU_CMD_SETTINGS },
		 { "Файлы", MENU_CMD_FILE },
		 { "Запрос Modbus", MENU_CMD_MODBUS_REQ },
		 { "Формат FLASH", MENU_CMD_FORMAT },
		 { 0 }
		},
	},
	{MENU_CMD_SETTINGS, MENU_CMD_MAIN, NO_PROC, NO_PROC, "** Настройки **", 
		{
		 { "Автозапуск", MENU_CMD_DEFAULT_APP },
		 { "Скорость порта", MENU_CMD_EXT1_BAUD },
		 { "Четность порта", MENU_CMD_EXT1_PARITY },
		 { "Стоп бит порта", MENU_CMD_EXT1_STOP},
		 { "Режим порта", MENU_CMD_EXT1_MODE},
		 { "Адрес Modbus", MENU_CMD_EXT1_ADDR},
 		 { 0 }
		},
	},
	{MENU_CMD_FORMAT, MENU_CMD_MAIN, NO_PROC, NO_PROC, "** Формат **", 
		{
		 { "Отменить", MENU_CMD_MAIN },
		 { "Подтвердить", MENU_CMD_FORMAT_OK },
 		 { 0 }
		},
	},
	{MENU_CMD_EXEC, MENU_CMD_FILE, NO_PROC, NO_PROC, "Исполнить ?", 
		{
		 { "Отменить", MENU_CMD_FILE },
		 { "Подтвердить", MENU_CMD_EXEC_OK },
 		 { 0 }
		},
	},
	{MENU_CMD_DEFAULT_APP_CONFIRM, MENU_CMD_DEFAULT_APP, NO_PROC, NO_PROC, "Автозап. файла ?", 
		{
		 { "Отменить", MENU_CMD_FILE },
		 { "Подтвердить", MENU_CMD_DEFAULT_APP_OK },
 		 { 0 }
		},
	},
	{MENU_CMD_DEFAULT_APP_OK, MENU_CMD_DEFAULT_APP, NO_PROC, &monitor_show_default_app_ok, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_EXT1_BAUD, MENU_CMD_SETTINGS, NO_PROC, NO_PROC, "Скорость порта:", 
		{
		 { "9600", MENU_CMD_EXT1_BAUD_SET },
		 { "19200", MENU_CMD_EXT1_BAUD_SET },
		 { "38400", MENU_CMD_EXT1_BAUD_SET },
		 { "57600", MENU_CMD_EXT1_BAUD_SET },
		 { "115200", MENU_CMD_EXT1_BAUD_SET },
		 { "250000", MENU_CMD_EXT1_BAUD_SET },
 		 { 0 }
		},
	},
	{MENU_CMD_EXT1_PARITY, MENU_CMD_SETTINGS, NO_PROC, NO_PROC, "Четность порта:", 
		{
		 { "Нет (N)", MENU_CMD_EXT1_PARITY_SET },
		 { "Четный (E)", MENU_CMD_EXT1_PARITY_SET },
		 { "Нечетный (O)", MENU_CMD_EXT1_PARITY_SET },
 		 { 0 }
		},
	},
	{MENU_CMD_EXT1_STOP, MENU_CMD_SETTINGS, NO_PROC, NO_PROC, "Стоп бит порта:", 
		{
		 { "Один бит (1)", MENU_CMD_EXT1_STOP_SET },
		 { "Половина (0.5)", MENU_CMD_EXT1_STOP_SET },
		 { "Два бита (2)", MENU_CMD_EXT1_STOP_SET },
		 { "Полтора (1.5)", MENU_CMD_EXT1_STOP_SET },
 		 { 0 }
		},
	},
	{MENU_CMD_EXT1_MODE, MENU_CMD_SETTINGS, NO_PROC, NO_PROC, "Режим порта:", 
		{
		 { "Modbus отключен", MENU_CMD_EXT1_MODE_SET },
		 { "Modbus MASTER", MENU_CMD_EXT1_MODE_SET },
		 { "Modbus SLAVE", MENU_CMD_EXT1_MODE_SET },
 		 { 0 }
		},
	},
	{MENU_CMD_380V_PORTS, MENU_CMD_MAIN, NO_PROC, NO_PROC, "Вх 380V порты", 
		{
		 { "Ввод #1", MENU_CMD_380V_1 },
		 { "Ввод #2", MENU_CMD_380V_2 },
 		 { 0 }
		},
	},
	{MENU_CMD_MODBUS_REQ, MENU_CMD_MAIN, NO_PROC, NO_PROC, "Тип запроса Mod.", 
		{
		 { "Запрос ID", MENU_CMD_MODBUS_REQ_ADDR },
		 { "Запрос COIL", MENU_CMD_MODBUS_REQ_ADDR },
		 { "Запрос HOLDING", MENU_CMD_MODBUS_REQ_ADDR },
		 { "Запрос INPUT", MENU_CMD_MODBUS_REQ_ADDR },
 		 { 0 }
		},
	},
	{MENU_CMD_FILE, MENU_CMD_MAIN, &monitor_navigate_file, &monitor_show_file, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_EXEC_OK, MENU_CMD_MAIN, NO_PROC, &monitor_show_exec, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_IN_PORTS, MENU_CMD_MAIN, &monitor_navigate_ports, &monitor_show_inputs, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_SENSOR_PORTS, MENU_CMD_MAIN, &monitor_navigate_ports, &monitor_show_sensors, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_380V_1, MENU_CMD_380V_PORTS, &monitor_navigate_ports, &monitor_show_380v_1, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_380V_2, MENU_CMD_380V_PORTS, &monitor_navigate_ports, &monitor_show_380v_2, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_DC_PORTS, MENU_CMD_MAIN, &monitor_navigate_ports, &monitor_show_dc_ports, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_AC_PORTS, MENU_CMD_MAIN, &monitor_navigate_ports, &monitor_show_ac_ports, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_DC_SET, MENU_CMD_DC_PORTS, &monitor_navigate_ports, &monitor_show_dc_set, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_AC_SET, MENU_CMD_AC_PORTS, &monitor_navigate_ports, &monitor_show_ac_set, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_DEFAULT_APP, MENU_CMD_SETTINGS, &monitor_navigate_file, &monitor_show_file, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_EXT1_BAUD_SET, MENU_CMD_SETTINGS, NO_PROC, &monitor_show_ext1_baud_set, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_EXT1_PARITY_SET, MENU_CMD_SETTINGS, NO_PROC, &monitor_show_ext1_parity_set, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_EXT1_STOP_SET, MENU_CMD_SETTINGS, NO_PROC, &monitor_show_ext1_stop_set, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_EXT1_MODE_SET, MENU_CMD_SETTINGS, NO_PROC, &monitor_show_ext1_mode_set, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_EXT1_ADDR, MENU_CMD_SETTINGS, monitor_navigate_ports, &monitor_show_ext1_addr, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_EXT1_ADDR_SET, MENU_CMD_SETTINGS, NO_PROC, &monitor_show_ext1_addr_set, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_MODBUS_REQ_ADDR, MENU_CMD_MODBUS_REQ, monitor_navigate_ports, &monitor_show_modbus_req_addr, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_MODBUS_REQ_REG, MENU_CMD_MODBUS_REQ, monitor_navigate_ports, &monitor_show_modbus_req_reg, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
	{MENU_CMD_MODBUS_REQ_RESPONSE, MENU_CMD_MODBUS_REQ, NO_PROC, &monitor_show_modbus_req_response, NULL, // this is programmatically defined menu 
		{
 		 { 0 }
		},
	},
};

// Settings menu



int cur_port = 0;
int menu_cur_cmd = 0;
int menu_cur_pos = 0;
int menu_prev_pos = 0;
int num_of_files = 0;
int prog_base_address = 0;
int modbus_req_type = 0;
int modbus_req_addr = 0;
int modbus_req_reg = 0;
char modbus_req_response_str[64];


char str[STR_BUF_SIZE];


// Default navigation procedure
void monitor_navigate_menu(int msg, int p1, int p2)
{
	if(p1 != 0) // we react to PRESS events only
		return;


	// Call custom showing pricedure if such was defined, otherwise call default one
	void (*navi_func)(int cmd, int p1, int p2) = (void(*)(int cmd, int p1, int p2))NULL;
	for(int i = 0; i < NUM_OF_MENUS; i++) {
		if(menus[i].menu_cmd == menu_cur_cmd) {
			if(menus[i].navi_proc != NO_PROC) {
				navi_func = (void(*)(int cmd, int p1, int p2)) (menus[i].navi_proc + prog_base_address);
				navi_func(msg, p1, p2);
				return;
			}
		}
	}



	// Defaut navigation procedure follows below

	switch(msg) {

		case KEY_LEFT_IRQ: {
			for(int i = 0; i < NUM_OF_MENUS; i++) {
				if(menus[i].menu_cmd == menu_cur_cmd) {
					if(menus[i].parent_cmd == 0) {
						svc_debug_print(str, sprintf(str, "Monitor quit by user request\r\n"));
						svc_oled_clear();
						svc_softtimer_stop(MONITOR_QUIT_TIMER);
						svc_stop(); // stop execution of monitor
						svc_exec(previous_app_name); // exec previous application 
						return;
					} else {
						menu_cur_cmd = menus[i].parent_cmd; // step up to parent menu 
						menu_prev_pos = menu_cur_pos; 
						menu_cur_pos = 0; 
						break;
					}
				}
			}
		} break;

		case KEY_RIGHT_IRQ: {
			for(int i = 0; i < NUM_OF_MENUS; i++) {
				if(menus[i].menu_cmd == menu_cur_cmd) {
					if(menus[i].menu_items[menu_cur_pos].item_cmd == 0) {
						return; // no submenu. do nothing
					} else {
						menu_cur_cmd = menus[i].menu_items[menu_cur_pos].item_cmd; // step down to submenu
						menu_prev_pos = menu_cur_pos; 

						if(menu_cur_cmd == MENU_CMD_EXT1_ADDR) {
							svc_get_config(CONFIG_EXT1_MODBUS_ADDR, &menu_cur_pos);
						} else if(menu_cur_cmd == MENU_CMD_EXT1_BAUD) {
							svc_get_config(CONFIG_EXT1_BAUD_RATE, &menu_cur_pos);
							for(int j = 0 ;j < sizeof(baud_rates) / sizeof(*baud_rates); j++)
								if(baud_rates[j] == menu_cur_pos) {
									menu_cur_pos = j;
									break;
								}
						} else if(menu_cur_cmd == MENU_CMD_EXT1_PARITY) {
							svc_get_config(CONFIG_EXT1_PARITY, &menu_cur_pos);
						} else if(menu_cur_cmd == MENU_CMD_EXT1_STOP) {
							svc_get_config(CONFIG_EXT1_STOP_BITS, &menu_cur_pos);
						} else if(menu_cur_cmd == MENU_CMD_EXT1_MODE) {
							svc_get_config(CONFIG_EXT1_MODBUS_MODE, &menu_cur_pos);
						} else if(menu_cur_cmd == MENU_CMD_MODBUS_REQ_ADDR) {
							modbus_req_type = menu_cur_pos;
							menu_cur_pos = 0;
						} else if(menu_cur_cmd == MENU_CMD_MODBUS_REQ_REG) {
							modbus_req_reg = menu_cur_pos;
							menu_cur_pos = 0;
						} else {
							menu_cur_pos = 0; 
						}
						break;
					}
				}
			}
		} break;

		case KEY_UP_IRQ: {
			if(--menu_cur_pos < 0)
				menu_cur_pos++; 
		} break;
	
		case KEY_DOWN_IRQ: {

			if(++menu_cur_pos >= MENU_ITEMS) {
				--menu_cur_pos; 
			} else {
				for(int i = 0; i < NUM_OF_MENUS; i++) {
					if(menus[i].menu_cmd == menu_cur_cmd) {
						if(menus[i].menu_items[menu_cur_pos].menu_name == NULL)
							--menu_cur_pos;
					}
				}
			}
		} break;

		default:
			break;
	}

	svc_post_message(MONITOR_REFRESH_OLED, 1, 0, 0);
}


// Default menu displaying procedure
void monitor_show_menu(void)
{
	// Call custom showing procedure if such was defined
	void (*show_func)(void) = (void(*)(void))NULL;
	for(int i = 0; i < NUM_OF_MENUS; i++) {
		if(menus[i].menu_cmd == menu_cur_cmd) {
			if(menus[i].show_proc != NO_PROC) {
				show_func = (void(*)(void)) (menus[i].show_proc + prog_base_address);
				show_func();
				return;
			}
		}
	}

	int len, pos;

	switch(menu_cur_cmd) {

		case MENU_CMD_FORMAT_OK: {
			svc_oled_clear();
			svc_oled_print(sprintf(str, "Идет формат..."), str, 0, 0);
			svc_stop(); // stop execution of monitor application
			svc_tfs_format();
			return;
		} break;


		default:
			for(int i = 0; i < NUM_OF_MENUS; i++) {
				if(menus[i].menu_cmd == menu_cur_cmd) {
					svc_debug_print(str, snprintf(str, STR_BUF_SIZE, "Showing menu: %d\r\n", menu_cur_cmd));

					len = strlen(menus[i].title);
					pos = (OLED_LINE_WIDTH - (len*OLED_CHAR_WIDTH))/2 ;
					svc_oled_clear();
					svc_oled_print(len, menus[i].title, pos, 0);
					svc_oled_print(sprintf(str, "%d %s          ", menu_cur_pos+1, menus[i].menu_items[menu_cur_pos].menu_name), str, 0, 1);

					return;
				}
			}

			svc_debug_print(str, snprintf(str, STR_BUF_SIZE, "Menu %d not found\r\n", menu_cur_cmd));
			menu_cur_cmd = MENU_CMD_MAIN;
			menu_prev_pos = menu_cur_pos; 
			menu_cur_pos = 0;
			svc_post_message(MONITOR_REFRESH_OLED, 1, 0, 0);
	}


	return;
}


char* exec_file_name = NULL;

// Custom showing function fpr file browsing menu
void monitor_show_file(void)
{
	// show file name, time and size from TFS

	num_of_files = 0;
	TFS_HEADER *tfs = svc_tfs_get_begin();
			

	if(!tfs) {
		svc_oled_clear();
		svc_oled_print(sprintf(str, "Сбой файл. сист."), str, 0, 1);
		return;
	}
	do {
		if(tfs->magic != TFS_MAGIC) {
			if(num_of_files == 0) {
				svc_oled_clear();
				svc_oled_print(sprintf(str, "Нет файлов!"), str, 0, 1);
			}
			break;
		}

		if(!(tfs->flags & TFS_FLAGS_DEL)) 
			continue;

		if(num_of_files == menu_cur_pos) {
			RTC_TimeTypeDef *t = (RTC_TimeTypeDef *) &tfs->time;
			svc_oled_clear();
				svc_oled_print(sprintf(str, "%0.2d %s", menu_cur_pos+1, tfs->name), str, 0, 0);
				svc_oled_print(sprintf(str, "%0.2d:%0.2d:%0.2d %d", t->RTC_Hours, t->RTC_Minutes, t->RTC_Seconds, tfs->size), str, 0, 1);
			exec_file_name = tfs->name;
		}
		num_of_files++;
	} while(tfs = svc_tfs_find_next(tfs));
}


// Custom navigation procedure for file browsing menu
void monitor_navigate_file(int cmd, int p1, int p2)
{

	int i;

	switch(cmd) {

		case KEY_LEFT_IRQ: {
			for(i = 0; i < NUM_OF_MENUS; i++) {
				if(menus[i].menu_cmd == menu_cur_cmd) {
					if(menus[i].parent_cmd == 0) {
						return; // no return to parrent menu was possible
					} else {
						menu_cur_cmd = menus[i].parent_cmd; // step up to parent menu 
						menu_prev_pos = menu_cur_pos; 
						menu_cur_pos = 0; 
						break;
					}
				}
			}
		} break;

		case KEY_RIGHT_IRQ: {
			if(menu_cur_cmd == MENU_CMD_FILE) {
				if(exec_file_name) {
					menu_cur_cmd = MENU_CMD_EXEC;
					menu_prev_pos = menu_cur_pos; 
					menu_cur_pos = 0; 
				}
			} else if(menu_cur_cmd == MENU_CMD_DEFAULT_APP) {
				if(exec_file_name) {
					menu_cur_cmd = MENU_CMD_DEFAULT_APP_CONFIRM;
					menu_prev_pos = menu_cur_pos; 
					menu_cur_pos = 0; 
				}
			}
		} break;

		case KEY_UP_IRQ: {
			if(--menu_cur_pos < 0)
				menu_cur_pos++; 
		} break;
	
		case KEY_DOWN_IRQ: {

			if(++menu_cur_pos >= num_of_files) {
				--menu_cur_pos; 
			}
		} break;

		default:
			break;
	}

	svc_post_message(MONITOR_REFRESH_OLED, 1, 0, 0);
}

// Custom procedure for file execution  
void monitor_show_exec(void)
{
	svc_debug_print(str, sprintf(str, "Executing: %s\r\n", exec_file_name));
	svc_oled_clear();
	svc_oled_print(sprintf(str, "Исполн. файл:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%s                ", exec_file_name), str, 0, 1);
	svc_stop(); // stop execution of monitor
	svc_exec(exec_file_name);
}


// Custom procedure for showing descrete inputs
void monitor_show_inputs(void)
{

	if(menu_cur_pos > MAX_INPUT_PORT_NUM)
		menu_cur_pos = 0;

	if(menu_cur_pos < 0)
		menu_cur_pos = MAX_INPUT_PORT_NUM;

	int val = svc_read_in(menu_cur_pos);

	svc_oled_clear();
	svc_oled_print(sprintf(str, "Вх дискр порты"), str, 0, 0);
	svc_oled_print(sprintf(str, "IN%d: %d (%s)", menu_cur_pos+1, val, val ? "OFF" : "ON"), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}


// Custom procedure for showing sensor inputs
void monitor_show_sensors(void)
{

	if(menu_cur_pos > MAX_SENSOR_PORT_NUM)
		menu_cur_pos = 0;

	if(menu_cur_pos < 0)
		menu_cur_pos = MAX_SENSOR_PORT_NUM;

	float val;
	svc_read_sensor(menu_cur_pos, &val);

	svc_oled_clear();
	svc_oled_print(sprintf(str, "Вх сенс. порты"), str, 0, 0);
	svc_oled_print(sprintf(str, "SNR%d: %01.02fV RMS   ", menu_cur_pos+1, val), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}

// Custom procedure for showing 380V L1 
void monitor_show_380v_1(void)
{

	if(menu_cur_pos > 1)
		menu_cur_pos = 1;

	float l1_p1;
	float l1_p2;
	float l1_p3;

 	svc_read_sensor(SENSOR_L1P1, &l1_p1);
	svc_read_sensor(SENSOR_L1P2, &l1_p2);
	svc_read_sensor(SENSOR_L1P3, &l1_p3);

	svc_oled_clear();
	svc_oled_print(sprintf(str, " L1P1 L1P2 L1P3 "), str, 0, 0);
	svc_oled_print(sprintf(str, " %.3dV %.3dV %.3dV", (int)l1_p1, (int)l1_p2, (int)l1_p3), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}

// Custom procedure for showing 380V L2 
void monitor_show_380v_2(void)
{

	if(menu_cur_pos > 1)
		menu_cur_pos = 1;

	float l2_p1;
	float l2_p2;
	float l2_p3;

	svc_read_sensor(SENSOR_L2P1, &l2_p1);
	svc_read_sensor(SENSOR_L2P2, &l2_p2);
	svc_read_sensor(SENSOR_L2P3, &l2_p3);

	svc_oled_clear();
	svc_oled_print(sprintf(str, " L2P1 L2P2 L2P3 "), str, 0, 0);
	svc_oled_print(sprintf(str, " %.3dV %.3dV %.3dV", (int)l2_p1, (int)l2_p2, (int)l2_p3), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}

// Custom procedure for navigating sensor inputs
void monitor_navigate_ports(int cmd, int p1, int p2)
{
	int i;

	switch(cmd) {

		case KEY_RIGHT_IRQ: {
			if(menu_cur_cmd == MENU_CMD_DC_PORTS) {
				menu_cur_pos = svc_read_dc_pwm(cur_port);
				menu_cur_cmd = MENU_CMD_DC_SET;
			} else if(menu_cur_cmd == MENU_CMD_AC_PORTS) {
				menu_cur_pos = svc_read_ac_pwm(cur_port);
				menu_cur_cmd = MENU_CMD_AC_SET;
			} else if(menu_cur_cmd == MENU_CMD_EXT1_ADDR) {
				menu_cur_cmd = MENU_CMD_EXT1_ADDR_SET;
			} else if(menu_cur_cmd == MENU_CMD_MODBUS_REQ_ADDR) {
				menu_cur_cmd = MENU_CMD_MODBUS_REQ_REG;
			} else if(menu_cur_cmd == MENU_CMD_MODBUS_REQ_REG) {
				menu_cur_cmd = MENU_CMD_MODBUS_REQ_RESPONSE;
			}
		} break;


		case KEY_LEFT_IRQ: {
			for(i = 0; i < NUM_OF_MENUS; i++) {
				if(menus[i].menu_cmd == menu_cur_cmd) {
					if(menus[i].parent_cmd == 0) {
						return; // no return to parrent menu was possible
					} else {
						menu_cur_cmd = menus[i].parent_cmd; // step up to parent menu 
						menu_prev_pos = menu_cur_pos; 
						menu_cur_pos = 0; 
						break;
					}
				}
			}
		} break;

		case KEY_UP_IRQ: {
			menu_cur_pos--; 
		} break;
	
		case KEY_DOWN_IRQ: {
			menu_cur_pos++;
		} break;

		default:
			break;
	}

	svc_post_message(MONITOR_REFRESH_OLED, 1, 0, 0);
}



// Custom procedure for showing DC PWM ports
void monitor_show_dc_ports(void)
{

	if(menu_cur_pos > MAX_DC_PORT_NUM)
		menu_cur_pos = 0;

	if(menu_cur_pos < 0)
		menu_cur_pos = MAX_DC_PORT_NUM;

	cur_port = menu_cur_pos;
	float pwm = svc_read_dc_pwm(menu_cur_pos);
	int pwm_h = (int)pwm;

	svc_oled_clear();
	svc_oled_print(sprintf(str, "Вых DC порты"), str, 0, 0);
	svc_oled_print(sprintf(str, "DC%d: %d%% ШИМ", menu_cur_pos+1, pwm_h), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}

// Custom procedure for showing AC PWM ports
void monitor_show_ac_ports(void)
{

	if(menu_cur_pos > MAX_AC_PORT_NUM)
		menu_cur_pos = 0;

	if(menu_cur_pos < 0)
		menu_cur_pos = MAX_AC_PORT_NUM;

	cur_port = menu_cur_pos;
	float pwm = svc_read_ac_pwm(menu_cur_pos);
	int pwm_h = (int)pwm;

	svc_oled_clear();
	svc_oled_print(sprintf(str, "Вых AC порты"), str, 0, 0);
	svc_oled_print(sprintf(str, "AC%d: %d%% ШИМ", menu_cur_pos+1, pwm_h), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}

void monitor_show_dc_set(void)
{

	if(menu_cur_pos > 100)
		menu_cur_pos = 0;

	if(menu_cur_pos < 0)
		menu_cur_pos = 100;

	svc_set_dc_pwm(cur_port, (float)menu_cur_pos);

	float pwm = svc_read_dc_pwm(cur_port);
	int pwm_h = (int)pwm;

	svc_oled_clear();
	svc_oled_print(sprintf(str, "Уст. вых. ШИМ"), str, 0, 0);
	svc_oled_print(sprintf(str, "DC%d: %d%%    ", cur_port+1, pwm_h), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}

void monitor_show_ac_set(void)
{

	if(menu_cur_pos > 100)
		menu_cur_pos = 0;

	if(menu_cur_pos < 0)
		menu_cur_pos = 100;

	svc_set_ac_pwm(cur_port, (float)menu_cur_pos);

	float pwm = svc_read_ac_pwm(cur_port);
	int pwm_h = (int)pwm;

	svc_oled_clear();
	svc_oled_print(sprintf(str, "Уст. вых. ШИМ"), str, 0, 0);
	svc_oled_print(sprintf(str, "AC%d: %d%%    ", cur_port+1, pwm_h), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}


// Custom procedure for setting up default app  
void monitor_show_default_app_ok(void)
{
	svc_debug_print(str, sprintf(str, "Default application set to: %s\r\n", exec_file_name));
	svc_oled_clear();
	svc_oled_print(sprintf(str, "Файл автозапуска:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%s                ", exec_file_name), str, 0, 1);

	svc_set_config(CONFIG_DEFAULT_APP, exec_file_name);
	svc_softtimer_run(MENU_CMD_SETTINGS, 2000, 0, 0);
}

// Custom procedure for setting up ext1 port baud rate
void monitor_show_ext1_baud_set(void)
{
	if(menu_prev_pos >= sizeof(baud_rates)/sizeof(*baud_rates) || menu_prev_pos < 0) 
		return;

	svc_debug_print(str, sprintf(str, "EXT1 baud rate set to: %d\r\n", baud_rates[menu_prev_pos]));
	svc_oled_clear();
	svc_oled_print(sprintf(str, "Скорость в порту:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%d бод        ", baud_rates[menu_prev_pos]), str, 0, 1);

	svc_set_config(CONFIG_EXT1_BAUD_RATE, &(baud_rates[menu_prev_pos]));
	svc_softtimer_run(MENU_CMD_SETTINGS, 2000, 0, 0);
}

// Custom procedure for setting up ext1 port parity 
void monitor_show_ext1_parity_set(void)
{
	if(menu_prev_pos > 2 || menu_prev_pos < 0) 
		return;

	svc_debug_print(str, sprintf(str, "EXT1 parity set to: %d\r\n", menu_prev_pos));
	svc_oled_clear();
	svc_oled_print(sprintf(str, "Четность в порту:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%s        ", menu_prev_pos ? (menu_prev_pos == 1 ? "Четные (E)" : "Нечетные (O)") : "Нет (N)"), str, 0, 1);

	svc_set_config(CONFIG_EXT1_PARITY, &menu_prev_pos);
	svc_softtimer_run(MENU_CMD_SETTINGS, 2000, 0, 0);
}


// Custom procedure for setting up ext1 port stop bits 
void monitor_show_ext1_stop_set(void)
{
	if(menu_prev_pos > 3 || menu_prev_pos < 0) 
		return;

	svc_debug_print(str, sprintf(str, "EXT1 stop bits set to: %d\r\n", stop_bits[menu_prev_pos]));
	svc_oled_clear();
	svc_oled_print(sprintf(str, "Стоп биты:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%s        ", stop_bits[menu_prev_pos]), str, 0, 1);

	svc_set_config(CONFIG_EXT1_STOP_BITS, &menu_prev_pos);
	svc_softtimer_run(MENU_CMD_SETTINGS, 2000, 0, 0);
}

// Custom procedure for setting up ext1 port operational mode 
void monitor_show_ext1_mode_set(void)
{
	if(menu_prev_pos > 2 || menu_prev_pos < 0) 
		return;

	svc_debug_print(str, sprintf(str, "EXT1 operational mode set to: %d\r\n", menu_prev_pos));
	svc_oled_clear();
	svc_oled_print(sprintf(str, "Режим порта:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%s", menu_prev_pos ? (menu_prev_pos == 1 ? "Modbus MASTER" : "Modbus SLAVE"):"Modbus отключен"), str, 0, 1);

	svc_set_config(CONFIG_EXT1_MODBUS_MODE, &menu_prev_pos);
	svc_softtimer_run(MENU_CMD_SETTINGS, 2000, 0, 0);
}

// Custom procedure for showing EXT1 port modbus addr
void monitor_show_ext1_addr(void)
{

	if(menu_cur_pos > 255)
		menu_cur_pos = 1;
	if(menu_cur_pos < 1)
		menu_cur_pos = 255;

	menu_prev_pos = menu_cur_pos;

	svc_oled_clear();
	svc_oled_print(sprintf(str, "Modbus адрес:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%d", menu_cur_pos), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}

// Custom procedure for setting up ext1 modbus address 
void monitor_show_ext1_addr_set(void)
{
	if(menu_prev_pos > 255 || menu_prev_pos < 1) 
		return;

	svc_debug_print(str, sprintf(str, "EXT1 modbus address set to: %d\r\n", menu_prev_pos));
	svc_oled_clear();
	svc_oled_print(sprintf(str, "Уст Modbus адрес:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%d", menu_prev_pos), str, 0, 1);

	svc_set_config(CONFIG_EXT1_MODBUS_ADDR, &menu_prev_pos);
	svc_softtimer_run(MENU_CMD_SETTINGS, 2000, 0, 0);
}

// Custom procedure for showing modbus request addr
void monitor_show_modbus_req_addr(void)
{

	if(menu_cur_pos > 255)
		menu_cur_pos = 1;
	if(menu_cur_pos < 1)
		menu_cur_pos = 255;

	menu_prev_pos = menu_cur_pos;
	modbus_req_addr = menu_cur_pos;

	svc_oled_clear();
	svc_oled_print(sprintf(str, "Modbus адрес:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%d", menu_cur_pos), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}


// Custom procedure for showing modbus request reg 
void monitor_show_modbus_req_reg(void)
{

	if(menu_cur_pos > 255)
		menu_cur_pos = 1;
	if(menu_cur_pos < 0)
		menu_cur_pos = 255;

	menu_prev_pos = menu_cur_pos;
	modbus_req_reg = menu_cur_pos;

	svc_oled_clear();
	svc_oled_print(sprintf(str, "Modbus регистр:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%d", menu_cur_pos), str, 0, 1);

	svc_softtimer_run(MONITOR_REFRESH_OLED, 500, 0, 0);
}

// Custom procedure for showing modbus response 
void monitor_show_modbus_req_response(void)
{
	svc_oled_clear();
	svc_oled_print(sprintf(str, "Получен ответ:"), str, 0, 0);
	svc_oled_print(sprintf(str, "%s", modbus_req_response_str), str, 0, 1);


	// Resend modbus request
	switch(modbus_req_type) {
		case 0: {
			svc_debug_print(str, sprintf(str, "Modbus request: READ ID, addr: 0x%02X\r\n", modbus_req_addr));
			MODBUS_REQUEST *req = svc_malloc(sizeof(MODBUS_REQUEST));
			MODBUS_READ_ID(req, modbus_req_addr, MONITOR_MODBUS_COMPLETE, MONITOR_MODBUS_TIMEOUT);
			svc_modbus1_enqueue_request(req);
		} break;

		case 1: {
			svc_debug_print(str, sprintf(str, "Modbus request: READ COIL REG, addr: 0x%02X, reg: %d\r\n", modbus_req_addr, modbus_req_reg));
			MODBUS_REQUEST *req = svc_malloc(sizeof(MODBUS_REQUEST));
			MODBUS_READ_COIL_REGS(req, modbus_req_addr, modbus_req_reg, 1, MONITOR_MODBUS_COMPLETE, MONITOR_MODBUS_TIMEOUT);
			svc_modbus1_enqueue_request(req);
		} break;

		case 2: {
			svc_debug_print(str, sprintf(str, "Modbus request: READ HOLDING REG, addr: 0x%02X, reg: %d\r\n", modbus_req_addr, modbus_req_reg));
			MODBUS_REQUEST *req = svc_malloc(sizeof(MODBUS_REQUEST));
			MODBUS_READ_HOLDING_REGS(req, modbus_req_addr, modbus_req_reg, 1, MONITOR_MODBUS_COMPLETE, MONITOR_MODBUS_TIMEOUT);
			svc_modbus1_enqueue_request(req);
		} break;

		case 3: {
			svc_debug_print(str, sprintf(str, "Modbus request: READ INPUT REG, addr: 0x%02X, reg: %d\r\n", modbus_req_addr, modbus_req_reg));
			MODBUS_REQUEST *req = svc_malloc(sizeof(MODBUS_REQUEST));
			MODBUS_READ_INPUT_REGS(req, modbus_req_addr, modbus_req_reg, 1, MONITOR_MODBUS_COMPLETE, MONITOR_MODBUS_TIMEOUT);
			svc_modbus1_enqueue_request(req);
		} break;

		default:
			break;
	}

	modbus_req_response_str[0] = 0;

	svc_softtimer_run(MONITOR_REFRESH_OLED, 2000, 0, 0);
}




// Once installed this procedure will be called for every system event, like timers, IRQ sources, ADC, etc...

void process_queue(MSG *msg)
{

        switch(msg->message) {

                case INFO_TIMER_INT: { // msg->p1 contains millisecond timer counter value
			uint32_t info_timer_ms = msg->p1;
			if(info_timer_ms % 1000 == 0) {
				snprintf(str, 128, "MONITOR menu = %d, pos = %d\r\n", menu_cur_cmd, menu_cur_pos);
				svc_debug_print(str, 0); // every second
			}


		} break;

		case KEY_UP_IRQ: { // msg->p1 contains key GPIO input value which equals to 0 when key is pressed, 1 - otherwise
			monitor_navigate_menu(msg->message, msg->p1, msg->p2);
			svc_softtimer_run(MONITOR_QUIT_TIMER, MONITOR_QUIT_TIMEOUT, 0, 0);
			break;
		}

		case KEY_DOWN_IRQ: { // msg->p1 contains key GPIO input value which equals to 0 when key is pressed, 1 - otherwise
			monitor_navigate_menu(msg->message, msg->p1, msg->p2);
			svc_softtimer_run(MONITOR_QUIT_TIMER, MONITOR_QUIT_TIMEOUT, 0, 0);
			break;
		}

		case KEY_LEFT_IRQ: { // msg->p1 contains key GPIO input value which equals to 0 when key is pressed, 1 - otherwise
			monitor_navigate_menu(msg->message, msg->p1, msg->p2);
			svc_softtimer_run(MONITOR_QUIT_TIMER, MONITOR_QUIT_TIMEOUT, 0, 0);
			break;
		}

		case KEY_RIGHT_IRQ: { // msg->p1 contains key GPIO input value which equals to 0 when key is pressed, 1 - otherwise
			monitor_navigate_menu(msg->message, msg->p1, msg->p2);
			svc_softtimer_run(MONITOR_QUIT_TIMER, MONITOR_QUIT_TIMEOUT, 0, 0);
			break;
		}

		case MONITOR_REFRESH_OLED: {
			monitor_show_menu();
		} break;

		case MONITOR_QUIT_TIMER: {
			svc_debug_print(str, sprintf(str, "Monitor quit by timeout!\r\n"));
			svc_oled_clear();
			svc_stop(); // stop execution of monitor
			svc_exec(previous_app_name); // exec previous application 
		} break;

		case MONITOR_MODBUS_COMPLETE: {
			MODBUS_REQUEST *req = (MODBUS_REQUEST*) msg->p1;
			switch(req->rxbuf[1]) {
				case FUNC_READ_ID:
					snprintf(modbus_req_response_str, MODBUS_RX_BUF_SIZE, "%s", req->rxbuf+3);
					break;
				case FUNC_READ_HOLD_REGS:
				case FUNC_READ_INPUT_REGS:
				case FUNC_READ_COILS:
					snprintf(modbus_req_response_str, MODBUS_RX_BUF_SIZE, "0x%04X", (req->rxbuf[3] << 8) | req->rxbuf[4]);
					break;
				default:
					sprintf(modbus_req_response_str, "** OK **");
					break;
					
			}
			svc_free(req);
		} break;

		case MONITOR_MODBUS_TIMEOUT: {
			sprintf(modbus_req_response_str, "** Нет ответа **");
			svc_free((MODBUS_REQUEST*) msg->p1);
		} break;

		default:
			if((msg->message >= MENU_CMD_MAIN) && (msg->message < MENU_CMD_LAST)) {
				menu_cur_cmd = msg->message;
				menu_prev_pos = menu_cur_pos; 
				menu_cur_pos = 0;
				monitor_show_menu();
			}
			break; // just skip all other system events
	}
}



void* _start(char* prev_app_name)
{
	// Remember previously running application file name, will call it upon return
	if(prev_app_name)
		memcpy(previous_app_name, prev_app_name, strnlen(prev_app_name, 255) + 1);
	else
		previous_app_name[0] = 0x0;


	prog_base_address = svc_get_text(); // we will need this to be able to call our custom functions in menu
	application_text_addr = prog_base_address;

	svc_oled_clear();

	modbus_req_response_str[0] = 0;
	menu_cur_cmd = MENU_CMD_MAIN;
	menu_cur_pos = 0;
	menu_prev_pos = 0;
	monitor_show_menu();

	svc_softtimer_run(MONITOR_QUIT_TIMER, MONITOR_QUIT_TIMEOUT, 0, 0);

	return &process_queue; // return address of our qqueue processing procedure
}




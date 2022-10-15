/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "stm32f4xx_flash.h"
#include "stm32f4xx_it.h"

#include "main.h"
#include "utils.h"
#include "lcd-ft800.h"
#include "msg.h"
#include "hardware.h"
#include "adc.h"
#include "ext_spi.h"

#include "fbqueue.h"
#include "ft_gpu.h"

#include "work.h"
#include "svc.h"
#include "config.h"
#include "logger.h"
#include "ext_gpio.h"
#include "elf.h"
#include "zmodem.h"
#include "crc16.h"
#include "tfs.h"
#include "oled.h"
#include "softtimer.h"
#include "modbus_common.h"
#include "modbus_ext1.h"
#include "modbus_ext2.h"

#include "dali_common.h"
#ifdef DALI1_MODE
#include "dali1.h"
#endif
#ifdef DALI1_MODE
#include "dali2.h"
#endif

#include "vault.h"
#include "audio.h"
#include "g711.h"
//#include "wnd.h"
//#include "svglib.h"
#include "ext_irq.h"
#include "ext_pwm.h"
#include "listener.h"

#define UILIB_IMPL
#include "uilib.h"




#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define swap32(u32)     (((u32) >> 24) | (((u32) & 0x00FF0000) >> 8) | (((u32) & 0x0000FF00) << 8) | ((u32) << 24))

// Writable holding registers
#define	REG_WRITE_SCRATCH	1
#define	REG_ADDRESS		2
#define	REG_BAUD		3
#define	REG_STOP		4
#define	REG_PARITY		5
#define	REG_SET_DEFAULT_APP	6
#define	REG_EXEC_APP		7
#define	REG_TFS_FORMAT		8
#define	REG_TFS_DELETE		9
#define	REG_TFS_CREATE		10
#define	REG_TFS_CLOSE		11
#define	REG_TFS_WRITE_BLOCK	12
#define	REG_TFS_FIND		13
#define	REG_TFS_READ_BLOCK	14
#define	REG_TFS_FREE_SPACE	15
#define	REG_WRITE_DATE		16
#define	REG_WRITE_TIME		17
#define	REG_PLAY_FILE		18	

#define	REG_ADC0_OFFSET			100
#define	REG_ADC0_COEF			101
#define	REG_ADC0_AVG			102
#define	REG_ADC0_RMS			103
#define	REG_ADC1_OFFSET			104
#define	REG_ADC1_COEF			105
#define	REG_ADC1_AVG			106
#define	REG_ADC1_RMS			107
#define	REG_ADC2_OFFSET			108
#define	REG_ADC2_COEF			109
#define	REG_ADC2_AVG			110
#define	REG_ADC2_RMS			111
#define	REG_ADC3_OFFSET			112
#define	REG_ADC3_COEF			113
#define	REG_ADC3_AVG			114
#define	REG_ADC3_RMS			115
#define	REG_ADC4_OFFSET			116
#define	REG_ADC4_COEF			117
#define	REG_ADC4_AVG			118
#define	REG_ADC4_RMS			119
#define	REG_ADC5_OFFSET			120
#define	REG_ADC5_COEF			121
#define	REG_ADC5_AVG			122
#define	REG_ADC5_RMS			123
#define	REG_ADC6_OFFSET			124
#define	REG_ADC6_COEF			125
#define	REG_ADC6_AVG			126
#define	REG_ADC6_RMS			127
#define	REG_ADC7_OFFSET			128
#define	REG_ADC7_COEF			129
#define	REG_ADC7_AVG			130
#define	REG_ADC7_RMS			131
#define	REG_ADC8_OFFSET			132
#define	REG_ADC8_COEF			133
#define	REG_ADC8_AVG			134
#define	REG_ADC8_RMS			135
#define	REG_ADC9_OFFSET			136
#define	REG_ADC9_COEF			137
#define	REG_ADC9_AVG			138
#define	REG_ADC9_RMS			139
#define	REG_ADC10_OFFSET		140
#define	REG_ADC10_COEF			141
#define	REG_ADC10_AVG			142
#define	REG_ADC10_RMS			143
#define	REG_ADC11_OFFSET		144
#define	REG_ADC11_COEF			145
#define	REG_ADC11_AVG			146
#define	REG_ADC11_RMS			147
#define	REG_ADC12_OFFSET		148
#define	REG_ADC12_COEF			149
#define	REG_ADC12_AVG			150
#define	REG_ADC12_RMS			151

// Readable holding registers
#define	REG_READ_SCRATCH	1
#define	REG_READ_DATE		16	
#define	REG_READ_TIME		17	
#define	REG_READ_TEMPR		19	



WND* SCREEN=NULL;
HIT_TEST_INFO TOUCH_INFO = {0};

uint32_t skip_touch_count = 0;
uint32_t skip_touches_time_128us = 0;
uint32_t info_timer_ms;

static void skip_few_touches();

uint8_t event_logging = 0;


static uint32_t logger_timestamp = 0;
static uint32_t lcd_check_timestamp = 0;
static int draw_img_index_counter=0;
static int lcd_update_counter = 0;
static int lcd_error = 0;
static int monitor_invoke_counter = 0;
static uint16_t scratch_register = 0;


TFS_HEADER* application_tfs = NULL;
void* application_elf = NULL;
uint32_t application_elf_size = 0;
int (*application_start_addr)(void*, char *argv[], int argn) = (int(*)(void*, char **, int))NULL;
void (*application_process_msg_addr)(MSG*) = (void(*)(MSG*))NULL;
uint8_t* application_data_addr = NULL;
uint32_t application_text_addr = 0;
uint32_t application_data_size = 0;
uint32_t application_memory[APPLICATION_MAX_MEM] = { 0 };


char cli_buf[1024+4];
int cli_buf_len = 0;

extern CONFIG *config_flash;
extern int baud_rates[];
extern char *stop_bits[];

extern unsigned char _binary_monitor_elf_start;
extern unsigned char _binary_monitor_elf_end;
extern unsigned char _binary_monitor_elf_size;

TFS_HEADER* zmodem_load_file(int attempts);

#ifdef USART_EXT1
MODBUS_RESPONSE modbus_hold_regs_read_ext1, modbus_hold_regs_write_ext1;
#endif

#ifdef USART_EXT2 
MODBUS_RESPONSE modbus_hold_regs_read_ext2, modbus_hold_regs_write_ext2;
#endif


#ifdef DALI1_MODE
DALI_REQUEST dali1_req1, dali1_req2;
#endif

WND* create_screen() {
	WND* hwnd = wnd_create(WINDOW);
        wnd_set_frame(hwnd, 0, 0, LCD_WIDTH, LCD_HEIGHT);
        hwnd->bgcolor=COLOR332(0,127,0); //GREEN DARK


	static char os_title[32];
	sprintf(os_title, "GroveOS ver %04d - %s", BUILD_NUMBER, OS_PRODUCT_NAME); 

	WND* label = wnd_create(LABEL);
	wnd_set_text(label, os_title);
	wnd_set_font(label, font_large);
	wnd_set_text_color(label, WHITE);
	wnd_set_text_alignment(label, center);
	Size sz=wnd_size_that_fits(label);
	wnd_set_frame(label, (LCD_WIDTH - sz.cx)/2, (LCD_HEIGHT - sz.cy)/2, sz.cx, sz.cy);
	wnd_add_subview(hwnd, label);

	return hwnd;
}


void draw_fb() {
        Fbinfo *fb=NULL;
	while(fb=fbqueue_get_tail()) {
		fb->img_index = draw_img_index_counter % 4;
		Context gc={0};
		Rect rect;
		switch(fb->img_index) {
		case 0: SetRect(&rect, 0, 0, 0+FRAMEBUFFER_WIDTH , 0+FRAMEBUFFER_HEIGHT); break;
		case 1: SetRect(&rect, FRAMEBUFFER_WIDTH, 0, FRAMEBUFFER_WIDTH+FRAMEBUFFER_WIDTH , 0+FRAMEBUFFER_HEIGHT); break;
		case 2: SetRect(&rect, 0, FRAMEBUFFER_HEIGHT, 0+FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT+FRAMEBUFFER_HEIGHT); break;
		case 3: SetRect(&rect, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, FRAMEBUFFER_WIDTH+FRAMEBUFFER_WIDTH , FRAMEBUFFER_HEIGHT+FRAMEBUFFER_HEIGHT); break;
		}

		gc_init(&gc, fb->framebuffer, &rect, font_small);

		//print("draw_fb::1\r\n");
        	wnd_proc_call(SCREEN, WM_DO_DRAW, (int) &gc, 0);
		//print("draw_fb::2\r\n");
		fbqueue_enqueue(fb);

		draw_img_index_counter++;
	}


}

void stop_application(void* app_text_addr)
{

	//print("stop_application\r\n");

	// remove all windows
	wnd_destroy(SCREEN);
	SCREEN = create_screen();

	// clear all ext irq handlers
	if(app_text_addr)
		ext_irq_clear_all(app_text_addr);

	listener_clear_all();

	application_process_msg_addr = (void (*)(MSG*))NULL;
	//application_tfs = (TFS_HEADER*) NULL;
	//application_elf = NULL;

	softtimer_clear_range(1000, 65535);

	#ifdef USART_EXT1
	modbus_unregister_range_ext1(1000, 65535);
	#endif

	// free memory used by the app
	for(int i = 0; i < APPLICATION_MAX_MEM; i++) {
		if(application_memory[i]) {
			free((void*)application_memory[i]);
			application_memory[i] = (uint32_t)NULL;
		}
	}

}

int exec(char* prev_app_name, char *argv[], int argn)
{
	int rc;

	stop_application((void*)application_text_addr);

	#ifdef USART_EXT1
	if(config_active.ext1_port_modbus_mode == MODBUS_MASTER)
		modbus_dequeue_range_ext1(1000, 65535);

	if(config_active.ext1_port_modbus_mode == MODBUS_SLAVE)
		modbus_unregister_range_ext1(1000, 65535);
	#endif

	#ifdef USART_EXT2
	if(config_active.ext2_port_modbus_mode == MODBUS_MASTER)
		modbus_dequeue_range_ext2(1000, 65535);

	if(config_active.ext2_port_modbus_mode == MODBUS_SLAVE)
		modbus_unregister_range_ext2(1000, 65535);
	#endif

	#ifdef DALI1_MODE
	if(config_active.dali1.mode == DALI_MASTER)
		dali1_dequeue_range(1000, 65535);

	if(config_active.dali1.mode == DALI_SLAVE)
		dali1_unregister_range(1000, 65535);
	#endif

	#ifdef DALI2_MODE
	if(config_active.dali2.mode == DALI_MASTER)
		dali2_dequeue_range(1000, 65535);

	if(config_active.dali2.mode == DALI_SLAVE)
		dali2_unregister_range(1000, 65535);
	#endif




	if((rc = elf_check_file(application_elf, application_elf_size, (void **)&application_start_addr, &application_text_addr, &application_data_addr, &application_data_size)) != ELF_ERROR_OK) {
		print("exec() ELF application cannot be run!\r\n");
		return -1;
	} else {
		print("exec() ELF application is OK at %p, text: %p, data: %d@%p, entry: %p. Calling application _start fuction...\r\n", application_elf, application_text_addr, application_data_size, application_data_addr, application_start_addr);


		//Clear RAM
		memset((void*)APPLICATION_DATA_RAM_ADDR, 0, APPLICATION_DATA_RAM_SIZE);

		// Copy data segment to RAM

		if(application_data_addr && application_data_size > 0)
		memcpy((void*)APPLICATION_DATA_RAM_ADDR, application_data_addr, application_data_size);

		DelayLoop(100);  // wait for 100ms to get UART buffers transmitted

		CPU_STATE old_state = saved_cpu_state;

		if(SaveCPUState(&saved_cpu_state)==0) {
			application_process_msg_addr = (void (*)(MSG*)) (application_text_addr + application_start_addr(prev_app_name, argv, argn)); // RUN
			// No exception

			print("exec() Application returned: %p, using it as msg processing function\r\n", application_process_msg_addr);

			if((unsigned int)application_process_msg_addr < (unsigned int)application_elf || (unsigned int)application_process_msg_addr >= (unsigned int)application_elf + application_elf_size) {
				print("exec() Returned number is outside of application text segment, MSG queue will not be called!\r\n");
				application_process_msg_addr = NULL;
			}
		} else {
			// Exception occured
			print("exec() Application returned with exception: %d\r\n", exception_code);
			application_process_msg_addr = NULL;
		}

		saved_cpu_state = old_state;

		return 0;
	}
}


int exec_tfs(TFS_HEADER *tfs, char *argv[], int argn)
{
	char* prev_app_name;

	if(application_tfs)
		prev_app_name = application_tfs->name;
	else
		prev_app_name = NULL;

	application_tfs = tfs;
	application_elf = (void*)((int)tfs + sizeof(TFS_HEADER));
	application_elf_size = tfs->size;

	return exec(prev_app_name, argv, argn);
}


int exec_elf(char *elf, int size)
{
	char* prev_app_name;

	if(application_tfs)
		prev_app_name = application_tfs->name;
	else
		prev_app_name = NULL;

	application_tfs = NULL;
	application_elf = elf;
	application_elf_size = size;

	return exec(prev_app_name, NULL, 0);
}


void init() {

	//__disable_irq();
	//main_queue_init();
	//softtimer_init();
	//__enable_irq();

	wnd_init();
	SCREEN = create_screen();

	svg_init();
	
	#ifdef LCD_SPI
	if(config_active.lcd.enabled) {
		draw_fb();
		Fbinfo* fb=fbqueue_get_head();
        	if(fb) 
			lcd_write_memory_dma_irq(RAM_G+FRAMEBUFFER_SIZE_BYTES*fb->img_index, (uint8_t*)fb->framebuffer, FRAMEBUFFER_SIZE_BYTES);
	}
	#endif


	//wait_for_autoexec = 1;

	/*
	int rc;
	int autoexec = 1;
	char buf[16];
	TFS_HEADER* tfs = NULL;
	print("init() Press XXX to stop autoexec.\r\n");
	_purge(USART_DEBUG);
	DelayLoop(2000);
	memset(buf, 0, 16);
	_read(USART_DEBUG, buf, 16, 1);
	buf[15] = 0;
	if(strstr(buf, "XXX")) {
		autoexec = 0;
		print("init() Autoexec will be aborted.\r\n");
	}

	if(autoexec)
		tfs = zmodem_load_file(3);

	OLED_WEG010032_PRINT(16, "     Ready      ", 0, 1);

	// Attempt to download application binary file

	if(autoexec) {
		if(tfs) {
			print("init() Running downloaded application...\r\n");
			exec_tfs(tfs);
		} else {
			tfs = tfs_find(config_active.default_application_file_name);

			if(tfs) {
				print("init() Running default application from file: %s\r\n", tfs->name);
				exec_tfs(tfs);
			} else {
				print("init() Application file %s not found!\r\n", config_active.default_application_file_name);
			}
		}
	}

	*/

	#ifdef USART_EXT1 
	if(config_active.ext1_port_modbus_mode == MODBUS_SLAVE) { // SLAVE mode
		MODBUS_CREATE_RESPONDER(&modbus_hold_regs_read_ext1, MODBUS1_READ_HOLD_REGS, FUNC_READ_HOLD_REGS, 1, 999);
		modbus_register_responder_ext1(&modbus_hold_regs_read_ext1);

		MODBUS_CREATE_RESPONDER(&modbus_hold_regs_write_ext1, MODBUS1_WRITE_HOLD_REGS, FUNC_WRITE_MANY_HOLD_REGS, 1, 999);
		modbus_register_responder_ext1(&modbus_hold_regs_write_ext1);

		print("init() Modbus responders registered for EXT1\r\n");
	}
	#endif

	#ifdef USART_EXT2
	if(config_active.ext2_port_modbus_mode == MODBUS_SLAVE) { // SLAVE mode
		MODBUS_CREATE_RESPONDER(&modbus_hold_regs_read_ext2, MODBUS2_READ_HOLD_REGS, FUNC_READ_HOLD_REGS, 1, 999);
		modbus_register_responder_ext2(&modbus_hold_regs_read_ext2);

		MODBUS_CREATE_RESPONDER(&modbus_hold_regs_write_ext2, MODBUS2_WRITE_HOLD_REGS, FUNC_WRITE_MANY_HOLD_REGS, 1, 999);
		modbus_register_responder_ext2(&modbus_hold_regs_write_ext2);

		print("init() Modbus responders registered for EXT2\r\n");
	}
	#endif


	#ifdef OLED_DATA_GPIO_PORT
        OLED_WEG010032_PRINT(16, "     Ready      ", 0, 1);
	#endif

	print("init() Press XXX to stop autoexec.\r\n");
        _purge(USART_DEBUG);
	softtimer_run_timer(CHECK_FOR_AUTOEXEC, 2000, 0, 0);


	print("init() done.\r\n");
}

void process_cmd_print_file(TFS_HEADER* tfs)
{
	if(!tfs)
		return;
	
	RTC_TimeTypeDef *t = (RTC_TimeTypeDef *) &tfs->time;
	RTC_DateTypeDef *d = (RTC_DateTypeDef *) &tfs->date;
	print("%p\t%p\t%s\t%d\t%0.4d-%0.2d-%0.2d\t%0.2d:%0.2d:%0.2d\t%s\r\n",
			tfs,
			(unsigned int)tfs + sizeof(TFS_HEADER),
			(tfs->flags & TFS_FLAGS_DEL) ? "OK":"DEL",
			tfs->size,
			d->RTC_Year+2000, d->RTC_Month, d->RTC_Date, t->RTC_Hours, t->RTC_Minutes, t->RTC_Seconds,
			tfs->name
	);
}

void prompt(void)
{
	_print("%c[2K\r%s ver %04d> ", 0x1B, _DEVICE_NAME_, BUILD_NUMBER);
}



int exec_cli(char* cli_str, int line_len);

void process_cmd(void)
{
	int read_bytes = _read(USART_DEBUG, cli_buf+cli_buf_len, 1024-cli_buf_len, 1);

	cli_buf_len += read_bytes;

	cli_buf[cli_buf_len] = 0x0;
	
	// process ^H
	for(int i = 0; i < cli_buf_len; i++)  {
		if(cli_buf[i] == 0x08 || cli_buf[i] == 0x7F) {
			if(i > 0) {
				memmove(cli_buf+i-1, cli_buf+i+1, cli_buf_len-i-1+1);
				cli_buf_len-=2;
			} else {
				memmove(cli_buf+i, cli_buf+i+1, cli_buf_len-i-1+1);
				cli_buf_len-=1;
			}
			i--;
		}
	}

	prompt();
	//for(int i = 0; i < cli_buf_len; i++) 
	//	_print("%c (%02X) ", cli_buf[i], cli_buf[i]);
	_print("%s", cli_buf);

	// Search for EOL

	char *line_end;

	if(cli_buf_len >= 1024) {
		line_end = cli_buf + 1024;
	} else {
		line_end = my_index(cli_buf, '\n', cli_buf_len);
		if(line_end == NULL) {
			line_end = my_index(cli_buf, '\r', cli_buf_len);
			if(line_end == NULL) {
				return; // CLI not yet ready
			}
		}	
	}

	//_print("\r\n\r\nXXXXXXXXXXX: ");
	//for(int i = 0; i < cli_buf_len; i++) 
	//	_print("%c (%02X) ", cli_buf[i], cli_buf[i]);

	_print("\n\r");
	// From here parsig CLI

	int line_len = line_end - cli_buf + 1;
	char cli_str[line_len];

	memcpy(cli_str, cli_buf, line_len);
	cli_str[line_len-1] = 0;

	memmove(cli_buf, cli_buf+line_len, cli_buf_len-line_len);
	cli_buf_len -= line_len;

	//print("CLI: line_len = %d, buf= %p, line_end = %p, nmea_str = %s\r\n", line_len, buf, line_end, nmea_str);

	exec_cli(cli_str, line_len);
}

void exec_shell(TFS_HEADER* tfs)
{
	print("=== EXECUTING SHELL SCRIPT: %s\r\n", tfs->name);

	char cli_str[256];
	int buf_len = tfs->size;
	char *line_begin = (char*)tfs + sizeof(TFS_HEADER);

	
	while(buf_len) {
		char *line_end = my_index(line_begin, '\n', buf_len);

		int line_len = line_end - line_begin;
		if(line_len < 1)
			continue;

		strncpy(cli_str, line_begin, line_len+1);
		cli_str[line_len] = 0;

		char *r = my_index(cli_str, '\r', line_len);
		if(r)
			*r = 0;

		int rc = exec_cli(cli_str, line_len);

		if(rc != 0) {
			print("=== BATCH EXEC BREAK AFTER '%s' (%d)\r\n", cli_str, rc);
			break;
		}

		line_begin += line_len + 1;
		buf_len -= line_len + 1;
	}
}

int exec_cli(char* cli_str, int line_len)
{

	char *c = cli_str;
	int argn = 1;
	char *arg[10] = {0};

	arg[0] = c;
	while( (c = strpbrkn(c, " ", cli_str + line_len)) ) { 
		*c = 0; c++;
		arg[argn++] = c;
		if(argn==10) 
			break;
	}

	char *cmd = strnarg(cli_str, 0, line_len);

	if(cmd == NULL || cmd[0] == 0x0 || cmd[0] == 0x0a || cmd[0] == 0x0d || cmd[0] == '#') {
		goto end;
	}


	if(strncasecmp(cmd, "SET", 3) == 0) {
		int item_type;
		void* item_data;
		int rc = config_map_find(arg[1], &item_type, &item_data);

		if(rc) {
			print("*** CONF VAR NOT FOUND: %s\r\n", arg[1]);
		} else {
			if(item_type > 5) {
				strncpy((char*)item_data, arg[2], 256);
				print("=== CONF VAR %s TYPE %s %s TO %s\r\n", arg[1], "ARRAY", "SET", (char*)item_data);
			} else {
				switch(item_type) {
					case 0:
					case 3: {
						print("*** CONF VAR %s TYPE %d CANNOT BE USED\r\n", arg[1], item_type);
					} break;
					case 1: {
						*(uint8_t*)item_data = strtol(arg[2], (char **)NULL, 0);
						print("=== CONF VAR %s TYPE %s %s TO %d (0x%08X)\r\n", arg[1], "BYTE", "SET", *(uint8_t*)item_data, *(uint8_t*)item_data);
					} break;
					case 2: {
						*(uint16_t*)item_data = strtol(arg[2], (char **)NULL, 0);
						print("=== CONF VAR %s TYPE %s %s TO %d (0x%08X)\r\n", arg[1], "WORD", "SET", *(uint16_t*)item_data, *(uint16_t*)item_data);
					} break;
					case 4: {
						*(uint32_t*)item_data = strtol(arg[2], (char **)NULL, 0);
						print("=== CONF VAR %s TYPE %s %s TO %d (0x%08X)\r\n", arg[1], "DWORD", "SET", *(uint32_t*)item_data, *(uint32_t*)item_data);
					} break;
					case 5: {
						*(float*)item_data = strtof(arg[2], (char **)NULL);
						print("=== CONF VAR %s TYPE %s %s TO %f (0x%08X)\r\n", arg[1], "FLOAT", "SET", *(float*)item_data, *(uint32_t*)item_data);
					} break;
					default:
						break;
				}
			}
		}

	} else if(strncasecmp(cmd, "GET", 3) == 0) {
		int item_type;
		void* item_data;
		int rc = config_map_find(arg[1], &item_type, &item_data);

		if(rc) {
			print("*** CONF VAR NOT FOUND: %s\r\n", arg[1]);
		} else {
			if(item_type > 5) {
					print("=== CONF VAR %s TYPE %s %s TO %s\r\n", arg[1], "ARRAY", "EQUALS", (char*)item_data);
					int len = strnlen((char*)item_data, 256);
					for(int i = 0; i < len; i++)
						_print("%02X ", ((char*)item_data)[i]);
					_print("\r\n");
			} else {
				switch(item_type) {
					case 0: 
					case 3: {
					print("*** CONF VAR %s TYPE %d CANNOT BE USED\r\n", arg[1], item_type);
					} break;
					case 1: {
						print("=== CONF VAR %s TYPE %s %s TO %d (0x%08X)\r\n", arg[1], "BYTE", "EQUALS", *(uint8_t*)item_data, *(uint8_t*)item_data);
					} break;
					case 2: {
						print("=== CONF VAR %s TYPE %s %s TO %d (0x%08X)\r\n", arg[1], "WORD", "EQUALS", *(uint16_t*)item_data, *(uint16_t*)item_data);
					} break;
					case 4: {
						print("=== CONF VAR %s TYPE %s %s TO %d (0x%08X)\r\n", arg[1], "DWORD", "EQUALS", *(uint32_t*)item_data, *(uint32_t*)item_data);
					} break;
					case 5: {
						print("=== CONF VAR %s TYPE %s %s TO %f (0x%08X)\r\n", arg[1], "FLOAT", "EQUALS", *(float*)item_data, *(uint32_t*)item_data);
					} break;
					default:
						break;
				}
			}
		}

	} else if(strncasecmp(cmd, "EXPORT", 6) == 0) {
		if(argn > 2) { // Export config to TFS file
			print("=== EXPORTING TO FILE NOT SUPPORTED\r\n");
		} else { // Otherwise, display on console
			char prefix[256];
			prefix[0] = 0;

			print("=== BEGIN OF CONFIG\r\n");
			config_map_enum(config_map, prefix);
			print("=== END OF CONFIG\r\n");
			
		}
	} else if(strncasecmp(cmd, "SAVE", 4) == 0) {
		save_config();
		load_config();

		print("=== ACTIVE CONF HAVE BEEN SAVED TO FLASH MEM, CRC16: 0x%04X\r\n", config_active.crc16);

	} else if(strncasecmp(cmd, "TIME", 4) == 0) {
		if(strncasecmp(arg[1], "SET", 3) == 0 && arg[2] && arg[3] && arg[4]) {
        		RTC_TimeTypeDef t;
                        t.RTC_Hours = atoi(arg[2]);
			t.RTC_Minutes = atoi(arg[3]);
			t.RTC_Seconds = atoi(arg[4]);
			PWR_BackupAccessCmd(ENABLE);
			RTC_WriteProtectionCmd(DISABLE);
			RTC_SetTime(RTC_Format_BIN, &t);
			RTC_WriteProtectionCmd(ENABLE);
			PWR_BackupAccessCmd(DISABLE);
			print("=== NEW TIME: %0.2d:%0.2d:%0.2d\r\n", t.RTC_Hours, t.RTC_Minutes, t.RTC_Seconds); 
		} else if(strncasecmp(arg[1], "GET", 3) == 0) {
        		RTC_TimeTypeDef t;
			RTC_GetTime(RTC_Format_BIN, &t);
			print("=== CUR TIME: %0.2d:%0.2d:%0.2d\r\n", t.RTC_Hours, t.RTC_Minutes, t.RTC_Seconds); 
		} else {
			goto usage;
		}

	} else if(strncasecmp(cmd, "DATE", 4) == 0) {
		if(strncasecmp(arg[1], "SET", 3) == 0 && arg[2] && arg[3] && arg[4]) {
        		RTC_DateTypeDef d;
			d.RTC_Year = atoi(arg[2]) - 2000;
			d.RTC_Month = atoi(arg[3]);
			d.RTC_Date = atoi(arg[4]);
             		d.RTC_WeekDay = RTC_Weekday_Tuesday;
			PWR_BackupAccessCmd(ENABLE);
			RTC_WriteProtectionCmd(DISABLE);
			RTC_SetDate(RTC_Format_BIN, &d);
			RTC_WriteProtectionCmd(ENABLE);
			PWR_BackupAccessCmd(DISABLE);
			print("=== NEW DATE: %0.4d-%0.2d-%0.2d\r\n", d.RTC_Year+2000, d.RTC_Month, d.RTC_Date); 
		} else if(strncasecmp(arg[1], "GET", 3) == 0) {
        		RTC_DateTypeDef d;
			RTC_GetDate(RTC_Format_BIN, &d);
			print("=== CUR DATE: %0.4d-%0.2d-%0.2d\r\n", d.RTC_Year+2000, d.RTC_Month, d.RTC_Date ); 
		} else {
			goto usage;
		}

	} else if(strncasecmp(cmd, "VAR", 3) == 0) {

		if(strncasecmp(arg[1], "ERASE", 5) == 0) {
			
			vault_erase();

			print("=== VAULT ERASED\r\n");

		} else if(strncasecmp(arg[1], "LIST", 4) == 0) {
			char app_name[16];
			char var_name[16];
			int size;
			int count = 0;
			print("=== VAULT VARIABLES:\r\n");
			for(int i = 0; i < VAULT_INDEX_SIZE; i++) {
				if(vault_enum(i, app_name, var_name, &size) >= 0) {


					if(size <= 0) 
						continue;

					char *data;
					int rc;

					if((rc = vault_get_var(app_name, var_name, (void**)&data)) < 0) {
						print("### FAILED TO GET VAR %s:%s, rc = %d\r\n", app_name, var_name, rc);
					}

					print("#%03d\t%d\t%s:%s\t:=\t%d\t%f\t0x%08x\t", i, size, app_name, var_name, *(int*)data, *(float*)data, *(int*)data);
					for(int j = 0; j < MIN(size, 32); j++) 
						_print("0x%02X '%c'  ", data[j], data[j]);
					_print("\r\n");
					count++;
				}
			}
			print("Total variables: %d\r\n", count);

		} else if(strncasecmp(arg[1], "DEL", 3) == 0 && arg[2] && arg[3]) {
		
			int rc;

			if((rc = vault_del_var(arg[2], arg[3])) < 0) {
				print("=== DEL VAR ERROR: rc = %d, name = (%s:%s)\r\n", rc, arg[2], arg[3] );
			} else {
				print("=== VAR DELETED: (%s:%s)\r\n", arg[2], arg[3] );
			}

		} else if(strncasecmp(arg[1], "DEL", 3) == 0 && arg[2] && arg[3] == NULL) {
		
			int rc;

			if((rc = vault_del_var_by_app(arg[2])) < 0) {
				print("=== DEL VAR ERROR: rc = %d, name = (%s:%s)\r\n", rc, arg[2], arg[3] );
			} else {
				print("=== ALL VARS FOR APP %s DELETED\r\n", arg[2] );
			}


		} else if(strncasecmp(arg[1], "GET", 3) == 0 && arg[2] && arg[3]) {
			char* data; 
			int size = vault_get_var(arg[2], arg[3], (void**)&data);
			if(size < 0) {
				print("=== GET VAR ERROR: rc = %d, name = (%s:%s)\r\n", size, arg[2], arg[3] ); 
			} else {
				print("=== VAR: (%s:%s) := 0x%08X (%d)\t", arg[2], arg[3], *(int*)data,  *(int*)data); 
				for(int j = 0; j < size; j++) 
					_print("0x%02X '%c'  ", data[j], data[j]);
				_print("\r\n");
			}
			free(data);

		} else if(strncasecmp(arg[1], "SETI", 4) == 0 && arg[2] && arg[3] && arg[4]) {
			char tmp[4];
			int val = strtol(arg[4], (char **)NULL, 0);
			int rc = vault_set_var(arg[2], arg[3], (void*)&val, 4, 0);
			if(rc < 0) {
				print("=== SET VAR ERROR: rc = %d, name = (%s:%s)\r\n", rc, arg[2], arg[3] ); 
			} else {
				uint32_t *v;
				rc = vault_get_var(arg[2], arg[3], (void**)&v);
				if(rc < 0) {
					print("=== GET WHILE SET VAR ERROR: rc = %d, name = (%s:%s)\r\n", rc, arg[2], arg[3] ); 
				} else {
					print("=== VAR: (%s:%s) := 0x%08X (%d)\t", arg[2], arg[3], *v, *v); 
					_print("\r\n");
				}
			}
		} else if(strncasecmp(arg[1], "SETF", 4) == 0 && arg[2] && arg[3] && arg[4]) {
			char tmp[4];
			float val = strtof(arg[4], (char **)NULL);
			int rc = vault_set_var(arg[2], arg[3], (void*)&val, 4, 0);
			if(rc < 0) {
				print("=== SET VAR ERROR: rc = %d, name = (%s:%s)\r\n", rc, arg[2], arg[3] ); 
			} else {
				float *v;
				rc = vault_get_var(arg[2], arg[3], (void**)&v);
				if(rc < 0) {
					print("=== GET WHILE SET VAR ERROR: rc = %d, name = (%s:%s)\r\n", rc, arg[2], arg[3] ); 
				} else {
					print("=== FLOAT VAR: (%s:%s) := %f\t", arg[2], arg[3], *v); 
					_print("\r\n");
				}
			}
		} else {
			goto usage;
		}

	} else if(strncasecmp(cmd, "ECHO", 4) == 0) {
	
		for(int i = 1; i < argn; i++)
			_print("%s ", arg[i]);
		_print("\r\n");

	} else if(strncasecmp(cmd, "WAIT", 4) == 0) {
	
		for(int i = 1; i < argn; i++)
			_print("%s ", arg[i]);
		_print("\r\n");


		while(cli_buf_len == 0) {
			Delay(500);
			int cli_buf_len = _read(USART_DEBUG, cli_buf, 1024, 1);
			if(cli_buf_len > 0) {
				if(cli_buf[0] == '\r')
					return 0;
				else
					return -1;
			}
		}

	} else if(strncasecmp(cmd, "RUN", 3) == 0) {

		if(arg[1]) {
			TFS_HEADER* tfs = tfs_find(arg[1]);

			if(tfs) {
				exec_shell(tfs);
			} else {
				print("*** NOT FOUND: %s\r\n", arg[1]);
			}
		}

	} else if(strncasecmp(cmd, "APLAY", 5) == 0 && arg[1] && arg[2]) {

		uint32_t len = strtol(arg[2], (char **)NULL, 0);
		uint32_t addr = strtol(arg[1], (char **)NULL, 0);

		play_start((char*)addr, len);

		print("=== PLAYING AUDIO BLOCK: buf = %p, len = %d\r\n", addr, len);

	} else if(strncasecmp(cmd, "TFS", 3) == 0) {
		if(strncasecmp(arg[1], "LIST", 4) == 0) {

			int count = 0;

			TFS_HEADER* tfs;

			if(arg[2]) 
				tfs = (TFS_HEADER*) strtol(arg[2], NULL, 0);
			else
				tfs = tfs_get_begin(); 

			print("=== LIST OF FILES AT %p\r\n", tfs);

			print("TFS ptr:\tDATA ptr:\tFlags:\tSize:\tDate:\t\tTime:\t\tName:\r\n");

			do {
				if(tfs->magic != TFS_MAGIC)
					break;
				process_cmd_print_file(tfs);
				count++;
			} while(tfs = tfs_find_next(tfs));

			print("=== FILES: %d, FREE SPACE: %d BYTES\r\n", count, tfs_get_free_space());

		} else if(strncasecmp(arg[1], "RECEIVE", 7) == 0) {

			print("=== RECEIVING FILE USING ZMODEM\r\n");

			TFS_HEADER* tfs = zmodem_load_file(15);

			if(tfs) {
				print("=== RECEIVED FILE: %s\r\n", tfs->name);
			} else {
				print("=== FAILED TO RECEIVE FILE\r\n", tfs);
			}

		} else if(strncasecmp(arg[1], "INFO", 4) == 0 && arg[2] != NULL) {

			TFS_HEADER* tfs = tfs_find(arg[2]);

			if(tfs == NULL) {
				print("=== FILE %s not FOUND!\r\n", arg[2]);
			} else {
				print("=== FILE %s INFO:\r\n");
				process_cmd_print_file(tfs);
			}

		} else if(strncasecmp(arg[1], "DEL", 3) == 0 && arg[2] != NULL) {

			TFS_HEADER* tfs = tfs_find(arg[2]);

			if(tfs == NULL) {
				print("=== FILE %s not FOUND!\r\n", arg[2]);
			} else {
				print("=== FILE %s DELETED!\r\n", arg[2]);

				tfs_delete(tfs);
			}

		} else if(strncasecmp(arg[1], "EXEC", 4) == 0 && arg[2] != NULL) {

			TFS_HEADER* tfs = tfs_find(arg[2]);
			int rc;

			if(tfs == NULL) {
				print("=== FILE %s NOT FOUND!\r\n", arg[2]);
			} else {
				
				print("=== EXECUTING APPLICATION FROM FILE: %s, TFS: %p, ELF ADDR: %p\r\n", arg[2], tfs, (int)tfs + sizeof(TFS_HEADER)) ;

				char *argv[3];
				int argn = 0;
				
				if(arg[2]) {argv[0] = arg[2]; argn++; }
				if(arg[3]) {argv[1] = arg[3]; argn++; }
				if(arg[4]) {argv[2] = arg[4]; argn++; }

				exec_tfs(tfs, argv, argn);
			}

		} else if(strncasecmp(arg[1], "APLAY", 5) == 0 && arg[2] != NULL) {

			TFS_HEADER* tfs = (TFS_HEADER*) tfs_find(arg[2]);

			if(tfs == NULL) {
				print("=== AUDIO PLAY FAILED, MISSING FILE: %s\r\n", arg[2]);
			} else {
				print("=== AUDIO PLAYING FILE: %s\r\n", arg[2]);

				play_start((char*)tfs + sizeof(TFS_HEADER), tfs->size);
			}

		} else if(strncasecmp(arg[1], "COPY", 4) == 0 && arg[2] != NULL && arg[3] != NULL) {

			TFS_HEADER* tfs_src = tfs_find(arg[2]);

			if(tfs_src == NULL) {
				print("=== COPY FAILED, MISSING FILE: %s\r\n", arg[2]);
			} else {
				TFS_HEADER* tfs_dst = tfs_create_file(arg[3], strnlen(arg[3], 256), tfs_src->size);

				if(tfs_dst == NULL) {
					print("=== COPY FAILED, CANNOT CREATE FILE: %s, SIZE: %d BYTES\r\n", arg[2], tfs_src->size);
				} else {

					int rc = tfs_write_block(tfs_dst, (uint8_t*)tfs_src + sizeof(TFS_HEADER), tfs_src->size, 0); 

					tfs_close(tfs_dst);

					print("=== COPIED %d BYTES FROM %s TO %s\r\n", rc, arg[2], arg[3]);
				}
			}

		} else if(strncasecmp(arg[1], "DECODE", 4) == 0 && arg[2] != NULL && arg[3] != NULL) {

			TFS_HEADER* tfs_src = tfs_find(arg[2]);

			if(tfs_src == NULL) {
				print("*** DECODE FAILED, MISSING FILE: %s\r\n", arg[2]);
			} else {
				TFS_HEADER* tfs_dst = tfs_create_file(arg[3], strnlen(arg[3], 256), tfs_src->size*2);

				if(tfs_dst == NULL) {
					print("=== DECODE FAILED, CANNOT CREATE FILE: %s, SIZE: %d BYTES\r\n", arg[2], tfs_src->size*2);
				} else {

					char dst_buf[160*2];
					uint8_t *src = (uint8_t*)tfs_src + sizeof(TFS_HEADER);

					for(int i = 0; i < tfs_src->size / 160; i++) {
						pcmu_decode(dst_buf, src, 160);
						tfs_write_block(tfs_dst, dst_buf, 160*2, i*160*2); 
						src += 160;
					}

					tfs_close(tfs_dst);

					print("=== DECODED %d BYTES FROM %s TO %s\r\n", tfs_src->size*2, arg[2], arg[3]);
				}
			}

		} else if(strncasecmp(arg[1], "FORMAT", 6) == 0) {

			stop_application((void*)application_text_addr);

			print("=== ERASING FILE SYSTEM\r\n");

			main_queue_enabled = 0;
			debug_mode = DEBUG_TO_SWD;

			tfs_format();

			debug_mode = DEBUG_TO_USART1_AND_SWD;
			main_queue_enabled = 1;

		} else if(strncasecmp(arg[1], "CREATE", 5) == 0) {

			TFS_HEADER* tfs = tfs_create_file(arg[2], strlen(arg[2]), strtol(arg[3], NULL, 0));

			if(tfs) {
				print("=== CREATED FILE, TFS: %p\r\n", tfs);
			} else {
				print("*** FILE CREATION ERROR\r\n");
			}
		}

	} else if(strncasecmp(cmd, "STOP", 4) == 0) {


		if(application_process_msg_addr) {

			stop_application((void*)application_text_addr);

			print("=== APPLICATION STOPPED\r\n");
		} else {
			print("*** APPLICATION WAS NOT RUNNING\r\n");
		}

	} else if(strncasecmp(cmd, "APP", 3) == 0) {
		if(arg[1]) { 
			application_start_addr = (int(*)(void*, char**, int)) strtol(arg[1], NULL, 0);
			print("=== NEW APP MSG PROCESSOR: %p\r\n", application_start_addr);

			char *argv[4];
			int argn = 0;
				
			if(arg[1]) {argv[0] = arg[1]; argn++; }
			if(arg[2]) {argv[1] = arg[2]; argn++; }
			if(arg[3]) {argv[2] = arg[3]; argn++; }
			if(arg[4]) {argv[3] = arg[4]; argn++; }

			application_process_msg_addr = (void (*)(MSG*)) application_start_addr(NULL, argv, argn); // RUN
		} else {
			print("*** APP MSG ADDRESS MUST BE PROVIDED!\r\n");
		}
	} else if(strncasecmp(cmd, "REBOOT", 6) == 0) {
		print("=== SYSTEM REBOOTING...\r\n");
		DelayLoop(10000);
		NVIC_SystemReset();
	} else if(strncasecmp(cmd, "RTC", 5) == 0) {
		if(strncasecmp(arg[1], "ON", 2) == 0) {
			config_active.use_rtc = 1;
			RTC_init();
		} else if(strncasecmp(arg[1], "OFF", 3) == 0) {
			config_active.use_rtc = 0;
		}
		print("=== REAL-TIME CLOCK IS %s\r\n", config_active.use_rtc ? "ON":"OFF");
	} else if(strncasecmp(cmd, "DEBUG", 5) == 0) {
		if(strncasecmp(arg[1], "ON", 2) == 0) {
			event_logging = 1;
		} else if(strncasecmp(arg[1], "OFF", 3) == 0) {
			event_logging = 0;
		} else if(arg[1]) {
			event_logging = strtoll(arg[1], (char **)NULL, 0);
		}
		print("=== DEBUGGING LEVEL: %d\r\n", event_logging);
	} else if(strncasecmp(cmd, "RESET", 5) == 0) {
                if(strncasecmp(arg[1], "CONFIG", 6) == 0) {
			config_active = *config_flash;	
			save_config();
			load_config();
                	print("=== CONFIG HAS BEEN RETURNED TO FACTORY DEFAULTS\r\n");
                } else if(strncasecmp(arg[1], "USER_CONFIG", 11) == 0) {
			memset(config_active.user_data, 0 , sizeof(config_active.user_data));
			save_config();
                        load_config();
			print("=== USER CONFIG HAS BEEN CLEARED\r\n");
                }

	#ifdef LCD_SPI
	} else if(strncasecmp(cmd, "LCD", 3) == 0) {
		if(strncasecmp(arg[1], "ON", 2) == 0) {
			config_active.lcd.enabled = 1;
			lcd_init();
	                draw_fb();
        	        Fbinfo* fb=fbqueue_get_head();
                	if(fb)
                        	lcd_write_memory_dma_irq(RAM_G+FRAMEBUFFER_SIZE_BYTES*fb->img_index, (uint8_t*)fb->framebuffer, FRAMEBUFFER_SIZE_BYTES);

		} else if(strncasecmp(arg[1], "OFF", 3) == 0) {
			config_active.lcd.enabled = 0;
		}
		print("=== LCD IS %s\r\n", config_active.lcd.enabled ? "ON":"OFF");
	#endif // LCD_SPI

	} else if(strncasecmp(cmd, "ADC", 3) == 0) {
		if(strncasecmp(arg[1], "ON", 2) == 0) {
			config_active.adc.enabled = 1;
			adc_stop();
			adc_init();
		} else if(strncasecmp(arg[1], "PERIOD", 6) == 0) {
			config_active.adc.tim_period = atoi(arg[2]);
			if(config_active.adc.enabled) {
				adc_stop();
				adc_init();
			}
		} else if(strncasecmp(arg[1], "MODE", 4) == 0) {
			if(strncasecmp(arg[2], "CONT", 4) == 0) {
				print("=== ADC MODE: %s\r\n", "CONTINUOUS");
				config_active.adc.mode = ADC_MODE_CONTINUOUS;
			} else if(strncasecmp(arg[2], "SINGLE", 6) == 0) {
				print("=== ADC MODE: %s\r\n", "SINGLE");
				config_active.adc.mode = ADC_MODE_SINGLE;
			} else if(strncasecmp(arg[2], "INTER", 5) == 0) {
				print("=== ADC MODE: %s\r\n", "INTER");
				config_active.adc.mode = ADC_MODE_INTERFRAME;
			} else {
				print("=== WRONG ADC MODE: %s\r\n", arg[2]);
			}

			if(config_active.adc.enabled) {
				adc_stop();
				adc_init();
			}

		} else if(strncasecmp(arg[1], "SAMPLES", 7) == 0) {
			config_active.adc.samples = atoi(arg[2]);
		} else if(strncasecmp(arg[1], "COEF", 4) == 0) {
			int i = atoi(arg[2]);
			if(i >=0 && i < ADC_NUM_OF_CHANNELS) {
				config_active.adc.coeff[i] = atof(arg[3]);
				print("=== ADC[%d] coefficient = %f\r\n", i, config_active.adc.coeff[i]);
			} else {
				print("=== WRONG ADC CHANNEL: %d\r\n", i);
			}
		} else if(strncasecmp(arg[1], "OFFSET", 6) == 0) {
			int i = atoi(arg[2]);
			if(i >=0 && i < ADC_NUM_OF_CHANNELS) {
				config_active.adc.offset[i] = atof(arg[3]);
				print("=== ADC[%d] offset = %f\r\n", i, config_active.adc.offset[i]);
			} else {
				print("=== WRONG ADC CHANNEL: %d\r\n", i);
			}
		} else if(strncasecmp(arg[1], "CALOFF", 6) == 0) {
			int i = atoi(arg[2]);
			if(i >=0 && i < ADC_NUM_OF_CHANNELS) {
				float old_coef = config_active.adc.coeff[i];
				config_active.adc.coeff[i] = 1.0;
				config_active.adc.offset[i] = 0.0;
				ADC_DATA.adc_avg[i] = 0.0;
				ADC_DATA.adc_rms[i] = 0.0;
				print("=== CALIBRATING OFFSET ADC[%d]", i);
				for(int j = 0; j < 3; j++) {
					Delay(1000);
					_print(".");
				}
				_print("\r\n");
				config_active.adc.offset[i] = ADC_DATA.adc_avg[i];
				config_active.adc.coeff[i] = old_coef;
				print("=== ADC[%d] OFFSET CALC COMPLETE: offset = %f\r\n", i, config_active.adc.offset[i]);
			} else {
				print("=== WRONG ADC CHANNEL: %d\r\n", i);
			}
			goto end;
		} else if(strncasecmp(arg[1], "CALCOEF", 7) == 0) {
			int i = atoi(arg[2]);
			if(i >=0 && i < ADC_NUM_OF_CHANNELS) {
				float ref_coef = atof(arg[3]);
				if(ref_coef == 0) {
					print("=== REF VALUE CANNOT BE ZERO!\r\n");
					goto end;
				}
				config_active.adc.coeff[i] = 1.0;
				ADC_DATA.adc_avg[i] = 0.0;
				ADC_DATA.adc_rms[i] = 0.0;
				print("=== CALIBRATING COEF ADC[%d]", i);
				for(int j = 0; j < 3; j++) {
					Delay(1000);
					_print(".");
				}
				_print("\r\n");
				config_active.adc.coeff[i] = ref_coef / ADC_DATA.adc_rms[i];
				print("=== ADC[%d] COEF CALC COMPLETE: coef = %f, ref = %f\r\n", i, config_active.adc.coeff[i], ref_coef);
			} else {
				print("=== WRONG ADC CHANNEL: %d\r\n", i);
			}
			goto end;
		} else if(strncasecmp(arg[1], "OFF", 3) == 0) {
			adc_stop();
			config_active.adc.enabled = 0;
		}


		print("=== ADC IS %s, MODE: %s, PERIOD: %d us, SAMPLES: %d per event\r\n", config_active.adc.enabled ? "ON":"OFF", config_active.adc.mode == 0 ? "CONTINUOUS":(config_active.adc.mode == 1 ? "SINGLE" : "INTERFRAME"), config_active.adc.tim_period, config_active.adc.samples);

		if(config_active.adc.enabled && config_active.adc.mode == ADC_MODE_SINGLE)
			adc_poll(ADC_MODE_SINGLE_TIMEOUT);

		for(int i = 0; i < ADC_NUM_OF_CHANNELS; i++) {
			print("ADC[%d]: coef = %f, offset = %f, avg = %f, rms = %f\r\n", i, config_active.adc.coeff[i], config_active.adc.offset[i], ADC_DATA.adc_avg[i], ADC_DATA.adc_rms[i]);
		}
	} else if(strncasecmp(cmd, "DEFAULT", 7) == 0) {
			if(arg[1]) {
				strncpy(config_active.default_application_file_name, arg[1], sizeof(config_active.default_application_file_name));
			}
			print("=== DEFAULT APPLICATION: %s\r\n", config_active.default_application_file_name);
	} else if(strncasecmp(cmd, "LOGGER", 6) == 0) {
		if(strncasecmp(arg[1], "ON", 2) == 0) {
			config_active.logger_enabled = 1;
			logger_init();
		} else if(strncasecmp(arg[1], "OFF", 3) == 0) {
			config_active.logger_enabled = 0;
		}
		print("=== LOGGER IS %s\r\n", config_active.logger_enabled ? "ON":"OFF");
			
	} else if(strncasecmp(cmd, "PWM", 3) == 0) {
		if(strncasecmp(arg[1], "SET", 3) == 0) {
			uint32_t port = strtol(arg[2], (char **)NULL, 0);
			float duty = (float) strtol(arg[3], (char **)NULL, 0);
			ext_pwm_set(port, duty);
			duty = ext_pwm_get(port);
			print("PWM%d	SET DUTY TO	%.1f%%\r\n", port, duty);
		} else if(strncasecmp(arg[1], "GET", 3) == 0) {
			uint32_t port = strtol(arg[2], (char **)NULL, 0);
			float duty = ext_pwm_get(port);
			print("PWM%d	duty = %.1f%%\r\n", port, duty);
		} else if(strncasecmp(arg[1], "CLOCK", 5) == 0) {
			uint32_t port = strtol(arg[2], (char **)NULL, 0);
			uint32_t clock = strtol(arg[3], (char **)NULL, 0);
			print("PWM%d	SET CLOCK TO	%d HZ\r\n", port, ext_pwm_set_clock(port, clock));
		} else {
			for(int port = 0; port < 8; port++) {
				float duty = ext_pwm_get(port);
				print("PWM%d	duty = %.1f%%\r\n", port, duty);
			}
		}
		
	} else if(strncasecmp(cmd, "EXTIRQ", 6) == 0) {
		print("=== EXT IRQ PROC LIST: size = %d\r\n", IRQ_LIST_SIZE);
		for(int i = 0; i < IRQ_LIST_SIZE; i++) {
			print("EXT IRQ[%d]:	proc = %p,	owner = %p\r\n", i, irq_proc_list[i].proc, irq_proc_list[i].owner); 
		}

#ifdef	GPIO_CHANNELS
	} else if(strncasecmp(cmd, "GPIO", 4) == 0) {
		if(strncasecmp(arg[1], "ON", 2) == 0) {
			config_active.gpios_enabled = 1;
			ext_gpio_init();
			print("=== GPIOS ARE %s\r\n", config_active.gpios_enabled ? "ON":"OFF");
		} else if(strncasecmp(arg[1], "OFF", 3) == 0) {
			config_active.gpios_enabled = 0;
			print("=== GPIOS ARE %s\r\n", config_active.gpios_enabled ? "ON":"OFF");
		} else if(strncasecmp(arg[1], "SET", 3) == 0) {
			uint32_t gpio = strtol(arg[2], (char **)NULL, 0);
			uint32_t val = strtol(arg[3], (char **)NULL, 0);
			if((val = ext_gpio_set(gpio, val)) < 0) {
				print("GPIO%d CANNOT BE %s, ERROR %d\r\n", gpio, "SET", val);
			} else {
				print("GPIO%d SET TO %d\r\n", gpio, val);
			}
		} else if(strncasecmp(arg[1], "GET", 3) == 0) {
			int val;
			uint32_t gpio = strtol(arg[2], (char **)NULL, 0);
			if((val = ext_gpio_get(gpio)) < 0) {
				print("GPIO%d CANNOT BE %s, ERROR %d\r\n", gpio, "GET", val);
			} else {
				print("GPIO%d = %d\r\n", gpio, val);
			}
		} else if(strncasecmp(arg[1], "IRQ", 3) == 0) {
			if(strncasecmp(arg[2], "ON", 2) == 0) {
				uint32_t gpio = strtol(arg[2], (char **)NULL, 0);
				print("GPIO%d IRQ %s\r\n", gpio, "ENABLED");
				ext_gpio_irq(gpio, 1);
			} else if(strncasecmp(arg[2], "OFF", 3) == 0) {
				uint32_t gpio = strtol(arg[2], (char **)NULL, 0);
				print("GPIO%d IRQ %s\r\n", gpio, "DISABLED");
				ext_gpio_irq(gpio, 0);
			} else {
			}
		} else {
			print("=== GPIOS ARE %s\r\n", config_active.gpios_enabled ? "ON":"OFF");
			if(config_active.gpios_enabled) {
				for(int i = 0; i < GPIO_CHANNELS; i++) {
					print("GPIO%d:	%s	=	%d	%s	%s\r\n", i, (ext_gpios[i].mode == GPIO_Mode_IN ? "IN" : "OUT"), ext_gpio_get(i), (ext_gpios[i].irq_line ? "IRQ":""), ext_gpios[i].descr);
				}
			}
		}

#endif
			
        } else if(strncasecmp(cmd, "MICROSEC_TIMER", 14) == 0) {
                if(strncasecmp(arg[1], "ON", 2) == 0) {
                        config_active.microsec_timer_enabled = 1;
                } else if(strncasecmp(arg[1], "OFF", 3) == 0) {
                        config_active.microsec_timer_enabled = 0;
                }

                print("=== MICROSEC_TIMER IS %s\r\n", config_active.microsec_timer_enabled ? "ON":"OFF");

#ifdef EXT_SPI
	} else if(strncasecmp(cmd, "SPI", 3) == 0) {
		if(strncasecmp(arg[1], "SEND8", 5) == 0 && arg[2]) {
			uint32_t val = strtol(arg[2], (char **)NULL, 0);
			ext_spi_send8(val);
			print("=== SENT TO SPI: 0x%02X\r\n", val);
			goto end;

		} else if(strncasecmp(arg[1], "SEND16", 6) == 0 && arg[2]) {
			uint32_t val = strtol(arg[2], (char **)NULL, 0);
			ext_spi_send16(val);
			print("=== SENT TO SPI: 0x%04X\r\n", val);
			goto end;

		} else if(strncasecmp(arg[1], "SEND24", 6) == 0 && arg[2]) {
			uint32_t val = strtol(arg[2], (char **)NULL, 0);
			ext_spi_send24(val);
			print("=== SENT TO SPI: 0x%06X\r\n", val);
			goto end;

		} else if(strncasecmp(arg[1], "SEND32", 6) == 0 && arg[2]) {
			uint32_t val = strtoll(arg[2], (char **)NULL, 0);
			ext_spi_send32(val);
			print("=== SENT TO SPI: 0x%08X\r\n", val);
			goto end;

		} else if(strncasecmp(arg[1], "PRE", 3) == 0 && arg[2]) {
			uint32_t pre = strtol(arg[2], (char **)NULL, 0);
			config_active.spi.prescaler = pre;
			ext_spi_init();
		} else if(strncasecmp(arg[1], "CPOL", 4) == 0 && arg[2]) {
			uint32_t cpol = strtol(arg[2], (char **)NULL, 0);
			if(cpol)
				config_active.spi.flags |= 2;
			else
				config_active.spi.flags &= ~2;
			ext_spi_init();
		} else if(strncasecmp(arg[1], "CPHA", 4) == 0 && arg[2]) {
			uint32_t cpha = strtol(arg[2], (char **)NULL, 0);
			if(cpha)
				config_active.spi.flags |= 4;
			else
				config_active.spi.flags &= ~4;
			ext_spi_init();
		} else if(strncasecmp(arg[1], "MODE", 3) == 0 && arg[2]) {
			if(strncasecmp(arg[2], "CONT", 4) == 0) {
				config_active.spi.mode = EXT_SPI_MODE_CONTINUOUS;
				ext_spi_init();
			} else if(strncasecmp(arg[2], "SINGLE", 6) == 0) {
				config_active.spi.mode = EXT_SPI_MODE_SINGLE;
				ext_spi_init();
			}
		} else if(strncasecmp(arg[1], "ON", 2) == 0) {
			config_active.spi.enabled = 1;
			ext_spi_init();
		} else if(strncasecmp(arg[1], "OFF", 3) == 0) {
			config_active.spi.enabled = 0;
			ext_spi_deinit();
		} else if(strncasecmp(arg[1], "LSB", 3) == 0) {
			config_active.spi.flags &= ~(0x01) ;
			ext_spi_init();
		} else if(strncasecmp(arg[1], "MSB", 3) == 0) {
			config_active.spi.flags |= 0x01 ;
			ext_spi_init();
		} 

		print("=== SPI IS %s, MODE: %s, PRESCALER: %d, FIRSTBIT: %s, CPOL: %s, CPHA: %s, CR1: 0x%04X\r\n", config_active.spi.enabled ? "ON":"OFF", config_active.spi.mode == 0 ? "SINGLE":"CONTINOUS", config_active.spi.prescaler, config_active.spi.flags & 0x01 ? "MSB" : "LSB", config_active.spi.flags & 0x02 ? "HIGH":"LOW", config_active.spi.flags & 0x04 ? "1EDGE":"2EDGE", EXT_SPI->CR1);
#endif // EXT_SPI


#ifdef DALI1_MODE
	} else if(strncasecmp(cmd, "DALI1", 5) == 0) {
		if(strncasecmp(arg[1], "MODE", 4) == 0 && arg[2]) {
			if(strncasecmp(arg[2], "MASTER", 6) == 0) {
				config_active.dali1.mode = DALI_MASTER;
				dali1_init();
			} else if(strncasecmp(arg[2], "SLAVE", 5) == 0) {
				config_active.dali1.mode = DALI_SLAVE;
				dali1_init();
			} else if(strncasecmp(arg[2], "OFF", 3) == 0) {
				config_active.dali1.mode = DALI_NONE;
				dali1_init();
			}
			print("=== %s MODE SET: %d\r\n", "DALI1", config_active.dali1.mode);
		} else if(strncasecmp(arg[1], "LIGHT", 5) == 0 && arg[2] && arg[3]) {
			uint8_t addr = strtoll(arg[2], (char **)NULL, 0) & 0x3f;
			uint8_t pwr = strtoll(arg[3], (char **)NULL, 0);
			DALI_TRANSMIT_CMD(&dali1_req1, addr << 1, pwr, 100, 0);
			dali1_enqueue_request(&dali1_req1);
			print("=== %s LIGHT: addr = %d, pwr = %d\r\n", "DALI1", addr, pwr);
		} else if(strncasecmp(arg[1], "PING", 4) == 0 && arg[2]) {
			uint8_t addr = strtoll(arg[2], (char **)NULL, 0) & 0x3f;
			DALI_TRANSMIT_REQUEST(&dali1_req1, (addr << 1) | 0x01, DALI_REQ_STATUS, 3000, DALI1_RESPONSE_RECEIVED, DALI1_RESPONSE_NOT_RECEIVED);
			dali1_enqueue_request(&dali1_req1);
		} else if(strncasecmp(arg[1], "REQ", 3) == 0 && arg[2] && arg[3]) {
			DALI_TRANSMIT_REQUEST(&dali1_req1, strtoll(arg[2], (char **)NULL, 0), strtoll(arg[3], (char **)NULL, 0), 3000, DALI1_RESPONSE_RECEIVED, DALI1_RESPONSE_NOT_RECEIVED);
			dali1_enqueue_request(&dali1_req1);
			print("=== %s REQUEST SENT: {0x%02X, 0x%02X}\r\n", "DALI1", dali1_req1.txbuf[0], dali1_req1.txbuf[1]);
		} else if(strncasecmp(arg[1], "CMD", 3) == 0 && arg[2] && arg[3]) {
			DALI_TRANSMIT_CMD(&dali1_req1, strtoll(arg[2], (char **)NULL, 0), strtoll(arg[3], (char **)NULL, 0), 100, 0);
			dali1_enqueue_request(&dali1_req1);
			print("=== %s CMD: 0x%2X 0x%02X\r\n", "DALI1", dali1_req1.txbuf[0], dali1_req1.txbuf[1]);
		} else if(strncasecmp(arg[1], "MANCHPOL", 8) == 0 && arg[2]) {
			config_active.dali1.manchester_pol = strtoll(arg[2], (char **)NULL, 0);
			print("=== %s MANCHESTER POLARITY: %d\r\n", "DALI1", config_active.dali1.manchester_pol);
		} else if(strncasecmp(arg[1], "TXRXPOL", 7) == 0 && arg[2]) {
			config_active.dali1.txrx_pol = strtoll(arg[2], (char **)NULL, 0);
			print("=== %s TXRX POLARITY: %d\r\n", "DALI1", config_active.dali1.txrx_pol);
		} else if(strncasecmp(arg[1], "BUSTEST", 7) == 0) {
			DALI_TRANSMIT_CMD(&dali1_req1, DALI_BROADCAST_CMD, DALI_CMD_OFF_C, 500, 0);
			dali1_enqueue_request(&dali1_req1);
			DALI_TRANSMIT_CMD(&dali1_req2, DALI_BROADCAST_CMD, DALI_CMD_ON_C, 100, 0);
			dali1_enqueue_request(&dali1_req2);
			print("=== %s BUS TEST INITIATED\r\n", "DALI1");
		} else {
			print("=== %s MODE: %d, addr: %d, manch_pol: %d, txrx_pol: %d\n", "DALI1", config_active.dali1.mode, config_active.dali1.short_addr, config_active.dali1.manchester_pol, config_active.dali1.txrx_pol);
			print("Usage: %s [MODE [MASTER|SLAVE|OFF] | [MANCHPOL <1|0>] | [TXRXPOL <0|1>] | CMD <cmd1> <cmd2> | LIGHT <addr> <pwr> | BUSTEST]\r\n", "DALI1");
		}
#endif

#ifdef USART_EXT1 
	} else if(strncasecmp(cmd, "EXT1", 4) == 0) {
		if(strncasecmp(arg[1], "BAUD", 4) == 0 && arg[2]) {
			int baud = atoi(arg[2]);
			for(int i = 0; i < BAUD_RATES; i++) {
				if(baud_rates[i] == baud) {
					config_active.ext1_port_baud_rate = i;
					USART2_init();
					break;
				}
			}
			print("=== EXTENTION PORT1 (RS485) SETTINGS:\r\n");
			print("Baud rate: %d\r\n", baud_rates[config_active.ext1_port_baud_rate]);
			
		} else if(strncasecmp(arg[1], "STOP", 4) == 0 && arg[2]) {
			for(int i = 0; i < STOP_BITS; i++) {
				if(strncmp(stop_bits[i], arg[2], 3) == 0) {
					config_active.ext1_port_stop_bits = i;
					USART2_init();
					break;
				}
			}
			print("=== EXTENTION PORT1 (RS485) SETTINGS:\r\n");
			print("Stop bits: %s\r\n", stop_bits[config_active.ext1_port_stop_bits]);

		} else if(strncasecmp(arg[1], "PARITY", 6) == 0 && arg[2]) {
			if(arg[2][0] == 'E' || arg[2][0] == 'e')
				config_active.ext1_port_parity  = 1;
			else if(arg[2][0] == 'O' || arg[2][0] == 'o')
				config_active.ext1_port_parity  = 2;
			else
				config_active.ext1_port_parity  = 0;
			
			USART2_init();

			print("=== EXTENTION PORT1 (RS485) SETTINGS:\r\n");
			print("Parity: %s\r\n", config_active.ext1_port_parity ? (config_active.ext1_port_parity == 1 ? "E" : "O") : "N");

		} else if(strncasecmp(arg[1], "MODE", 4) == 0 && arg[2]) {
			
			if(strncasecmp(arg[2], "MASTER", 6) == 0) {
				config_active.ext1_port_modbus_mode = 1;
			} else if(strncasecmp(arg[2], "SLAVE", 5) == 0) {
				config_active.ext1_port_modbus_mode = 2;

				MODBUS_CREATE_RESPONDER(&modbus_hold_regs_read_ext1, MODBUS1_READ_HOLD_REGS, FUNC_READ_HOLD_REGS, 1, 999);
				modbus_register_responder_ext1(&modbus_hold_regs_read_ext1);

				MODBUS_CREATE_RESPONDER(&modbus_hold_regs_write_ext1, MODBUS1_WRITE_HOLD_REGS, FUNC_WRITE_MANY_HOLD_REGS, 1, 999);
				modbus_register_responder_ext1(&modbus_hold_regs_write_ext1);

				print("init() Modbus responders registered for EXT1\r\n");
			} else {
				config_active.ext1_port_modbus_mode = 0;
			}


			print("=== EXTENTION PORT1 (RS485) SETTINGS:\r\n");
			print("Modbus mode: %s\r\n", config_active.ext1_port_modbus_mode ? (config_active.ext1_port_modbus_mode == 1 ? "MASTER" : "SLAVE") : "OFF");

		} else if(strncasecmp(arg[1], "ADDR", 4) == 0 && arg[2]) {
			int addr = strtol(arg[2], (char **)NULL, 0);
			if(addr > 0 && addr < 256) {
				config_active.ext1_port_modbus_addr = addr;
			}

			print("=== EXTENTION PORT1 (RS485) SETTINGS:\r\n");
			print("Modbus addr: %d\r\n", config_active.ext1_port_modbus_addr);

		} else if(strncasecmp(arg[1], "KEY", 3) == 0 && arg[2]) {
			uint32_t key = strtol(arg[2], (char **)NULL, 0);
			config_active.ext1_port_modbus_key = key;

			print("=== EXTENTION PORT1 (RS485) SETTINGS:\r\n");
			print("Modbus key: 0x%08X\r\n", config_active.ext1_port_modbus_key);

		} else {
			print("=== EXTENTION PORT1 (RS485) SETTINGS:\r\n");
			print("Baud rate: %d\r\n", baud_rates[config_active.ext1_port_baud_rate]);
			print("Parity: %s\r\n", config_active.ext1_port_parity ? (config_active.ext1_port_parity == 1 ? "E" : "O") : "N");
			print("Stop bits: %s\r\n", stop_bits[config_active.ext1_port_stop_bits]);
			print("Modbus mode: %s\r\n", config_active.ext1_port_modbus_mode ? (config_active.ext1_port_modbus_mode == 1 ? "MASTER" : "SLAVE") : "OFF");
			print("Modbus addr: %d\r\n", config_active.ext1_port_modbus_addr);
			print("Modbus key: 0x%08X\r\n", config_active.ext1_port_modbus_key);
		}
#endif // USART_EXT1 

#ifdef USART_EXT2 
	} else if(strncasecmp(cmd, "EXT2", 4) == 0) {
		if(strncasecmp(arg[1], "BAUD", 4) == 0 && arg[2]) {
			int baud = atoi(arg[2]);
			for(int i = 0; i < BAUD_RATES; i++) {
				if(baud_rates[i] == baud) {
					config_active.ext2_port_baud_rate = i;
					USART3_init();
					break;
				}
			}
			print("=== EXTENTION PORT2 (RS485) SETTINGS:\r\n");
			print("Baud rate: %d\r\n", baud_rates[config_active.ext2_port_baud_rate]);
			
		} else if(strncasecmp(arg[1], "STOP", 4) == 0 && arg[2]) {
			for(int i = 0; i < STOP_BITS; i++) {
				if(strncmp(stop_bits[i], arg[2], 3) == 0) {
					config_active.ext2_port_stop_bits = i;
					USART3_init();
					break;
				}
			}
			print("=== EXTENTION PORT2 (RS485) SETTINGS:\r\n");
			print("Stop bits: %s\r\n", stop_bits[config_active.ext2_port_stop_bits]);

		} else if(strncasecmp(arg[1], "PARITY", 6) == 0 && arg[2]) {
			if(arg[2][0] == 'E' || arg[2][0] == 'e')
				config_active.ext2_port_parity  = 1;
			else if(arg[2][0] == 'O' || arg[2][0] == 'o')
				config_active.ext2_port_parity  = 2;
			else
				config_active.ext2_port_parity  = 0;
			
			USART3_init();

			print("=== EXTENTION PORT2 (RS485) SETTINGS:\r\n");
			print("Parity: %s\r\n", config_active.ext2_port_parity ? (config_active.ext2_port_parity == 1 ? "E" : "O") : "N");

		} else if(strncasecmp(arg[1], "MODE", 4) == 0 && arg[2]) {
			
			if(strncasecmp(arg[2], "MASTER", 6) == 0) {
				config_active.ext2_port_modbus_mode = MODBUS_MASTER;
			} else if(strncasecmp(arg[2], "SLAVE", 5) == 0) {
				config_active.ext2_port_modbus_mode = 2;

				MODBUS_CREATE_RESPONDER(&modbus_hold_regs_read_ext2, MODBUS2_READ_HOLD_REGS, FUNC_READ_HOLD_REGS, 1, 999);
				modbus_register_responder_ext2(&modbus_hold_regs_read_ext2);

				MODBUS_CREATE_RESPONDER(&modbus_hold_regs_write_ext2, MODBUS2_WRITE_HOLD_REGS, FUNC_WRITE_MANY_HOLD_REGS, 1, 999);
				modbus_register_responder_ext2(&modbus_hold_regs_write_ext2);

				print("init() Modbus responders registered for EXT2\r\n");
			} else {
				config_active.ext2_port_modbus_mode = 0;
			}


			print("=== EXTENTION PORT2 (RS485) SETTINGS:\r\n");
			print("Modbus mode: %s\r\n", config_active.ext1_port_modbus_mode ? (config_active.ext2_port_modbus_mode == 1 ? "MASTER" : "SLAVE") : "OFF");

		} else if(strncasecmp(arg[1], "ADDR", 4) == 0 && arg[2]) {
			int addr = strtol(arg[2], (char **)NULL, 0);
			if(addr > 0 && addr < 256) {
				config_active.ext2_port_modbus_addr = addr;
			}

			print("=== EXTENTION PORT2 (RS485) SETTINGS:\r\n");
			print("Modbus addr: %d\r\n", config_active.ext2_port_modbus_addr);

		} else if(strncasecmp(arg[1], "KEY", 3) == 0 && arg[2]) {
			uint32_t key = strtol(arg[2], (char **)NULL, 0);
			config_active.ext2_port_modbus_key = key;

			print("=== EXTENTION PORT2 (RS485) SETTINGS:\r\n");
			print("Modbus key: 0x%08X\r\n", config_active.ext2_port_modbus_key);

		} else {
			print("=== EXTENTION PORT2 (RS485) SETTINGS:\r\n");
			print("Baud rate: %d\r\n", baud_rates[config_active.ext2_port_baud_rate]);
			print("Parity: %s\r\n", config_active.ext2_port_parity ? (config_active.ext2_port_parity == 1 ? "E" : "O") : "N");
			print("Stop bits: %s\r\n", stop_bits[config_active.ext2_port_stop_bits]);
			print("Modbus mode: %s\r\n", config_active.ext2_port_modbus_mode ? (config_active.ext2_port_modbus_mode == 1 ? "MASTER" : "SLAVE") : "OFF");
			print("Modbus addr: %d\r\n", config_active.ext2_port_modbus_addr);
			print("Modbus key: 0x%08X\r\n", config_active.ext2_port_modbus_key);
		}
#endif // USART_EXT2 

	} else if(strncasecmp(cmd, "SHOW", 4) == 0) {
		if(strncasecmp(arg[1], "RUN", 3) == 0) {

			if(application_tfs) 
				print("application_name: %s\r\n", application_tfs->name);
			print("application_tfs: %p\r\n", application_tfs);
			print("application_elf: %p\r\n", application_elf);
			print("application_start_addr: %p\r\n", application_start_addr);
			print("application_process_msg_addr: %p\r\n", application_process_msg_addr);
			print("Exception_code: %p\r\n", exception_code);
			for(int i = 0; i < APPLICATION_MAX_MEM; i++)
				if(application_memory[i])
					print("application memory[%d] @ %p\r\n", i, application_memory[i]);

			int ht = DMA_GetITStatus(DMA1_Stream5, DMA_IT_HTIF5);
                	int tc = DMA_GetITStatus(DMA1_Stream5, DMA_IT_TCIF5);
			int fe = DMA_GetITStatus(DMA1_Stream5, DMA_IT_FEIF5);
			int dme = DMA_GetITStatus(DMA1_Stream5, DMA_IT_DMEIF5);
			int te = DMA_GetITStatus(DMA1_Stream5, DMA_IT_TEIF5);

			print("DMA1_Stream5 status: tc = %d, ht = %d, fe = %d, dme = %d, te = %d, DAC_IT_DMAUDR = %d\r\n", tc, ht, fe, dme, te, DAC_GetITStatus(DAC_Channel_1, DAC_IT_DMAUDR));
			print("TIM6->CNT: %p, TIM6->ARR: %p, TIM6->PSC: %p, TIM6->CR1: %p, TIM6->CR2: %p\r\n", TIM6->CNT, TIM6->ARR, TIM6->PSC, TIM6->CR1, TIM6->CR2);


		} else if(strncasecmp(arg[1], "DEFAULT", 7) == 0) {
			print("=== DEFAULT APPLICATION: %s\r\n", config_active.default_application_file_name);
		} else {
			goto usage;
		}
	} else if(strncasecmp(cmd, "**B01000000", 11) == 0 || strncasecmp(cmd, "RZ", 2) == 0) {

			print("=== RECEIVING FILE USING ZMODEM\r\n");

			TFS_HEADER* tfs = zmodem_load_file(15);

			if(tfs) {
				print("=== RECEIVED FILE: %s\r\n", tfs->name);
			} else {
				print("=== FAILED TO RECEIVE FILE\r\n", tfs);
			}
	} else if(strncasecmp(cmd, "HELP", 4) == 0) {
		goto usage;
	} else {

		TFS_HEADER *tfs = tfs_find(cmd);

		if(tfs) {
			char *argv[5];
			int argn = 0;
				
			if(cmd) {argv[0] = cmd; argn++; }
			if(arg[1]) {argv[1] = arg[1]; argn++; }
			if(arg[2]) {argv[2] = arg[2]; argn++; }
			if(arg[3]) {argv[3] = arg[3]; argn++; }
			if(arg[4]) {argv[4] = arg[4]; argn++; }

			exec_tfs(tfs, argv, argn);
		} else {
			char exec_name[128];
			strncpy(exec_name, cmd, 128);
			strncat(exec_name, ".elf", 128);

			tfs = tfs_find(exec_name);
			if(tfs) {
				char *argv[5];
				int argn = 0;
				
				if(exec_name) {argv[0] = exec_name; argn++; }
				if(arg[1]) {argv[1] = arg[1]; argn++; }
				if(arg[2]) {argv[2] = arg[2]; argn++; }
				if(arg[3]) {argv[3] = arg[3]; argn++; }
				if(arg[4]) {argv[4] = arg[4]; argn++; }

				exec_tfs(tfs, argv, argn);
			} else {
				print("*** COMMAND NOT RECOGNIZED: %s:\r\n", cli_buf);
			}
		}
	} 

	goto end;

	usage:


	print("GroveOS ver %04d - %s\r\n", BUILD_NUMBER, OS_PRODUCT_NAME); 
	print("Usage:\r\n"
	"[\r\n\tTFS [LIST [addr]|COPY <from> <to>|DEL <file>|INFO <file>|FORMAT|CREATE <file> <size>|EXEC <file>|RECEIVE] |\r\n"
	"\tAPP <addr> |\r\n"
	"\tDEBUG <on/off> |\r\n");
	_print(
	"\tADC [<on/off> MODE SINGLE/CONTINUOUS/INTERFRAME | PERIOD <msec> | SAMPLES <N> | COEF <chan> <val> | OFFSET <chan> < val>] | CALOFF <chan> | CALCOEF <chan> <ref>\r\n");
	_print(
	"\tSPI [on/off] | [PRE prescaler] | [MODE cont/single] | [SENT8/SEND16/SEND32] [CHPOL/CHPA]|\r\n"
	"\tINPUTS <on/off> |\r\n"
	"\tLCD <on/off> |\r\n");
	_print(
	"\tLOGGER [<0-60>] |\r\n"
	"\tEXT12 [BAUD <baudrate>] | [PARITY [E]|[O]|[N]] | [STOP [1|1.5|2|0.5] | [MODE [MASTER|SLAVE] | [ADDR <modbus_addr> ] |\r\n"
	"\tEXTIRQ |\r\n"
	"\tDALI12 [CMD | REQ | BUSTEST] |\r\n"
	"\tSHOW [RUN|DEFAULT|EXT1] |\r\n");
	_print(
	"\tTIME [SET <hh mm ss>] | [GET] |\r\n"
	"\tDATE [SET <yyyy mm dd>] [GET] |\r\n"
	"\tRTC [ON|OFF] |\r\n"
	"\tVAR [SETI/SETF <app_name> <var_name> <value>] | [GET <app_name> <var_name>] | [LIST] |\r\n");
	_print(
	"\tGPIO [SET|GET|CLOCK|ON|OFF] |\r\n"
	"\tRESET [CONFIG | USER_CONFIG] |\r\n"
	"\tSET var_name value |\r\n"
	"\tGET var_name |\r\n"
	"\tEXPORT |\r\n"
	"\tSAVE |\r\n"
	"\tRUN batch_file \r\n"
	"]\r\n"
	);

	end:

	prompt();

	_purge(USART_DEBUG);

	return 0;
}


TFS_HEADER* zmodem_load_file(int attempts)
{
	int rc;

	print("ZMODEM node has been started!\r\n");

	// Disable message queue processing and free serial line for ZMODEM
	main_queue_enabled = 0;
	debug_mode = DEBUG_TO_SWD;

	//rc = wcreceive(attempts, tfs_get_free_space()); // Initiate ZMODEM transmission
	rc = wcreceive(attempts, tfs_get_capacitance() ); // Initiate ZMODEM transmission

	DelayLoop(200); // wait for remote side to shut off 

	_purge(USART_DEBUG);

	// Restore debugging on serial line and message queue processing
	debug_mode = DEBUG_TO_USART1_AND_SWD;
	main_queue_enabled = 1;

	if(rc == 0) {
		print("\r\n\r\n");
		print("ZMODEM received file: %s, size: %d, crc16: 0x%04X\r\n", ZModemPathname, ZModemRxBytes, crc16_ccitt((const unsigned char *)ZModemTFS+ZModemTFS->size, ZModemRxBytes));
		return ZModemTFS;
	} else {
		print("\r\n\r\n");
		print("ZMODEM failed to transfer file\r\n", rc);
		if(ZModemBytesleft >  ZModemRxBytes) {
			print("File is too long, max allowed size: %d bytes\r\n", tfs_get_free_space());
		}
		return NULL;
	}
}


void process_message(MSG* msg) {

	if(event_logging) {
		if(msg->message != LCD_DMA_TC && msg->message != INFO_TIMER_INT && msg->message != MICRO_TIMER)
			print("process_message() msg = %d, queue_len = %d\r\n", msg->message, main_queue_len());
	}

	switch(msg->message) {
		case CHECK_FOR_AUTOEXEC: {
        		if(strcasestr(cli_buf, "XXX")) {
                		print("init() Autoexec will be aborted.\r\n");
				cli_buf[0] = 0;
				cli_buf_len = 0;
				prompt();
        		} else {
        			// Attempt to download application binary file

        			TFS_HEADER* tfs = zmodem_load_file(3);

                		if(tfs) {
                        		print("init() Running downloaded application...\r\n");
                        		exec_tfs(tfs, NULL, 0);
                		} else {
					// Check for AUTOEXEC.BAT 

					if((tfs = tfs_find("AUTOEXEC.BAT"))) {
						exec_shell(tfs);
					}

					// Check for default application
 
                        		if((tfs = tfs_find(config_active.default_application_file_name))) {
                                		print("init() Running default application: %s\r\n", tfs->name);
                                		exec_tfs(tfs, NULL, 0);
                        		} else {
                                		print("init() Application file %s not found!\r\n", config_active.default_application_file_name);
                        		}

					prompt();
                		}
        		}
		} break;

		case INFO_TIMER_INT: {
			info_timer_ms = msg->p1;
	
			if(info_timer_ms % 1000 == 0) {

				if(error_main_queue_isfull) print("WARN: main queue is full, %d events lost\r\n");
                                error_main_queue_isfull = 0;

				if(event_logging) {
					print("Timestamp: %d, lcd_update_counter: %d, main_queue_len: %d, skip_touches: %d\r\n", info_timer_ms, lcd_update_counter, main_queue_len(), skip_touch_count);
				}

				// Check for monitor invoke counter: if UP and DOWN buttons pressed and held for MONITO_INVOKE secs, then run nomitor.elf 

				#if defined(KEY_UP_GPIO_PORT) && defined(KEY_DOWN_GPIO_PORT)
				if(GPIO_ReadInputDataBit(KEY_UP_GPIO_PORT, KEY_UP_GPIO_PIN) == 0 &&
				   GPIO_ReadInputDataBit(KEY_DOWN_GPIO_PORT, KEY_DOWN_GPIO_PIN) == 0)  {
					monitor_invoke_counter++;

					if(monitor_invoke_counter++ >= MONITOR_INVOKE) {

						TFS_HEADER *tfs = tfs_find("monitor.elf");
						if(tfs) {
							print("Invoking monitor from TFS: %p\r\n", tfs);
							
							exec_tfs(tfs, NULL, 0);
						} else {
							print("Invoking monitor from system image: _binary_monitor_elf_size = %d, _binary_monitor_elf_start = %p\r\n", (int)&_binary_monitor_elf_size, &_binary_monitor_elf_start);
							exec_elf(&_binary_monitor_elf_start, (int)&_binary_monitor_elf_size);
						}

						monitor_invoke_counter = 0;
					}
				} else {
					monitor_invoke_counter = 0;
				}
				#endif


				#ifdef LCD_SPI
				if(config_active.lcd.enabled) {
					if(lcd_update_counter == 0) {
						Fbinfo *fb=fbqueue_get_head();
                        			print("RESTARTING LCD UPDATE!!!\r\n");
                        			if(fb) lcd_write_memory_dma_irq(RAM_G+FRAMEBUFFER_SIZE_BYTES*fb->img_index, (uint8_t*)fb->framebuffer, FRAMEBUFFER_SIZE_BYTES);
					}
					lcd_update_counter = 0;
				}
				#endif

				GPIO_ToggleBits(LED_GPIO_PORT, LED_GPIO_PIN);
				
			}

		} break;

		case ADC1_EVENT: {
			listener_update(msg->message, msg->p1, msg->p2);
		} break;


		case SVC_FORMAT_PENDING: {
			print("=== ERASING FILE SYSTEM\r\n");

			#ifdef OLED_DATA_GPIO_PORT
			OLED_WEG010032_CLEAR();
			OLED_WEG010032_PRINT(16, "Erasing FLASH...", 0, 0);
			#endif

			main_queue_enabled = 0;
			debug_mode = DEBUG_TO_SWD;

			tfs_format();

			debug_mode = DEBUG_TO_USART1_AND_SWD;
			main_queue_enabled = 1;

			#ifdef OLED_DATA_GPIO_PORT
			OLED_WEG010032_PRINT(16, "Format COMPLETED", 0, 0);
			#endif
		} break;

		case SVC_EXEC_PENDING: {
			print("SVC exec pending, TFS: %p\r\n", msg->p1);
			exec_tfs((TFS_HEADER*) msg->p1, NULL, 0);
		} break;

		case USART1_RX_DATA: {
			process_cmd();
		} break;


	#ifdef USART_EXT1 
		case USART2_RX_DATA: {
			modbus_rx_ext1();
		} break;

		case MODBUS1_RX_COMPLETE: {
			modbus_msg_ext1(msg->message, msg->p1, msg->p2);
		} break;
	#endif

	#ifdef USART_EXT2 
		case USART3_RX_DATA: {
			modbus_rx_ext2();
		} break;

		case MODBUS2_RX_COMPLETE: {
			modbus_msg_ext2(msg->message, msg->p1, msg->p2);
		} break;
	#endif


	#ifdef DALI1_MODE
		case DALI1_TX_DELAY_COMPLETE: 
		case DALI1_TX_COMPLETE: 
		case DALI1_RX_COMPLETE: {
			dali1_msg(msg->message, msg->p1, msg->p2);
		} break;

		case DALI1_RESPONSE_RECEIVED: {
			DALI_REQUEST *req = (DALI_REQUEST*) msg->p1;
			print("%s RESPONSE FROM: addr = %d, status = %d (0x%02X)\r\n", "DALI1", (req->txbuf[0] & 0x7f)>> 1, req->rxbuf[0], req->rxbuf[0]);
		} break;

		case DALI1_RESPONSE_NOT_RECEIVED: {
			DALI_REQUEST *req = (DALI_REQUEST*) msg->p1;
			print("%s NO RESPONSE FROM: addr = %d\r\n", "DALI1", (req->txbuf[0] & 0x7f)>> 1);
		} break;
	#endif

	#ifdef DALI2_MODE
		case DALI2_TX_DELAY_COMPLETE: 
		case DALI2_TX_COMPLETE: 
		case DALI2_RX_COMPLETE: {
			dali2_msg(msg->message, msg->p1, msg->p2);
		} break;

		case DALI2_RESPONSE_RECEIVED: {
			DALI_REQUEST *req = (DALI_REQUEST*) msg->p1;
			print("%s RESPONSE FROM: addr = %d, status = %d (0x%02X)\r\n", "DALI2", (req->txbuf[0] & 0x7f)>> 1, req->rxbuf[0], req->rxbuf[0]);
		} break;

		case DALI2_RESPONSE_NOT_RECEIVED: {
			DALI_REQUEST *req = (DALI_REQUEST*) msg->p1;
			print("%s NO RESPONSE FROM: addr = %d\r\n", "DALI2", (req->txbuf[0] & 0x7f)>> 1);
		} break;
	#endif

		case LCD_TOUCH: {
			if(info_timer_ms<skip_touches_time_128us) {
				skip_touch_count ++; 
				break;
			}

			int touch_X, touch_Y, touch_P;
			touch_X = (msg->p1 >> 16);
			touch_Y = (msg->p1 & 0xFFFF);
			touch_P = msg->p2;

			TOUCH2SCREEN_PARAM param = {
				.KX1 = config_active.lcd.KX1,
				.KX2 = config_active.lcd.KX2,
				.KX3 = config_active.lcd.KX3,
				.KY1 = config_active.lcd.KY1,
                        	.KY2 = config_active.lcd.KY2,
                        	.KY3 = config_active.lcd.KY3,
				.screen_width = LCD_WIDTH,
				.screen_height = LCD_HEIGHT,
				.touch_x = touch_X,
				.touch_y = touch_Y,
			};
        		TOUCH_INFO.pt=wnd_touch2screen_pt(&param);

			wnd_proc_call(SCREEN, WM_HIT_TEST, (int) &TOUCH_INFO, 0);

			//print("SCREEN_TOUCH_PRESSED X=%d, Y=%d, view=%d\r\n", TOUCH_INFO.pt.x, TOUCH_INFO.pt.y, (TOUCH_INFO.view ? TOUCH_INFO.view->tag : -1));

			if(TOUCH_INFO.view) {
				wnd_proc_call(TOUCH_INFO.view, WM_TOUCH_PRESSED, TOUCH_INFO.pt.x, TOUCH_INFO.pt.y);
			}
		} break;

		case LCD_TOUCH_MOVED: {
			if(info_timer_ms<skip_touches_time_128us) {
                                skip_touch_count ++;
                                break;
                        }

			int touch_X, touch_Y, touch_P;
                        touch_X = (msg->p1 >> 16);
                        touch_Y = (msg->p1 & 0xFFFF);
                        touch_P = msg->p2;

			HIT_TEST_INFO old_touch = TOUCH_INFO;


			TOUCH2SCREEN_PARAM param = {
                                .KX1 = config_active.lcd.KX1,
                                .KX2 = config_active.lcd.KX2,
                                .KX3 = config_active.lcd.KX3,
                                .KY1 = config_active.lcd.KY1,
                                .KY2 = config_active.lcd.KY2,
                                .KY3 = config_active.lcd.KY3,
                                .screen_width = LCD_WIDTH,
                                .screen_height = LCD_HEIGHT,
                                .touch_x = touch_X,
                                .touch_y = touch_Y,
                        };
                        TOUCH_INFO.pt=wnd_touch2screen_pt(&param);

			wnd_proc_call(SCREEN, WM_HIT_TEST, (int) &TOUCH_INFO, 0);

			if(TOUCH_INFO.view != old_touch.view) {
				if(old_touch.view) {
					//print("SCREEN_TOUCH_CANCELLED (MOOVED) view=%d, flags=%d\r\n", old_touch.view->tag, old_touch.view->flags);
					wnd_proc_call(old_touch.view, WM_TOUCH_CANCELLED, 0, 0);
					//print("SCREEN_TOUCH_CANCELLED ::2\r\n");
				}

				if(TOUCH_INFO.view) {
					//print("SCREEN_TOUCH_PRESSED (MOOVED) X=%d, Y=%d, view=%d\r\n", TOUCH_INFO.pt.x, TOUCH_INFO.pt.y, TOUCH_INFO.view->tag);
                        		wnd_proc_call(TOUCH_INFO.view, WM_TOUCH_PRESSED, TOUCH_INFO.pt.x, TOUCH_INFO.pt.y);
				}		
			}

		} break;


		case LCD_UNTOUCH: {
			if(info_timer_ms<skip_touches_time_128us) {
                                skip_touch_count ++;
                                break;
                        }

			int touch_X, touch_Y, touch_P;
			touch_X = (msg->p1 >> 16);
			touch_Y = (msg->p1 & 0xFFFF);
			touch_P = msg->p2;

			if(TOUCH_INFO.view) {
                               	//print("SCREEN_TOUCH_RELEASED view: %d\r\n", TOUCH_INFO.view->tag);
                               	wnd_proc_call(TOUCH_INFO.view, WM_TOUCH_RELEASED, TOUCH_INFO.pt.x, TOUCH_INFO.pt.y);
				skip_few_touches();
			}

			memset(&TOUCH_INFO, 0, sizeof(TOUCH_INFO));
		} break;


		#ifdef LCD_SPI
		case LCD_DMA_TC: {
			//print("LCD_DMA_TC\r\n");
			uint32_t val;

			// We need to poll ADC between LCD frame updates to reduce RF noise
			 if(config_active.adc.enabled && config_active.adc.mode == ADC_MODE_INTERFRAME)
                        	adc_poll(ADC_MODE_SINGLE_TIMEOUT);

			if(!config_active.lcd.enabled)
				break;
			
			if(lcd_irq_requested) {
				int touch_X, touch_Y, touch_P;
				lcd_read_touch(&touch_X, &touch_Y, &touch_P);

				if(event_logging> 1)
					print("LCD_DMA_TC LCD_TOUCH x=%d, y=%d, p=%d\r\n", lcd_touch_x, lcd_touch_y, touch_P);

				if(touch_P != 0x0007FFF) {
					if(lcd_touched == 0) {
						PostMessage(LCD_TOUCH, 1, (touch_X << 16) | touch_Y, touch_P);
						lcd_touched = 1;

						lcd_touch_x = touch_X;
						lcd_touch_y = touch_Y;

						//print("LCD_DMA_TC LCD_TOUCH x=%d, y=%d\r\n", lcd_touch_x, lcd_touch_y);

					} else {
						PostMessage(LCD_TOUCH_MOVED, 1, (touch_X << 16) | touch_Y, touch_P);

						lcd_touch_x = 0.1*lcd_touch_x + 0.9*touch_X;
						lcd_touch_y = 0.1*lcd_touch_y + 0.9*touch_Y;

						//print("LCD_DMA_TC LCD_TOUCH_MOVED x=%d, y=%d\r\n", lcd_touch_x, lcd_touch_y);

					}
				} else {
					if(lcd_touched) {
						//print("LCD_DMA_TC LCD_UNTOUCH x=%d, y=%d\r\n", lcd_touch_x, lcd_touch_y);
						PostMessage(LCD_UNTOUCH, 1, (lcd_touch_x << 16) | lcd_touch_y, touch_P);
                                        	lcd_touched = 0;
                                	}
				}
				lcd_irq_requested = 0;
			}


			// Check LCD availability

			if(info_timer_ms - lcd_check_timestamp >= 1000) {//every second
                        	lcd_check_timestamp = info_timer_ms;

                                        int old = lcd_error;
                                        lcd_error = lcd_check();

                                        if(lcd_error != old) {

                                                if(lcd_error == 1) {
                                                        print("LCD is not connected!\r\n");
                                                } else {
                                                        print("LCD is malfunctioning, calling lcd_init()\r\n");
                                                        lcd_init();
                                                        lcd_error = lcd_check();
                                                }
                                        }

			}



			lcd_read_mem32(REG_INT_FLAGS, &val); // read INT flags to clear pending request, otherwise we won't get more IRQs

			//print("LCD_DMA_TC:\r\n");
			Fbinfo *fb=fbqueue_dequeue();
			if(fb && fb->img_index == 3) { 
                		lcd_dl(CLEAR_COLOR_RGB(0, 255, 0)); // clear SCREEN
                		lcd_dl(CLEAR(1, 1, 1)); // clear SCREEN

                		for(int i=0; i<4; i++) {
                        		lcd_dl(BITMAP_SOURCE(RAM_G+FRAMEBUFFER_SIZE_BYTES*i));
                        		lcd_dl(BITMAP_LAYOUT(RGB332, FRAMEBUFFER_WIDTH*sizeof(*fb->framebuffer), FRAMEBUFFER_HEIGHT));
                        		lcd_dl(BITMAP_SIZE(NEAREST, BORDER, BORDER, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT));
                        		lcd_dl(BEGIN(BITMAPS));

                        		switch(i) {
                        		case 0: lcd_dl(VERTEX2II(0, 0, 0, 0)); break;
                        		case 1: lcd_dl(VERTEX2II(FRAMEBUFFER_WIDTH, 0, 0, 0)); break;
                        		case 2: lcd_dl(VERTEX2II(0, FRAMEBUFFER_HEIGHT, 0, 0)); break;
                        		case 3: lcd_dl(VERTEX2II(FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0, 0)); break;
                        		}

                        		lcd_dl(END());
                		}

                		lcd_dl(DISPLAY());
                		lcd_dl_flush();

                		lcd_update_counter++;
			}

			fb=fbqueue_get_head();
			if(fb) lcd_write_memory_dma_irq(RAM_G+FRAMEBUFFER_SIZE_BYTES*fb->img_index, (uint8_t*)fb->framebuffer, FRAMEBUFFER_SIZE_BYTES);
			draw_fb();	
		} break;
		#endif // LCD_SPI


	#ifdef USART_EXT1 
		case MODBUS1_READ_HOLD_REGS: {

			MODBUS_RESPONSE *resp = (MODBUS_RESPONSE *)msg->p1;

			print("Modbus1 %s request: reg = %d, qty = %d\r\n", "READ HOLD REGS", MODBUS_GET_REQ_REG(resp), MODBUS_GET_REQ_QTY(resp));

			MODBUS_RESPONSE_START(resp, MODBUS_GET_REQ_FUNC(resp));


			for(int reg = MODBUS_GET_REQ_REG(resp); reg < MODBUS_GET_REQ_REG(resp) + MODBUS_GET_REQ_QTY(resp); reg++) {
				switch(MODBUS_GET_REQ_REG(resp)) {

					case REG_READ_SCRATCH: {
						MODBUS_RESPONSE_ADD_WORD(resp, scratch_register);
					} break;

					case REG_READ_DATE: {
						RTC_DateTypeDef d;
						RTC_GetDate(RTC_Format_BIN, &d);
						MODBUS_RESPONSE_ADD_DWORD(resp, *(uint32_t*)&d);
					} break;

					case REG_READ_TIME: {
						RTC_TimeTypeDef t;
						RTC_GetTime(RTC_Format_BIN, &t);
						MODBUS_RESPONSE_ADD_DWORD(resp, *(uint32_t*)&t);
					} break;

					case REG_READ_TEMPR: {
						float tempr = 0;

						if(config_active.adc.enabled && config_active.adc.mode == ADC_MODE_SINGLE)
                        				adc_poll(ADC_MODE_SINGLE_TIMEOUT);
	
						tempr = ADC_DATA.adc_avg[ADC_NUM_OF_CHANNELS - 1];
	
						MODBUS_RESPONSE_ADD_WORD(resp, (int) tempr);
					} break;


					case REG_ADC0_OFFSET ... REG_ADC12_RMS: {
						uint32_t adc_chan = (reg - REG_ADC0_OFFSET) / 4;

						if(adc_chan >= ADC_NUM_OF_CHANNELS) {
							MODBUS_RESPONSE_ERROR(resp, FUNC_READ_HOLD_REGS, ERR_ADDR_NOT_AVAILABLE);
							goto modbus1_end;
						}

						int adc_feature = (reg - REG_ADC0_OFFSET) % 4;

						switch(adc_feature) {
							case 0: // ADC Offset
								MODBUS_RESPONSE_ADD_DWORD(resp, *(int*)&config_active.adc.offset[adc_chan]);
								break;
							case 1: // ADC Coefficient 
								MODBUS_RESPONSE_ADD_DWORD(resp, *(int*)&config_active.adc.coeff[adc_chan]);
								break;
							case 2: // ADC AVG  
								MODBUS_RESPONSE_ADD_DWORD(resp, *(int*)&ADC_DATA.adc_avg[adc_chan]);
								break;
							case 3: // ADC RMS  
								MODBUS_RESPONSE_ADD_DWORD(resp, *(int*)&ADC_DATA.adc_rms[adc_chan]);
								break;
						}
					} break;

					default: {
						MODBUS_RESPONSE_ERROR(resp, FUNC_READ_HOLD_REGS, ERR_ADDR_NOT_AVAILABLE);
						goto modbus1_end;
					} break;
				}
			}

			modbus1_end:

			modbus_submit_response_ext1(resp);

		} break;



		case MODBUS1_WRITE_HOLD_REGS: {

			MODBUS_RESPONSE *resp = (MODBUS_RESPONSE *)msg->p1;

			print("Modbus1 %s request: reg = %d, qty = %d\r\n", "WRITE HOLD REGS", MODBUS_GET_REQ_REG(resp), MODBUS_GET_REQ_QTY(resp));

			MODBUS_WRITE_RESPONSE_START(resp, MODBUS_GET_REQ_FUNC(resp));

			switch(MODBUS_GET_REQ_REG(resp)) {

				case REG_WRITE_SCRATCH: {
					scratch_register = MODBUS_GET_REQ_REG_VAL(resp, 0);
					MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
				} break;

				case REG_WRITE_DATE: {
					RTC_DateTypeDef *d = (RTC_DateTypeDef*) MODBUS_GET_REQ_REG_BUF(resp, 0);
					PWR_BackupAccessCmd(ENABLE);
					RTC_WriteProtectionCmd(DISABLE);
					RTC_SetDate(RTC_Format_BIN, d);
					RTC_WriteProtectionCmd(ENABLE);
					PWR_BackupAccessCmd(DISABLE);
					MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
				} break;

				case REG_WRITE_TIME: {
					RTC_TimeTypeDef *t = (RTC_TimeTypeDef*) MODBUS_GET_REQ_REG_BUF(resp, 0);
					PWR_BackupAccessCmd(ENABLE);
					RTC_WriteProtectionCmd(DISABLE);
					RTC_SetTime(RTC_Format_BIN, t);
					RTC_WriteProtectionCmd(ENABLE);
					PWR_BackupAccessCmd(DISABLE);
					MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
				} break;

				case REG_ADDRESS: {
					uint32_t addr = MODBUS_GET_REQ_REG_VAL(resp, 0);
					if(addr > 0 && addr < 256) {
						config_active.ext1_port_modbus_addr = addr;
						save_config();
						load_config();
						MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
					} else {
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_BAUD: {
					uint32_t baud = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);
					int i;
					for(i = 0; i < BAUD_RATES; i++) 
						if(baud_rates[i] == baud)
							break;
					if(i < BAUD_RATES) {
						config_active.ext1_port_baud_rate = i;
						save_config();
						load_config();
						USART2_init();
						MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
					} else {
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_STOP: {
					uint32_t stop = MODBUS_GET_REQ_REG_VAL(resp, 0);
					if(stop < STOP_BITS) {
						config_active.ext1_port_stop_bits = stop;
						save_config();
						load_config();
						USART2_init();
						MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
					} else {
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_PARITY: {
					uint32_t parity = MODBUS_GET_REQ_REG_VAL(resp, 0);
					if(parity < 3) {
						config_active.ext1_port_parity  = parity;
						save_config();
						load_config();
						USART2_init();
						MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
					} else {
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
						goto resp_submit;
					}
				} break;

				case REG_SET_DEFAULT_APP: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						char *file_name = MODBUS_GET_REQ_REG_BUF(resp, 4);
						int file_name_len = strnlen(file_name,MODBUS_GET_REQ_REG_BUF_SIZE(resp) - 4);
						file_name[file_name_len] = 0x0;

						print("Modbus set default app: %s\r\n", file_name);

						strncpy(config_active.default_application_file_name, file_name, sizeof(config_active.default_application_file_name));
						save_config();
						load_config();
						MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS FORMAT", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_EXEC_APP: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						char *file_name = MODBUS_GET_REQ_REG_BUF(resp, 4);
						int file_name_len = strnlen(file_name,MODBUS_GET_REQ_REG_BUF_SIZE(resp) - 4);
						file_name[file_name_len] = 0x0;

						print("Modbus request to execute: %s\r\n", file_name);

						TFS_HEADER* tfs = tfs_find(file_name);

						if(tfs) {
							if(exec_tfs(tfs, NULL, 0) < 0) {
								MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
							} else {
								MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
							}
						} else {
							MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
						}
					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS FORMAT", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_TFS_FORMAT: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						stop_application((void*)application_text_addr);

						main_queue_enabled = 0;
						debug_mode = DEBUG_TO_SWD;

						tfs_format();

						debug_mode = DEBUG_TO_USART1_AND_SWD;
						main_queue_enabled = 1;
						MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS FORMAT", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_TFS_DELETE: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						char *file_name = MODBUS_GET_REQ_REG_BUF(resp, 4);
						int file_name_len = strnlen(file_name,MODBUS_GET_REQ_REG_BUF_SIZE(resp) - 4);
						file_name[file_name_len] = 0x0;
						TFS_HEADER* tfs = tfs_find(file_name);
						if(tfs) {
							tfs_delete(tfs);
							MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
						} else {
							MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
						}
					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS DELETE", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_TFS_FIND: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						char *file_name = MODBUS_GET_REQ_REG_BUF(resp, 4);
						int file_name_len = strnlen(file_name,MODBUS_GET_REQ_REG_BUF_SIZE(resp) - 4);
						file_name[file_name_len] = 0x0;
						
						TFS_HEADER *tfs = tfs_find(file_name);

						if(tfs) {
							MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)REG_TFS_CREATE); // reg
							MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)0x0001); // qty
							MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)0x0000); // nothing (for perl Device::Modbus)
							MODBUS_WRITE_RESPONSE_ADD_DWORD(resp, (uint32_t)tfs); // tfs
							MODBUS_WRITE_RESPONSE_ADD_DWORD(resp, (uint32_t)tfs->size); //file size 
						} else {
							MODBUS_RESPONSE_ERROR(resp, FUNC_READ_HOLD_REGS, ERR_WRONG_ARGS);
						}
					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS FORMAT", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_TFS_READ_BLOCK: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						TFS_HEADER *tfs = (TFS_HEADER*)((MODBUS_GET_REQ_REG_VAL(resp, 2) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 3));
						uint32_t offset = (MODBUS_GET_REQ_REG_VAL(resp, 4) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 5);
						uint32_t size = (MODBUS_GET_REQ_REG_VAL(resp, 6) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 7);

						size = MIN(size, tfs->size - offset);

						if(size < 0 || size > 240) {
							MODBUS_RESPONSE_ERROR(resp, FUNC_READ_HOLD_REGS, ERR_WRONG_ARGS);
							break;
						}

						if(tfs_read_block(tfs, offset, MODBUS_WRITE_RESPONSE_BUF(resp), size) >= 0) {
							MODBUS_WRITE_RESPONSE_BUF_SIZE(resp, size);
						} else {
							MODBUS_RESPONSE_ERROR(resp, FUNC_READ_HOLD_REGS, ERR_WRONG_ARGS);
						}
					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS FORMAT", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;


				case REG_TFS_CREATE: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						uint32_t size = (MODBUS_GET_REQ_REG_VAL(resp, 2) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 3);
						char *file_name = MODBUS_GET_REQ_REG_BUF(resp, 8);
						int file_name_len = strnlen(file_name,MODBUS_GET_REQ_REG_BUF_SIZE(resp) - 8);
						file_name[file_name_len] = 0x0;
						TFS_HEADER* tfs = tfs_create_file(file_name, file_name_len, size);
						if(tfs) {
							MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)REG_TFS_CREATE); // reg
							MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)0x0001); // qty
							MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)0x0000); // nothing (for perl Device::Modbus)
							MODBUS_WRITE_RESPONSE_ADD_DWORD(resp, (uint32_t)tfs); // tfs
						} else {
							MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
						} 
					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS CREATE", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_TFS_WRITE_BLOCK: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						uint32_t tfs = (MODBUS_GET_REQ_REG_VAL(resp, 2) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 3);
						uint32_t offset = (MODBUS_GET_REQ_REG_VAL(resp, 4) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 5);
						char *data = MODBUS_GET_REQ_REG_BUF(resp, 12);
						int32_t size = MODBUS_GET_REQ_REG_BUF_SIZE(resp) - 12;

						size = tfs_write_block((TFS_HEADER*)tfs, data, size, offset);

						if(size >= 0) {
							MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)REG_TFS_WRITE_BLOCK); // reg
							MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)0x0001); // qty
							MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)0x0000); // nothing (for perl Device::Modbus)
							MODBUS_WRITE_RESPONSE_ADD_DWORD(resp, (uint32_t)size); // tfs
						} else {
							MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
						}
					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS WRITE BLOCK", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}

				} break;

				case REG_TFS_CLOSE: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						TFS_HEADER *tfs = (TFS_HEADER*) ((MODBUS_GET_REQ_REG_VAL(resp, 2) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 3));
						int32_t size = tfs_close(tfs);
						if(size >= 0) {
							MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
						} else {
							MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
						}
					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS CLOSE", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_TFS_FREE_SPACE: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)REG_TFS_WRITE_BLOCK); // reg
						MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)0x0001); // qty
						MODBUS_WRITE_RESPONSE_ADD_WORD(resp, (uint32_t)0x0000); // nothing (for perl Device::Modbus)
						MODBUS_WRITE_RESPONSE_ADD_DWORD(resp, (uint32_t)tfs_get_free_space()); // 

					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS CLOSE", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_PLAY_FILE: {
					uint32_t key = (MODBUS_GET_REQ_REG_VAL(resp, 0) << 16) | MODBUS_GET_REQ_REG_VAL(resp, 1);

					if(key == MODBUS_SECURITY_KEY) {
						char *file_name = MODBUS_GET_REQ_REG_BUF(resp, 4);
						int file_name_len = strnlen(file_name,MODBUS_GET_REQ_REG_BUF_SIZE(resp) - 4);
						file_name[file_name_len] = 0x0;
						
						TFS_HEADER *tfs = tfs_find(file_name);

						if(tfs) {
							MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
							play_start(((char*)tfs) + sizeof(TFS_HEADER), tfs->size);
						} else {
							MODBUS_RESPONSE_ERROR(resp, FUNC_READ_HOLD_REGS, ERR_WRONG_ARGS);
						}
					} else {
						print("Modbus %s request declined, wrong key: 0x%08X\r\n", "TFS FORMAT", key);
						MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_WRONG_ARGS);
					}
				} break;

				case REG_ADC0_OFFSET ... REG_ADC12_RMS: {
					uint32_t adc_chan = (MODBUS_GET_REQ_REG(resp) - REG_ADC0_OFFSET) / 4;

					if(adc_chan >= ADC_NUM_OF_CHANNELS) {
						MODBUS_RESPONSE_ERROR(resp, FUNC_READ_HOLD_REGS, ERR_ADDR_NOT_AVAILABLE);
						break;
					}

					int adc_feature = (MODBUS_GET_REQ_REG(resp) - REG_ADC0_OFFSET) % 4;

					char *resp_buf = MODBUS_GET_REQ_REG_BUF(resp, 0);

					switch(adc_feature) {
						case 0: // ADC Offset
							*(uint32_t*)resp_buf = swap32(*(uint32_t*)resp_buf); 
							config_active.adc.offset[adc_chan] = *(float*) resp_buf;
							save_config();
							load_config();
							MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
							break;
						case 1: // ADC Coefficient 
							*(uint32_t*)resp_buf = swap32(*(uint32_t*)resp_buf); 
							config_active.adc.coeff[adc_chan] = *(float*) resp_buf;
							save_config();
							load_config();
							MODBUS_RESPONSE_OK(resp, MODBUS_GET_REQ_FUNC(resp));
							break;
						default:
							MODBUS_RESPONSE_ERROR(resp, FUNC_READ_HOLD_REGS, ERR_ADDR_NOT_AVAILABLE);
							break;
					}
				} break;

				default: {
					MODBUS_RESPONSE_ERROR(resp, FUNC_WRITE_HOLD_REGS, ERR_ADDR_NOT_AVAILABLE);
				} break;
			}


			resp_submit:

			modbus_submit_response_ext1(resp);

		} break;

	#endif // USART_EXT1 

	#ifdef USART_EXT2 
		case MODBUS2_READ_HOLD_REGS: {

			MODBUS_RESPONSE *resp = (MODBUS_RESPONSE *)msg->p1;

			print("Modbus2 %s request: reg = %d, qty = %d\r\n", "READ HOLD REGS", MODBUS_GET_REQ_REG(resp), MODBUS_GET_REQ_QTY(resp));

			MODBUS_RESPONSE_START(resp, MODBUS_GET_REQ_FUNC(resp));


			for(int reg = MODBUS_GET_REQ_REG(resp); reg < MODBUS_GET_REQ_REG(resp) + MODBUS_GET_REQ_QTY(resp); reg++) {
				switch(MODBUS_GET_REQ_REG(resp)) {

					case REG_READ_SCRATCH: {
						MODBUS_RESPONSE_ADD_WORD(resp, scratch_register);
					} break;

					case REG_READ_DATE: {
						RTC_DateTypeDef d;
						RTC_GetDate(RTC_Format_BIN, &d);
						MODBUS_RESPONSE_ADD_DWORD(resp, *(uint32_t*)&d);
					} break;

					case REG_READ_TIME: {
						RTC_TimeTypeDef t;
						RTC_GetTime(RTC_Format_BIN, &t);
						MODBUS_RESPONSE_ADD_DWORD(resp, *(uint32_t*)&t);
					} break;

					case REG_READ_TEMPR: {
						float tempr = 0;

						if(config_active.adc.enabled && config_active.adc.mode == ADC_MODE_SINGLE)
                        				adc_poll(ADC_MODE_SINGLE_TIMEOUT);
	
						tempr = ADC_DATA.adc_avg[ADC_NUM_OF_CHANNELS - 1];
	
						MODBUS_RESPONSE_ADD_WORD(resp, (int) tempr);
					} break;


					case REG_ADC0_OFFSET ... REG_ADC12_RMS: {
						uint32_t adc_chan = (reg - REG_ADC0_OFFSET) / 4;

						if(adc_chan >= ADC_NUM_OF_CHANNELS) {
							MODBUS_RESPONSE_ERROR(resp, FUNC_READ_HOLD_REGS, ERR_ADDR_NOT_AVAILABLE);
							goto modbus2_end;
						}

						int adc_feature = (reg - REG_ADC0_OFFSET) % 4;

						switch(adc_feature) {
							case 0: // ADC Offset
								MODBUS_RESPONSE_ADD_DWORD(resp, *(int*)&config_active.adc.offset[adc_chan]);
								break;
							case 1: // ADC Coefficient 
								MODBUS_RESPONSE_ADD_DWORD(resp, *(int*)&config_active.adc.coeff[adc_chan]);
								break;
							case 2: // ADC AVG  
								MODBUS_RESPONSE_ADD_DWORD(resp, *(int*)&ADC_DATA.adc_avg[adc_chan]);
								break;
							case 3: // ADC RMS  
								MODBUS_RESPONSE_ADD_DWORD(resp, *(int*)&ADC_DATA.adc_rms[adc_chan]);
								break;
						}
					} break;

					default: {
						MODBUS_RESPONSE_ERROR(resp, FUNC_READ_HOLD_REGS, ERR_ADDR_NOT_AVAILABLE);
						goto modbus2_end;
					} break;
				}
			}

			modbus2_end:

			modbus_submit_response_ext2(resp);

		} break;

	#endif // USART_EXT2 


                case AUDIO_PLAY_STOP: {
			if(event_logging)
                        	print("AUDIO_PLAY_STOP: play_buf = %p\r\n", msg->p1);
                } break;

                case AUDIO_PLAY_DMA_TC: {
                        if(event_logging)
                        	print("AUDIO_PLAY_DMA_TC play_buf_len=%d\r\n", play_buf_len);

			play_tc();
                } break;

                case AUDIO_PLAY_DMA_HT: {
                        if(event_logging)
				print("AUDIO_PLAY_DMA_HT play_buf_len=%d\r\n", play_buf_len);

			play_ht();
                } break;



	}//end switch;

	CPU_STATE old_state = saved_cpu_state;

	if(SaveCPUState(&saved_cpu_state)==0) {
		if(application_process_msg_addr)
			application_process_msg_addr(msg);
	} else {
		stop_application((void*)application_text_addr);
		printf("\r\nException while application MSG queue processing, application stopped!\r\n");
	}

	saved_cpu_state = old_state;

	//print("process_message() msg = %d end\r\n", msg->message);
}

void skip_few_touches() {
	skip_touches_time_128us = info_timer_ms + 200;
}

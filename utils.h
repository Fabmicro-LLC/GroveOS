/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef ___UTILS_H___
#define ___UTILS_H___

#include <stm32f4xx_usart.h>
#include <string.h>
#include <math.h>
#include "usart_dma.h"

#ifndef NULL
#define NULL 0
#endif

#ifndef PI
#define PI 3.14159265358979f
#endif


#define htons(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))
#define ntohs(n) (((((unsigned short)(n) & 0xFF)) << 8) | (((unsigned short)(n) & 0xFF00) >> 8))

#define SIZE(x) (sizeof(x)/sizeof(x[0]))

#define DEG2RAD(x) ((x)*PI/180.0)
#define RAD2DEG(x) ((x)*180.0/PI)

#define	DEBUG_TO_NONE		0
#define	DEBUG_TO_USART1_AND_SWD	1
#define	DEBUG_TO_SWD		2
#define	DEBUG_TO_USART1		3


extern int debug_mode;
extern char timestamp_string[];

//void print(const char *format, ...);
int swd_print(const char *format, ...);
int nmea_print(const char *format, ...);
void nmea_add_crc(unsigned char *buf);
void print_time(void);
char* gettimestamp(void);
int print(const char *format, ...);

void USART_ext_init();
void RTC_init();

void Delay(__IO uint32_t nTime); // millisecs
void DelayLoop(__IO uint32_t nTime);
void DelayLoopMicro(__IO uint32_t nTime);


void gpio_init(uint32_t RCC_AHB1Periph, unsigned short GPIO_SPEED, void* GPIOx, unsigned short GPIO_Pin, unsigned short InOut, unsigned short PushPull, unsigned short PullUpDown);
void gpio_irq(unsigned short PortSrc, unsigned short PortPin, unsigned short Line, unsigned short IrqChan, unsigned short RaiseFall);
void gpio_irq_disable(unsigned short PortSrc, unsigned short PortPin, unsigned short Line, unsigned short IrqChan);
void gpio_set(GPIO_TypeDef* GPIOx, unsigned short GPIO_Pin, int val); // val = 0, 1 or 2 for toggle
void gpio_write(GPIO_TypeDef* GPIOx, int val);
void gpio_pinconfig(void* GPIOx, unsigned short GPIO_Pin, unsigned short afconfig);

void sos(const char* msg);

int utf8_to_utf16 (unsigned short *u16string, int *u16string_size, unsigned char* utf8);

double xround(double x, unsigned int precision);
double degmod180(double deg);

char* strpbrkn(char* string_begin, char* accept, char* string_end);
char* my_index(char* str, char c, int len);
char * strnstrn(char *s, const char *find, int slen);
char * strnarg(char *s, int argn, int slen);

int32_t read_from_backup_rtc( uint32_t *data, uint16_t bytes, uint16_t offset );
int32_t write_to_backup_rtc( uint32_t *data, uint16_t bytes, uint16_t offset );
int32_t read_from_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset );
int32_t write_to_backup_sram( uint8_t *data, uint16_t bytes, uint16_t offset );
void* get_backup_sram_ptr(uint16_t offset );

void memset16(unsigned short* dst, unsigned short val, int count);

uint16_t modbus_crc16(uint8_t *buf, uint16_t len);
void modbus_add_crc(uint8_t *buf, uint16_t len);
void print_hex(uint8_t *buf, uint16_t len);

#endif

/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <stdint.h>

#define LOGGER_START_SECTOR	17
#define LOGGER_END_SECTOR	23
#define	LOGGER_MAX_BYTES	(12*4)

#ifndef __STM32F4xx_RTC_H
#define __STM32F4xx_RTC_H

typedef struct {
        uint8_t RTC_Hours;
        uint8_t RTC_Minutes;
        uint8_t RTC_Seconds;
        uint8_t RTC_H12;
} RTC_TimeTypeDef;

typedef struct {
        uint8_t RTC_WeekDay;
        uint8_t RTC_Month;
        uint8_t RTC_Date;
        uint8_t RTC_Year;
} RTC_DateTypeDef;

#endif // __STM32F4xx_RTC_H


#pragma pack(1)
struct LOGGER_ITEM {
	uint8_t data[LOGGER_MAX_BYTES];
	RTC_DateTypeDef rtc_date;
	RTC_TimeTypeDef rtc_time;
	unsigned int serial;
	short	res1;
	unsigned short	crc16;
};
#pragma pack()

#ifndef SVC_CLIENT

extern struct LOGGER_ITEM *current_logger_item;
extern unsigned int current_logger_sector; 
extern unsigned int current_logger_serial; 

void logger_init(void);
int logger_get_sector(void *addr);
int logger_erase_current_sector(void);
int logger_write_data(uint8_t *data, int num);
struct LOGGER_ITEM* logger_get_previous_item(struct LOGGER_ITEM* item);

#else

int svc_logger_write_data(uint8_t* data, unsigned int data_len);
int svc_logger_erase_current_sector();
struct LOGGER_ITEM* svc_logger_get_previous_item(struct LOGGER_ITEM* item);
struct LOGGER_ITEM* svc_logger_get_current_item(struct LOGGER_ITEM* item);

#endif //SVC_CLIENT
#endif //_LOGGER_H_

#ifdef SVC_CLIENT_IMPL

#ifndef _LOGGER_H_IMPL_
#define _LOGGER_H_IMPL_

#include "svc.h"

__attribute__ ((noinline)) int svc_logger_write_data(uint8_t* data, unsigned int data_len) {
        svc(SVC_LOGGER_WRITE_DATA);
}

__attribute__ ((noinline)) int svc_logger_erase_current_sector() {
        svc(SVC_LOGGER_ERASE_CURRENT_SECTOR);
}

__attribute__ ((noinline)) struct LOGGER_ITEM* svc_logger_get_previous_item(struct LOGGER_ITEM* item) {
        svc(SVC_LOGGER_GET_PREVIOUS_ITEM);
}

__attribute__ ((noinline)) struct LOGGER_ITEM* svc_logger_get_current_item(struct LOGGER_ITEM* item) {
        svc(SVC_LOGGER_GET_CURRENT_ITEM);
}

#endif //_LOGGER_H_IMPL_
#endif //SVC_CLIENT_IMPL

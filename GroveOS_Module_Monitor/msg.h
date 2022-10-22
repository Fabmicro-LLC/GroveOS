/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#ifndef ___MAIN_QUEUE_H___
#define ___MAIN_QUEUE_H___

#include "stm32f4xx.h"

extern int main_queue_enabled;

typedef struct {
	int message;
	int p1;
	int p2;
	void* hwnd;
} MSG;

#define MSGSET_SIZE 		100
#define MAIN_QUEUE_MAX_SIZE 	100

unsigned int main_queue_changed;
unsigned int main_queue_wait_counter;

unsigned int error_main_queue_isfull;

void main_queue_init();
int main_queue_len();

int PostMessage(int message,  int unique, int p1, int p2);
int PostMessageIRQ(int message,  int unique, int p1, int p2);
int GetMessage(MSG* msg);

#define AUDIO_RECORD_DMA_HT			1
#define AUDIO_RECORD_DMA_TC			2
#define AUDIO_PLAY_DMA_TC			3
#define AUDIO_PLAY_DMA_HT       		4

#define TEST_IRQ				5	
#define INFO_TIMER_INT				6	
#define LCD_DMA_TC				7	
#define ENCODERS_TIMER_INT			8	
#define LCD_INT_ACTIVE				9	
#define LCD_INT_RELEASED			10	
#define	AUDIO_PLAY_STOP				11	

#define	LEFT_ENCODER_READY			12	
#define	LEFT_ENCODER_ERROR			13	
#define RIGHT_ENCODER_READY			14	
#define RIGHT_ENCODER_ERROR			15	
#define WHEEL_ENCODER_READY			16	
#define WHEEL_ENCODER_ERROR			17

#define	LEFT_STEPPER_DONE			18
#define RIGHT_STEPPER_DONE			19

#define	ADC1_EOC				20

//RITT
#define	ADC2_EOC				21	

// Left and right nozzle limiters
#define LEFT_NOZZLE_LEFT_LIMIT_PRESSED		22	
#define LEFT_NOZZLE_LEFT_LIMIT_RELEASED		23	
#define LEFT_NOZZLE_RIGHT_LIMIT_PRESSED		24	
#define LEFT_NOZZLE_RIGHT_LIMIT_RELEASED	25	
#define RIGHT_NOZZLE_LEFT_LIMIT_PRESSED		26	
#define RIGHT_NOZZLE_LEFT_LIMIT_RELEASED	27	
#define RIGHT_NOZZLE_RIGHT_LIMIT_PRESSED	28	
#define RIGHT_NOZZLE_RIGHT_LIMIT_RELEASED	29	

// Controls
#define	KEY_UP_IRQ				31	
#define	KEY_DOWN_IRQ				32	
#define	KEY_LEFT_IRQ				33	
#define	KEY_RIGHT_IRQ				34	

//IN lines
#define	IN1_IRQ					35	
#define	IN2_IRQ					36	
#define	IN3_IRQ					37	
#define	IN4_IRQ					38	

// LCD touch
#define	LCD_TOUCH				40
#define	LCD_UNTOUCH				41
#define LCD_TOUCH_MOVED				42


// ACD sensor array
#define	ADC_DMA_TC				51

//APS protocol
#define	APS_RX_CMD				61

//
#define	EXCEPTION_APPLICATION			65
#define	EXCEPTION_SUPERVISOR			66	

//FLASH memory
#define FLASH_ERASE_OK                          70
#define FLASH_ERASE_ERROR                       71

#define	RTC_ALARM				80
#define	USART1_RX_DATA				81

#endif

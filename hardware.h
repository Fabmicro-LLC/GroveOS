/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#ifdef PRODUCT_GROVEX
#include "hardware_grovex.h"
#endif

#ifdef PRODUCT_APS
#include "hardware_aps.h"
#endif

#ifdef PRODUCT_SPUTNIK
#include "hardware_sputnik.h"
#endif

#ifdef PRODUCT_SMARTPWR6
#include "hardware_smartpwr6.h"
#endif


#define	SRAM_BACKUP_FILENAME	"backup_mem.bin"

#define	GPIO_SPEED		GPIO_Speed_50MHz

// Serial port configuration for DEBUG output
#define	USART_DEBUG			USART1
#define	USART_DEBUG_BAUD		115200
#define	USART_DEBUG_GPIO_PORT		GPIOA
#define	USART_DEBUG_GPIO_AF		GPIO_AF_USART1
#define	USART_DEBUG_USART_PERIPH	RCC_APB2Periph_USART1
#define	USART_DEBUG_GPIO_PERIPH		RCC_AHB1Periph_GPIOA
#define	USART_DEBUG_GPIO_TX_PIN		GPIO_Pin_9
#define	USART_DEBUG_GPIO_RX_PIN		GPIO_Pin_10
#define	USART_DEBUG_GPIO_TX_SRC_PIN	GPIO_PinSource9
#define	USART_DEBUG_GPIO_RX_SRC_PIN	GPIO_PinSource10



#define CONFIG_SECTOR_NUMBER			1 //config section begins with 0x08004000

// Module allocation
#define	APPLICATION_DATA_RAM_ADDR	0x10000000	
#define	APPLICATION_DATA_RAM_SIZE	(64*1024)
#define	APPLICATION_FILE_SYSTEM		0x080A0000	// Start of Flash memory region used to store user applications (modules)




#endif //


/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2017-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef __EXT_SPI_H__
#define __EXT_SPI_H__

#include <stdint.h>

#define	EXT_SPI_MODE_SINGLE		0
#define	EXT_SPI_MODE_CONTINUOUS		1

int ext_spi_dma_request(char* tx_buf, char* rx_buf, int xfer_len);
void ext_spi_init(void);
void ext_spi_deinit(void);
void ext_spi_stop(void);
void ext_spi_send8(uint32_t val);
void ext_spi_send16(uint32_t val);
void ext_spi_send24(uint32_t val);
void ext_spi_send32(uint32_t val);


#endif // __EXT_SPI_H__


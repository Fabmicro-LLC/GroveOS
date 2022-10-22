/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#define CRC6_POLY 0b1000011

// data - 14 bit of data frame: {D11-D0, nE, wE}
unsigned int crc6(unsigned int data)
{
	int i;
	data <<= 6; // add 6 bits of empty CRC6

	unsigned int reminder = (data >> 14) & 0x3f;
	
	unsigned int cur_bit= 0b10000000000000;

	for(i = 0; i < 14; i++) {
		unsigned int tmp = reminder << 1;
		if(data & cur_bit) 
			tmp |= 1;

		if(tmp & 0b1000000) { 
			reminder = tmp ^ CRC6_POLY;
		} else {
			reminder = tmp & 0b0111111;
		}	
		cur_bit >>= 1;
	}
	
	return (reminder & 0b111111);
}




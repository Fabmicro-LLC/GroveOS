/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
int pcmu_codec_encoder(const void * _from, unsigned * fromLen, void * _to,   unsigned int * toLen);
int pcmu_codec_decoder(const void * _from, unsigned * fromLen, void * _to,   unsigned int * toLen);
int pcmu_decode(void * _to,  void *_from, unsigned int fromLen);
int pcma_decode(void * _to,  void *_from, unsigned int fromLen);

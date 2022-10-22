/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#ifndef __MODBUS_EXT1_H__
#define __MODBUS_EXT1_H__

void modbus_rx_ext1(void); // read serial buffers input
void modbus_msg_ext1(int msg, int p1, int p2); // process modbus related events
int modbus_enqueue_request_ext1(MODBUS_REQUEST* modbus_req);
int modbus_dequeue_range_ext1(int msg_start, int msg_end);
void modbus_check_queue_ext1(void);
int modbus_register_responder_ext1(MODBUS_RESPONSE* resp);
int modbus_unregister_responder_ext1(MODBUS_RESPONSE* resp);
int modbus_unregister_range_ext1(int msg_start, int msg_end);
int modbus_submit_response_ext1(MODBUS_RESPONSE* resp);


#endif


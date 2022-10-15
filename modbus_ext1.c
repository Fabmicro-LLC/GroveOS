/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "hardware.h"

#ifdef USART_EXT1

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "msg.h"
#include "modbus_common.h"
#include "modbus_ext1.h"
#include "softtimer.h"
#include "config.h"



#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

#define	MODBUS_STATE_READY		0
#define	MODBUS_STATE_WAIT_FOR_RESPONSE	1

extern uint8_t event_logging;

MODBUS_REQUEST *modbus_req_queue_ext1[MODBUS_MAX_REQ] = { 0 };
MODBUS_RESPONSE *modbus_responders_ext1[MODBUS_MAX_RESP] = { 0 };

int modbus_current_req_idx_ext1 = 0;
int modbus_state_ext1 = MODBUS_STATE_READY;
char modbus_slave_rxbuf_ext1[MODBUS_RX_BUF_SIZE];
int modbus_slave_rxbuf_len_ext1 = 0;

void modbus_check_req_queue_ext1(void);


void modbus_rx_ext1(void)
{

	if(config_active.ext1_port_modbus_mode == MODBUS_MASTER) {

		if( modbus_req_queue_ext1[modbus_current_req_idx_ext1] == NULL) {
			// There's no request pending, skip data that has been read
			char tmp_buf[MODBUS_RX_BUF_SIZE];
			int bytes_read = _read(USART_EXT1, tmp_buf, MODBUS_RX_BUF_SIZE, 1);
			if(event_logging) { 
				print("modbus_rx_ext1() skipped %d bytes of not requested data\r\n", bytes_read);
			}
			return;
		}
	
		int bytes_to_read = MIN(_maxread(USART_EXT1), MODBUS_RX_BUF_SIZE - modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxlen);

		if(bytes_to_read < 1) { 
			if(event_logging) { 
				print("modbus_rx() nothing to be read or rx buffer is full!\r\n");
			}
			return;
		}


		bytes_to_read = _read(USART_EXT1, modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxbuf + modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxlen, bytes_to_read, 1);

		if(bytes_to_read > 0)
			modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxlen += bytes_to_read;

		if(modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxlen >= MODBUS_RX_BUF_SIZE) {
			// RX buffer is full
			softtimer_stop_timer(MODBUS1_RX_COMPLETE);
			PostMessage(MODBUS1_RX_COMPLETE, 1, 0, 0);
		}

		softtimer_run_timer(MODBUS1_RX_COMPLETE, 30, 0, 0); // wait for 30ms more


	} else if(config_active.ext1_port_modbus_mode == MODBUS_SLAVE) {

		int bytes_to_read = MIN(_maxread(USART_EXT1), MODBUS_RX_BUF_SIZE - modbus_slave_rxbuf_len_ext1);

		if(bytes_to_read < 1) { 
			if(event_logging) { 
				print("modbus_rx_ext1() nothing to be read or rx buffer is full!\r\n");
			}
			return;
		}


		bytes_to_read = _read(USART_EXT1, modbus_slave_rxbuf_ext1 + modbus_slave_rxbuf_len_ext1, bytes_to_read, 1);

		if(bytes_to_read > 0)
			modbus_slave_rxbuf_len_ext1 += bytes_to_read;

		if(modbus_slave_rxbuf_len_ext1 >= MODBUS_RX_BUF_SIZE) {
			// RX buffer is full
			softtimer_stop_timer(MODBUS1_RX_COMPLETE);
			PostMessage(MODBUS1_RX_COMPLETE, 1, 0, 0);
		}

		softtimer_run_timer(MODBUS1_RX_COMPLETE, 5, 0, 0); // wait for 5ms more
	}
}


void modbus_msg_ext1(int msg, int p1, int p2)
{
	//print("modbus_msg_ext1() msg = %d, p1 = %d, p2 = %d\r\n", msg, p1, p2);

	switch(msg) {
		case MODBUS1_RX_COMPLETE: {


			if(config_active.ext1_port_modbus_mode == MODBUS_SLAVE) {

				if(event_logging) {
					print("modbus_msg_ext1() SLAVE RECV %d data bytes: ", modbus_slave_rxbuf_len_ext1);
					print_hex(modbus_slave_rxbuf_ext1, modbus_slave_rxbuf_len_ext1);
				}

				if(modbus_slave_rxbuf_len_ext1 < 4) {
					print("modbus_msg_ext1() bogus packet size: %d\r\n", modbus_slave_rxbuf_len_ext1);
					goto slave_cleanup;
				} 

				uint16_t my_crc = modbus_crc16(modbus_slave_rxbuf_ext1, modbus_slave_rxbuf_len_ext1-2);
				uint16_t his_crc = (modbus_slave_rxbuf_ext1[modbus_slave_rxbuf_len_ext1-1] << 8) | modbus_slave_rxbuf_ext1[modbus_slave_rxbuf_len_ext1-2];

				if(my_crc != his_crc) {
					print("modbus_msg_ext1() CRC mismatch: his = 0x%04X, my = 0x%04X\r\n", his_crc, my_crc);
					goto slave_cleanup;
				} 

				if(modbus_slave_rxbuf_ext1[0] != config_active.ext1_port_modbus_addr) {
					// dst address is not ours 
					goto slave_cleanup;
				}

				uint8_t func = modbus_slave_rxbuf_ext1[1];
				uint16_t reg = (modbus_slave_rxbuf_ext1[2] << 8) | modbus_slave_rxbuf_ext1[3];
				uint16_t qty = (modbus_slave_rxbuf_ext1[4] << 8) | modbus_slave_rxbuf_ext1[5];
				char txbuf[16];

				for(int i = 0; i < MODBUS_MAX_RESP; i++) {
					if(modbus_responders_ext1[i]->registered_func == func) {
						if(func <= FUNC_WRITE_MANY_HOLD_REGS) {
							if(reg >= modbus_responders_ext1[i]->registered_reg_start && 
							   reg + qty < modbus_responders_ext1[i]->registered_reg_end) {
								memcpy(modbus_responders_ext1[i]->rxbuf, modbus_slave_rxbuf_ext1, modbus_slave_rxbuf_len_ext1);
								PostMessage(modbus_responders_ext1[i]->msg_response, 0, (int)modbus_responders_ext1[i], 0);
								if(event_logging) {
									print("modbus_msg_ext1() Passing request to responder %d\r\n", i);
								}
								goto slave_cleanup;
							} 
						} else {
							memcpy(modbus_responders_ext1[i]->rxbuf, modbus_slave_rxbuf_ext1, modbus_slave_rxbuf_len_ext1);
							PostMessage(modbus_responders_ext1[i]->msg_response, 0, (int)modbus_responders_ext1[i], 0);
							if(event_logging) {
								print("modbus_msg_ext1() Passing request to responder %d\r\n", i);
							}
							goto slave_cleanup;
						} 
					}
				} 

				if(event_logging) {
					print("modbus_msg_ext1() No responder registered for func: 0x%02X, reg: 0x%02X, qty: %d\r\n", func, reg, qty );
					print("\r\n");
				}

				txbuf[0] = config_active.ext1_port_modbus_addr;
        			txbuf[1] = func | 0x80;
        			txbuf[2] = ERR_ADDR_NOT_AVAILABLE;
        			modbus_add_crc(txbuf, 3);
				USART_puts(USART_EXT1, txbuf, 5);

				slave_cleanup: 

				modbus_slave_rxbuf_len_ext1 = 0;

			} else if(config_active.ext1_port_modbus_mode == MODBUS_MASTER) {

				if(event_logging) {
					print("modbus_msg_ext1() MASTER RECV %d data bytes\r\n", modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxlen);
					print_hex(modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxbuf, modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxlen);
				}

				if(modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxlen == 0) {
					// TIMEOUT
					PostMessage(modbus_req_queue_ext1[modbus_current_req_idx_ext1]->msg_req_timeout, 0, (int)modbus_req_queue_ext1[modbus_current_req_idx_ext1], 0);
					goto master_rx_skip_packet;
				}

				if(modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxlen < 4) {
					print("modbus_msg_ext1() bogus packet size\r\n");
					PostMessage(modbus_req_queue_ext1[modbus_current_req_idx_ext1]->msg_req_timeout, 0, (int)modbus_req_queue_ext1[modbus_current_req_idx_ext1], 0);
					goto master_rx_skip_packet;
				} 

				uint16_t payload_size = modbus_calc_rx_payload_size(modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxbuf, modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxlen);
				uint16_t my_crc = modbus_crc16(modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxbuf, payload_size);
				uint16_t his_crc = (modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxbuf[payload_size+1] << 8) | modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxbuf[payload_size];

				if(my_crc != his_crc) {
					print("modbus_msg_ext1() CRC mismatch: his = 0x%04X, my = 0x%04X, payload_size = %d\r\n", his_crc, my_crc, payload_size);
					PostMessage(modbus_req_queue_ext1[modbus_current_req_idx_ext1]->msg_req_timeout, 0, (int)modbus_req_queue_ext1[modbus_current_req_idx_ext1], 0);

				} else {

					PostMessage(modbus_req_queue_ext1[modbus_current_req_idx_ext1]->msg_req_complete, 0, (int)modbus_req_queue_ext1[modbus_current_req_idx_ext1], 0);

				}

				goto master_next_req;

				master_rx_skip_packet:

				modbus_req_queue_ext1[modbus_current_req_idx_ext1]->rxlen = 0;

	
				master_next_req:

				// Deallocate slot in the queue
				modbus_req_queue_ext1[modbus_current_req_idx_ext1] = NULL;

				// Process next request
				modbus_state_ext1 = MODBUS_STATE_READY;
				modbus_check_req_queue_ext1();
			}

		} break;

		default:
			break;
	}
}

void modbus_check_req_queue_ext1(void)
{
	// Check request queue, initiate pending request only when bus is free

	if(modbus_state_ext1 == MODBUS_STATE_READY) {
		for(int i = 0; i < MODBUS_MAX_REQ; i++) {
			int k = (i + modbus_current_req_idx_ext1) % MODBUS_MAX_REQ;
			if(modbus_req_queue_ext1[k]) {
				USART_puts(USART_EXT1, modbus_req_queue_ext1[k]->txbuf, modbus_req_queue_ext1[k]->txlen);
				modbus_current_req_idx_ext1 = k;
				modbus_state_ext1 = MODBUS_STATE_WAIT_FOR_RESPONSE;
				softtimer_run_timer(MODBUS1_RX_COMPLETE,  modbus_req_queue_ext1[k]->timeout, 0, 0); // wait response for this amount of ms 
				if(event_logging) {
					print("modbus_check_req_queue_ext1() SENT %d data bytes: ", modbus_req_queue_ext1[k]->txlen);
					print_hex(modbus_req_queue_ext1[k]->txbuf, modbus_req_queue_ext1[k]->txlen);
				}
				break;
			}
		}
	}
}


int modbus_enqueue_request_ext1(MODBUS_REQUEST* modbus_request)
{
	int i, k;
	
	if(modbus_request == NULL)
		return -1;

	if(config_active.ext1_port_modbus_mode != MODBUS_MASTER) {
		print("modbus_enqueue_request_ext1() Request can be initiated only in MASTER mode!\r\n");
		return -1;
	}

	// Allocate free slot in queue for new request
	for(i = 0; i < MODBUS_MAX_REQ; i++) {
		k = (i + modbus_current_req_idx_ext1) % MODBUS_MAX_REQ;
		if(modbus_req_queue_ext1[k] == NULL) // free slot has been found
			break;
	}

	if(i == MODBUS_MAX_REQ) { 
		print("modbus_queue_request_ext1() queue overflow, no more free slots for new request!\r\n");
		return -2;
	}

	modbus_req_queue_ext1[k] = modbus_request;
	modbus_req_queue_ext1[k]->rxlen = 0;

	modbus_add_crc(modbus_req_queue_ext1[k]->txbuf, modbus_req_queue_ext1[k]->txlen);
	modbus_req_queue_ext1[k]->txlen += 2;

	modbus_check_req_queue_ext1();

	return 0; // OK
}


int modbus_dequeue_range_ext1(int msg_start, int msg_end)
{
	for(int i = 0; i < MODBUS_MAX_REQ; i++) {
		if(modbus_req_queue_ext1[i] && (modbus_state_ext1 == MODBUS_STATE_READY || modbus_current_req_idx_ext1 != i)) {
			if((modbus_req_queue_ext1[i]->msg_req_complete >= msg_start && modbus_req_queue_ext1[i]->msg_req_complete <= msg_end) ||
			   (modbus_req_queue_ext1[i]->msg_req_timeout >= msg_start && modbus_req_queue_ext1[i]->msg_req_timeout <= msg_end) 
			) {
				free(modbus_req_queue_ext1[i]);
				modbus_req_queue_ext1[i] = NULL;
			}
		}
	}
}


int modbus_register_responder_ext1(MODBUS_RESPONSE* resp)
{
	int i;

	if(resp == NULL)
		return -1;

	if(config_active.ext1_port_modbus_mode != MODBUS_SLAVE) {
		print("modbus_register_responder_ext1() Operation available only in SLAVE mode!\r\n");
		return -1;
	}

	// Find free slot
	for(i = 0; i < MODBUS_MAX_RESP; i++) {
		if(modbus_responders_ext1[i] == NULL)
			break;
	}	

	if(i == MODBUS_MAX_RESP) {
		print("modbus_register_responder_ext1() No free slot found for new responder!\r\n");
		return -2;
	}


	modbus_responders_ext1[i] = resp;

	return 0;
}


int modbus_unregister_responder_ext1(MODBUS_RESPONSE* resp)
{
	int i;

	if(resp == NULL)
		return -1;

	if(config_active.ext1_port_modbus_mode != MODBUS_SLAVE && resp->registered_reg_start >= 1000) {
		print("modbus_unregister_responder_ext1() Operation available only in SLAVE mode!\r\n");
		return -1;
	}

	// Find responder 
	for(i = 0; i < MODBUS_MAX_RESP; i++) {
		if(modbus_responders_ext1[i] == resp) {
			modbus_responders_ext1[i] = NULL;
			return 0;	
		}
	}	
	
	return -2; // responder not found
}


int modbus_unregister_range_ext1(int msg_start, int msg_end)
{
	for(int i = 0; i < MODBUS_MAX_RESP; i++) {
		if(modbus_responders_ext1[i]) {
			if(modbus_responders_ext1[i]->msg_response >= msg_start && modbus_responders_ext1[i]->msg_response <= msg_end) {
				free(modbus_responders_ext1[i]);
				modbus_responders_ext1[i] = NULL;
			}
		}
	}
}



int modbus_submit_response_ext1(MODBUS_RESPONSE* resp)
{
	int i;

	if(resp == NULL)
		return -1;

	if(config_active.ext1_port_modbus_mode != MODBUS_SLAVE && resp->registered_reg_start >= 1000) {
		print("modbus_submit_response_ext1() Operation available only in SLAVE mode!\r\n");
		return -1;
	}


	for(i = 0; i < MODBUS_MAX_RESP; i++) {
		if(modbus_responders_ext1[i]) {
			if(modbus_responders_ext1[i] == resp)
				break;
		}
	}

	if(i == MODBUS_MAX_RESP) {
		print("modbus_submit_response_ext1() Responder %p is not registered, hence cannot submit responses!\r\n", resp);
		return -1;
	}


	resp->rxlen = 0;
	resp->txbuf[0] = config_active.ext1_port_modbus_addr;

	modbus_add_crc(resp->txbuf, resp->txlen);
	resp->txlen += 2;


	if(event_logging) {
		print("mdbus_submit_response_ext1() Sending response of %d bytes: ", resp->txlen);
		print_hex(resp->txbuf, resp->txlen);
	}

	return USART_puts(USART_EXT1, resp->txbuf, resp->txlen);
}

#endif // USART_EXT1


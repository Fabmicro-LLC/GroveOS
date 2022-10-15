/*
	GroveOS - a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "hardware.h"

#ifdef DALI1_MODE 

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"
#include "msg.h"
#include "dali_common.h"
#include "dali1.h"
#include "softtimer.h"
#include "config.h"
#include "ext_gpio.h"
#include "ext_irq.h"


extern uint8_t event_logging; 

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

#define	DALI_STATE_READY		0
#define	DALI_STATE_WAIT_FOR_TX_COMPLETE	1
#define	DALI_STATE_WAIT_FOR_RESPONSE	2

extern uint8_t event_logging;
uint32_t microsec_timestamp;

DALI_REQUEST *dali1_req_queue[DALI_MAX_REQ] = { 0 };
DALI_RESPONSE *dali1_responders[DALI_MAX_RESP] = { 0 };

int dali1_current_req_idx = 0;
int dali1_state = DALI_STATE_READY;

char dali1_rx_chirp_buf[DALI_RX_BUF_SIZE];
char dali1_rx_time_buf[DALI_RX_BUF_SIZE];
volatile int dali1_rxbuf_len = 0;
volatile uint32_t dali1_rx_last_timestamp = 0;
volatile int dali1_release_bus = 0;

char dali1_txbuf[DALI_TX_BUF_SIZE];
int dali1_txbuf_len = 0;
int dali1_txbuf_byte = 0;
uint16_t dali1_txbuf_bit = 0x80; // start sending from MSB
uint16_t dali1_txbuf_clk = 0;

void dali1_check_req_queue(void);

void dali1_rx_irq(int exti_line);


void dali1_init(void)
{
	print("dali1_init() mode = %d, addr = %d, txrx_pol = %d, manchester_pol = %d\r\n", config_active.dali1.mode, config_active.dali1.short_addr, config_active.dali1.txrx_pol, config_active.dali1.manchester_pol);

	switch(config_active.dali1.mode) {
		case DALI_NONE:
			ext_irq_clear(ext_gpios[DALI1_RX_GPIO].irq_line, EXT_IRQ_OWNER_SYSTEM);
			print("dali1_init() Disabled!\r\n");
			break;
		case DALI_MASTER:
		case DALI_SLAVE:
			ext_irq_set(ext_gpios[DALI1_RX_GPIO].irq_line, dali1_rx_irq, EXT_IRQ_OWNER_SYSTEM);
			print("dali1_init() IRQ set on RX GPIO: %d\r\n", DALI1_RX_GPIO);
			break;
		default:
			print("dali1_init() Unsupported mode: %d\r\n", config_active.dali1.mode);
	}
}


// Must be called 1200*2 times a second, preferrably from Timer ISR. 
void dali1_transmit_one_bit(void)
{
	uint8_t chirp;

	if(dali1_txbuf_len - dali1_txbuf_byte <= 0) {

		// Nothing to send, tight TX line High

		if(dali1_release_bus) {
			// Send ONE while idle 
			if(config_active.dali1.txrx_pol == 0) // Transciever polariry
				ext_gpio_set(DALI1_TX_GPIO, 1);
			else
				ext_gpio_set(DALI1_TX_GPIO, 0);

			dali1_release_bus = 0;
		}


		return;
	}


	if(event_logging > 1) {
		if(dali1_txbuf_clk == 0)
			_print("DALI1: Sending buffer: {0x%02X, 0x%02X}\r\n", dali1_txbuf[0], dali1_txbuf[1]);
	}

	if(dali1_txbuf_clk < 2) {
		// Modulate preambule: first two clocks


		if((dali1_txbuf_clk % 2) == config_active.dali1.manchester_pol) { // Positive clock 
			// Send ZERO 
			if(config_active.dali1.txrx_pol == 0) // Transciever polariry
				chirp = 0;
			else
				chirp = 1;

		} else { // Negatve clock

			// Send ONE 
			if(config_active.dali1.txrx_pol == 0) // Transciever polariry
				chirp = 1;
			else
				chirp = 0;
		}

		ext_gpio_set(DALI1_TX_GPIO, chirp);

		if(event_logging > 1) 
			_print("DALI1: PREAMB clk = %d, chirp = %d\r\n", dali1_txbuf_clk, chirp);

		dali1_txbuf_clk++;

		dali1_rxbuf_len = 0; // empty RX buffer, prepare to receive echo

		return;
	}

	if(dali1_txbuf_len - dali1_txbuf_byte > 0) {
		
		// Modulate one byte of dali1_txbuf[dali1_txbuf_byte] using Manchester

		// Bit by bit at a time 
		if(dali1_txbuf[dali1_txbuf_byte] & dali1_txbuf_bit) {
			// Send ONE 
			if((dali1_txbuf_clk % 2) == config_active.dali1.manchester_pol) { // Positive clock 
				if(config_active.dali1.txrx_pol == 0) // Transciever polariry
					chirp = 0;
				else
					chirp = 1;
			} else {
				if(config_active.dali1.txrx_pol == 0) // Transciever polariry
					chirp = 1;
				else
					chirp = 0;
			}

		} else { 
			// Send ZERO 
			if((dali1_txbuf_clk % 2) == config_active.dali1.manchester_pol) { // Positive clock 
				if(config_active.dali1.txrx_pol == 0) // Transciever polariry
					chirp = 1;
				else
					chirp = 0;
			} else {
				if(config_active.dali1.txrx_pol == 0) // Transciever polariry
					chirp = 0;
				else
					chirp = 1;
			}

		}


		ext_gpio_set(DALI1_TX_GPIO, chirp);

		if(event_logging > 1) 
			_print("DALI1: DATA clk = %d, byte_idx = %d, byte = 0x%02X, mask = 0x%02X, bit = %d, chirp = %d\r\n", dali1_txbuf_clk, dali1_txbuf_byte, dali1_txbuf[dali1_txbuf_byte], dali1_txbuf_bit, (dali1_txbuf[dali1_txbuf_byte] & dali1_txbuf_bit) != 0, chirp);

		dali1_txbuf_clk++;

		if(dali1_txbuf_clk % 2 == 0) {

			dali1_txbuf_bit = dali1_txbuf_bit >> 1;
		
			if(dali1_txbuf_bit == 0x0) {
				// A complete byte has been clocked 
				dali1_txbuf_byte++;
				dali1_txbuf_bit = 0x80;

				if(dali1_txbuf_len - dali1_txbuf_byte == 0) {
					// Whole buffer has been sent
					dali1_txbuf_len = 0;
					dali1_txbuf_byte = 0;
					dali1_txbuf_clk = 0;

					dali1_release_bus++;

					if(event_logging) {
						print("dali1_transmit_one_bit() Preparing for response\r\n");

						if(dali1_rxbuf_len) {
							print("dali1_transmit_one_bit() RX buffer is not empty, dali1_rxbuf_len = %d\r\n", dali1_rxbuf_len);
							if(event_logging>1)
								print_hex(dali1_rx_chirp_buf, dali1_rxbuf_len);
						}
					}

					softtimer_run_timer(DALI1_TX_COMPLETE,  2, 0, 0); 

				}
			}
		}

	}
}

int dali1_tx(char *buf, int len)
{
	if(event_logging)
		print("dali1_tx() buf = %p, len = %d, dali1_txbuf_len = %d\r\n", buf, len, dali1_txbuf_len);

	__disable_irq();

	if(dali1_txbuf_len + len > DALI_TX_BUF_SIZE) {
		__enable_irq();
		if(event_logging)
			print("dali1_tx() TX buffer overflow: dali1_txbuf_len + len = %d > %d\r\n", dali1_txbuf_len + len, DALI_TX_BUF_SIZE);

		return -1; // TX buffer overflow
	}

	memcpy(dali1_txbuf + dali1_txbuf_len, buf, len);
	dali1_txbuf_len += len;

	__enable_irq();

	return 0; // OK, data has been scheduled for transmission
}

void dali1_rx_irq(int ext_num)
{

	if(dali1_rx_last_timestamp == 0)
		dali1_rx_last_timestamp = microsec_timestamp;
	


	if(config_active.dali1.mode == DALI_MASTER 
		&& dali1_state == DALI_STATE_WAIT_FOR_RESPONSE
		&& !dali1_release_bus) {

		//_print("dali1_rx_irq() 1 dali1_state = %d, dali1_release_bus = %d\r\n", dali1_state, dali1_release_bus);

		if(dali1_rxbuf_len >= DALI_RX_BUF_SIZE) {
			// RX buffer overflow ???
			if(event_logging) { 
				_print("dali1_rx_irq() RX chirp buffer overflow while MASTER!\r\n");
			}
			goto dali1_rx_irq_end;
		}

		dali1_rx_chirp_buf[dali1_rxbuf_len] = ext_gpio_get(DALI1_RX_GPIO);
		dali1_rx_time_buf[dali1_rxbuf_len] = microsec_timestamp - dali1_rx_last_timestamp;

		dali1_rxbuf_len++;

		if(dali1_req_queue[dali1_current_req_idx]->need_response) {
			if(dali1_rxbuf_len >= DALI_RX_BUF_SIZE) {
				// RX buffer is full
				softtimer_stop_timer(DALI1_RX_COMPLETE);
				PostMessage(DALI1_RX_COMPLETE, 1, 0, 0);
			} else {
				softtimer_run_timer(DALI1_RX_COMPLETE, 5, 0, 0); // wait for 5ms for more chirps
			}
		} else {

			dali1_rxbuf_len = 0;
		}

	} else if(config_active.dali1.mode == DALI_SLAVE) {

		if(dali1_rxbuf_len >= DALI_RX_BUF_SIZE) {
			// RX buffer overflow ???
			if(event_logging) { 
				_print("dali1_rx_irq() RX chirp buffer overflow while SLAVE!\r\n");
			}
			goto dali1_rx_irq_end;
		}

		dali1_rx_chirp_buf[dali1_rxbuf_len] = ext_gpio_get(DALI1_RX_GPIO);
		dali1_rx_time_buf[dali1_rxbuf_len] = microsec_timestamp - dali1_rx_last_timestamp;

		dali1_rxbuf_len++;

		if(dali1_rxbuf_len >= DALI_RX_BUF_SIZE) {
			// RX buffer is full
			softtimer_stop_timer(DALI1_RX_COMPLETE);
			PostMessage(DALI1_RX_COMPLETE, 1, 0, 0);
		} else {
			softtimer_run_timer(DALI1_RX_COMPLETE, 5, 0, 0); // wait for 5ms for more chirps
		}
	} else {
		if(event_logging > 1) {
			print("dali1_rx_irq() Unexpected!\r\n");
		}
	}

	dali1_rx_irq_end:

	dali1_rx_last_timestamp = microsec_timestamp;
}


void dali1_msg(int msg, int p1, int p2)
{
	//print("dali1_msg() msg = %d, p1 = %d, p2 = %d\r\n", msg, p1, p2);

	switch(msg) {

		case DALI1_TX_COMPLETE: {


			if(config_active.dali1.mode == DALI_MASTER) {
				// Now wait for response data

				dali1_state = DALI_STATE_WAIT_FOR_RESPONSE;
				dali1_rxbuf_len = 0; // Clear RX buffer, just in case

				if(!dali1_req_queue[dali1_current_req_idx]) {
					print("dali1_transmit_one_bit() NULL pointer, TX bypassing request ?\r\n");
					break;
				}

				if(dali1_req_queue[dali1_current_req_idx]->need_response) {
					// wait response for this amount of ms 
					softtimer_run_timer(DALI1_RX_COMPLETE,  dali1_req_queue[dali1_current_req_idx]->timeout, 0, 0); 
				} else {
					// just delay for this amount of ms 
					softtimer_run_timer(DALI1_TX_DELAY_COMPLETE,  dali1_req_queue[dali1_current_req_idx]->delay, 0, 0); 
				}

			} else if(config_active.dali1.mode == DALI_SLAVE) {
				// Now wait for new request 

				dali1_state = DALI_STATE_READY;
				dali1_rxbuf_len = 0; // Clear RX buffer, just in case
			}
		} break;

		case DALI1_TX_DELAY_COMPLETE: {
			if(config_active.dali1.mode != DALI_MASTER) {
				print("dali1_msg() DALI1_TX_DELAY_COMPLETE can happen for MASTER mode only!\r\n");
				break;
			}

			if(dali1_req_queue[dali1_current_req_idx]->msg_req_complete)
				PostMessage(dali1_req_queue[dali1_current_req_idx]->msg_req_complete, 0, (int)dali1_req_queue[dali1_current_req_idx], 0);

			// Deallocate slot in the queue
			dali1_req_queue[dali1_current_req_idx] = NULL;

			// Process next request
			dali1_state = DALI_STATE_READY;
			dali1_check_req_queue();

		} break;

		case DALI1_RX_COMPLETE: {


			if(config_active.dali1.mode == DALI_SLAVE) {

				if(event_logging) {
					print("dali1_msg() SLAVE RECV %d chirps: ", dali1_rxbuf_len);
					for(int i = 0; i < dali1_rxbuf_len; i++)
						_print("DALI1: RX chirp = %d, time = %d\r\n", dali1_rx_chirp_buf[i], dali1_rx_time_buf[i]);
				}

				if(dali1_rxbuf_len < 17) {
					if(event_logging)
						print("dali1_msg() bogus chirp packet size: %d\r\n", dali1_rxbuf_len);
					goto slave_cleanup;
				} 


				char data_buf[2];

				int bytes_decoded = dali_decode_manchester(data_buf, dali1_rx_chirp_buf, dali1_rx_time_buf, dali1_rxbuf_len, config_active.dali1.manchester_pol ^ config_active.dali1.txrx_pol);

				if(bytes_decoded < 1) {
					if(event_logging)
						print("dali1_msg() manchester decoder failed!\r\n");
					goto slave_cleanup;
				}

				if(event_logging)
					print("dali1_msg() manchester decoded %d bytes\r\n", bytes_decoded);

				if(data_buf[0] & 0x80 == 0) { // addressed packet
					char addr = (data_buf[0] & 0x7f) >> 1;

					if(addr == config_active.dali1.short_addr) {
						for(int i = 0; i < DALI_MAX_RESP; i++) {
							if(dali1_responders[i]->cmd == 0) {
								memcpy(dali1_responders[i]->rxbuf, data_buf, 2);
								PostMessage(dali1_responders[i]->msg_response, 0, (int)dali1_responders[i], 0);
								if(event_logging) {
									print("dali1_msg() Passing request to responder %d\r\n", i);
								}
								goto slave_cleanup;
							}
						} 
					} else {
						// dst address is not ours 
						if(event_logging) 
							print("dali1_msg() dst addr = %d is not ours = %d\r\n", addr, config_active.dali1.short_addr);

						goto slave_cleanup;
					}
				} else { // Command packet
					char cmd = data_buf[0];
					for(int i = 0; i < DALI_MAX_RESP; i++) {
						if(dali1_responders[i]->cmd == cmd) {
							memcpy(dali1_responders[i]->rxbuf, data_buf, 2);
							PostMessage(dali1_responders[i]->msg_response, 0, (int)dali1_responders[i], 0);
							if(event_logging) {
								print("dali1_msg() Passing request to responder %d\r\n", i);
							}
							goto slave_cleanup;
						}
					} 

				} 

				if(event_logging) {
					print("dali1_msg() No responder registered for: {0x%02X, 0x%02X}\r\n", data_buf[0], data_buf[1] );
				}

				slave_cleanup: 

				dali1_rxbuf_len = 0;

			} else if(config_active.dali1.mode == DALI_MASTER) {


				if(dali1_state != DALI_STATE_WAIT_FOR_RESPONSE) {
					if(event_logging)
						print("dali1_msg() Master received %d chirps while in wrong state %d, skipped!\r\n", dali1_rxbuf_len, dali1_state);
					goto master_rx_skip_packet;
				}

				if(event_logging) {
					print("dali1_msg() Master received %d chirps of response:\r\n", dali1_rxbuf_len);
					if(event_logging > 1)
						print_hex(dali1_rx_chirp_buf, dali1_rxbuf_len);
				}

				if(!dali1_req_queue[dali1_current_req_idx]->need_response) {
					if(event_logging)
						print("dali1_msg() Master was not expected this response, skipped!\r\n");
					goto master_rx_skip_packet;
				}

				if(dali1_rxbuf_len < 9) { // Response must contain 18 chirps (9 bits)
					// TIMEOUT
					if(event_logging)
						print("dali1_msg() Master was expecting at least 18 chirps instead of %d!\r\n", dali1_rxbuf_len);

					if(dali1_req_queue[dali1_current_req_idx]->msg_req_timeout)
						PostMessage(dali1_req_queue[dali1_current_req_idx]->msg_req_timeout, 0, (int)dali1_req_queue[dali1_current_req_idx], 0);
					goto master_rx_skip_packet;
				}

				dali1_req_queue[dali1_current_req_idx]->rxlen = 
					dali_decode_manchester(dali1_req_queue[dali1_current_req_idx]->rxbuf, dali1_rx_chirp_buf, dali1_rx_time_buf, dali1_rxbuf_len, config_active.dali1.manchester_pol ^ config_active.dali1.txrx_pol);

				if(dali1_req_queue[dali1_current_req_idx]->rxlen < 1) {
					if(event_logging)
						print("dali1_msg() manchester decoder failed!\r\n");

					if(dali1_req_queue[dali1_current_req_idx]->msg_req_timeout)
						PostMessage(dali1_req_queue[dali1_current_req_idx]->msg_req_timeout, 0, (int)dali1_req_queue[dali1_current_req_idx], 0);

					goto master_rx_skip_packet;
				}

				if(event_logging)
					print("dali1_msg() manchester decoded %d bytes\r\n", dali1_req_queue[dali1_current_req_idx]->rxlen);

				if(dali1_req_queue[dali1_current_req_idx]->msg_req_complete)
					PostMessage(dali1_req_queue[dali1_current_req_idx]->msg_req_complete, 0, (int)dali1_req_queue[dali1_current_req_idx], 0);

				goto master_next_req;

				master_rx_skip_packet:

				dali1_req_queue[dali1_current_req_idx]->rxlen = 0;

	
				master_next_req:

	
				dali1_rxbuf_len = 0; // Empty RX buffer

				// Deallocate slot in the queue
				dali1_req_queue[dali1_current_req_idx] = NULL;

				// Process next request
				dali1_state = DALI_STATE_READY;
				dali1_check_req_queue();
			}

		} break;

		default:
			break;
	}
}

void dali1_check_req_queue(void)
{
	// Check request queue, initiate pending request only when bus is free

	int i;

	if(dali1_state == DALI_STATE_READY) {
		for(i = 0; i < DALI_MAX_REQ; i++) {
			int k = (i + dali1_current_req_idx) % DALI_MAX_REQ;
			if(dali1_req_queue[k]) {
				dali1_tx(dali1_req_queue[k]->txbuf, dali1_req_queue[k]->txlen);
				dali1_current_req_idx = k;
				dali1_state = DALI_STATE_WAIT_FOR_TX_COMPLETE;

				if(event_logging) {
					print("dali1_check_req_queue() To be sent %d data bytes:\r\n", dali1_req_queue[k]->txlen);
					if(event_logging>1)
						print_hex(dali1_req_queue[k]->txbuf, dali1_req_queue[k]->txlen);
				}

				break;
			}
		}

		if(i == DALI_MAX_REQ && event_logging) 
			print("dali1_check_req_queue() Nothing to be sent\r\n");
	}
}


int dali1_enqueue_request(DALI_REQUEST* dali1_request)
{
	int i, k;
	
	if(dali1_request == NULL)
		return -1;

	if(config_active.dali1.mode != DALI_MASTER) {
		print("dali1_enqueue_request() Request can be initiated only in MASTER mode!\r\n");
		return -1;
	}

	// Allocate free slot in queue for new request
	for(i = 0; i < DALI_MAX_REQ; i++) {
		k = (i + dali1_current_req_idx) % DALI_MAX_REQ;
		if(dali1_req_queue[k] == NULL) // free slot has been found
			break;
	}

	if(i == DALI_MAX_REQ) { 
		print("dali1_queue_request() queue overflow, no more free slots for new request!\r\n");
		return -2;
	}

	dali1_req_queue[k] = dali1_request;
	dali1_req_queue[k]->rxlen = 0;


	if(event_logging)
		print("dali1_queue_request() Request queued, dali1_request = %p, idx = %d\r\n", dali1_request, k);

	dali1_check_req_queue();

	return 0; // OK
}


int dali1_dequeue_range(int msg_start, int msg_end)
{
	for(int i = 0; i < DALI_MAX_REQ; i++) {
		if(dali1_req_queue[i] && (dali1_state == DALI_STATE_READY || dali1_current_req_idx != i)) {
			if((dali1_req_queue[i]->msg_req_complete >= msg_start && dali1_req_queue[i]->msg_req_complete <= msg_end) ||
			   (dali1_req_queue[i]->msg_req_timeout >= msg_start && dali1_req_queue[i]->msg_req_timeout <= msg_end) 
			) {
				free(dali1_req_queue[i]);
				dali1_req_queue[i] = NULL;
			}
		}
	}
}


int dali1_register_responder(DALI_RESPONSE* resp)
{
	int i;

	if(resp == NULL)
		return -1;

	if(config_active.dali1.mode != DALI_SLAVE) {
		print("dali1_register_responder() Operation available only in SLAVE mode!\r\n");
		return -1;
	}

	// Find free slot
	for(i = 0; i < DALI_MAX_RESP; i++) {
		if(dali1_responders[i] == NULL)
			break;
	}	

	if(i == DALI_MAX_RESP) {
		print("dali1_register_responder() No free slot found for new responder!\r\n");
		return -2;
	}


	dali1_responders[i] = resp;

	return 0;
}


int dali1_unregister_responder(DALI_RESPONSE* resp)
{
	int i;

	if(resp == NULL)
		return -1;

	if(config_active.dali1.mode != DALI_SLAVE) {
		print("dali1_unregister_responder() Operation available only in SLAVE mode!\r\n");
		return -1;
	}

	// Find responder 
	for(i = 0; i < DALI_MAX_RESP; i++) {
		if(dali1_responders[i] == resp) {
			dali1_responders[i] = NULL;
			return 0;	
		}
	}	
	
	return -2; // responder not found
}


int dali1_unregister_range(int msg_start, int msg_end)
{
	for(int i = 0; i < DALI_MAX_RESP; i++) {
		if(dali1_responders[i]) {
			if(dali1_responders[i]->msg_response >= msg_start && dali1_responders[i]->msg_response <= msg_end) {
				free(dali1_responders[i]);
				dali1_responders[i] = NULL;
			}
		}
	}
}



int dali1_submit_response(DALI_RESPONSE* resp)
{
	int i;

	if(resp == NULL)
		return -1;

	if(config_active.dali1.mode != DALI_SLAVE) {
		print("dali1_submit_response() Operation available only in SLAVE mode!\r\n");
		return -1;
	}


	for(i = 0; i < DALI_MAX_RESP; i++) {
		if(dali1_responders[i]) {
			if(dali1_responders[i] == resp)
				break;
		}
	}

	if(i == DALI_MAX_RESP) {
		print("dali1_submit_response() Responder %p is not registered, hence cannot submit responses!\r\n", resp);
		return -1;
	}


	resp->rxlen = 0;
	resp->txbuf[0] = config_active.dali1.short_addr;


	if(event_logging) {
		print("dali1_submit_response() Sending response of %d bytes: ", resp->txlen);
		print_hex(resp->txbuf, resp->txlen);
	}

	return dali1_tx(resp->txbuf, resp->txlen);
}

#endif // DALI1_MODE 


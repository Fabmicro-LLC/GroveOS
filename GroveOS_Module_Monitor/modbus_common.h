/*
        GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers.

	Written by Ruslan Zalata and Evgeny Korolenko.
        
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/

#ifndef __MODBUS__H__
#define __MODBUS__H__

#define	MODBUS_NONE	0
#define	MODBUS_MASTER	1
#define	MODBUS_SLAVE	2

#define	MODBUS_MAX_REQ		16			// Request queue length
#define	MODBUS_MAX_RESP		64			// Max number of responders 
#define	MODBUS_TX_BUF_SIZE	256
#define	MODBUS_RX_BUF_SIZE	256
#define	MODBUS_REQ_TIMEOUT	100			// Wait for response this amount of ms


#define FUNC_READ_COILS                 0x01            // Read one or more coils' status
#define FUNC_READ_DISC_INPUT            0x02            // Read one or more discrete inputes
#define FUNC_READ_HOLD_REGS             0x03            // Read one or more holding registers (current PWM)
#define FUNC_READ_INPUT_REGS            0x04            // Read one or more input registers (inpud ADC)
#define FUNC_WRITE_ONE_COIL_REG         0x05            // Write one coil (ON/OFF)
#define FUNC_WRITE_ONE_HOLD_REG         0x06            // Write one holding register (PWM)
#define FUNC_WRITE_MANY_COIL_REG        0x0F            // Write many coils (ON/OFF)
#define FUNC_WRITE_MANY_HOLD_REGS	0x10            // Write many holding registers (PWM)
#define	FUNC_WRITE_HOLD_REGS		FUNC_WRITE_MANY_HOLD_REGS
#define FUNC_READ_ID                    0x11            // Read device ID

#define ERR_FUNC_OK			0x00            // Error code: no error
#define ERR_FUNC_NOT_IMPLEMENTED        0x01            // Error code: func not implemented
#define ERR_ADDR_NOT_AVAILABLE          0x02            // Error code: register address not available
#define ERR_WRONG_ARGS                  0x03            // Error code: invalid arguments


typedef struct _MODBUS_REQUEST {
	int msg_req_complete;
	int msg_req_timeout;
	char txbuf[MODBUS_TX_BUF_SIZE];
	char rxbuf[MODBUS_RX_BUF_SIZE];
	int timeout;
	int txlen;
	int rxlen;
} MODBUS_REQUEST;


#define	MODBUS_READ_ID(modbus_req, addr, msg_complete, msg_timeout) \
	if(modbus_req) {\
		modbus_req->txbuf[0] = (addr); \
		modbus_req->txbuf[1] = FUNC_READ_ID; \
		modbus_req->txbuf[2] = 0x0; \
		modbus_req->txbuf[3] = 0x0; \
		modbus_req->txlen = 4; \
		modbus_req->rxlen = 0; \
		modbus_req->timeout = MODBUS_REQ_TIMEOUT; \
		modbus_req->msg_req_complete = msg_complete; \
		modbus_req->msg_req_timeout = msg_timeout; \
	} else {\
		svc_debug_print(str, sprintf(str, "MODBUS: out of memory (addr=%d, func=%d)\r\n", addr, FUNC_READ_ID)); \
	}\
	

#define	MODBUS_READ_COIL_REGS(modbus_req, addr, reg, qty, msg_complete, msg_timeout) \
	if(modbus_req) {\
		modbus_req->txbuf[0] = (addr); \
		modbus_req->txbuf[1] = FUNC_READ_COILS; \
		modbus_req->txbuf[2] = ((reg) >> 8) & 0xff; \
		modbus_req->txbuf[3] = (reg) & 0xff; \
		modbus_req->txbuf[4] = ((qty) >> 8) & 0xff; \
		modbus_req->txbuf[5] = (qty) & 0xff; \
		modbus_req->txlen = 6; \
		modbus_req->rxlen = 0; \
		modbus_req->timeout = MODBUS_REQ_TIMEOUT; \
		modbus_req->msg_req_complete = msg_complete; \
		modbus_req->msg_req_timeout = msg_timeout; \
	} else {\
		svc_debug_print(str, sprintf(str, "MODBUS: out of memory (addr=%d, func=%d)\r\n", addr, FUNC_READ_COILS)); \
	}\


#define	MODBUS_READ_HOLDING_REGS(modbus_req, addr, reg, qty, msg_complete, msg_timeout) \
	if(modbus_req) {\
		modbus_req->txbuf[0] = (addr); \
		modbus_req->txbuf[1] = FUNC_READ_HOLD_REGS; \
		modbus_req->txbuf[2] = ((reg) >> 8) & 0xff; \
		modbus_req->txbuf[3] = (reg) & 0xff; \
		modbus_req->txbuf[4] = ((qty) >> 8) & 0xff; \
		modbus_req->txbuf[5] = (qty) & 0xff; \
		modbus_req->txlen = 6; \
		modbus_req->rxlen = 0; \
		modbus_req->timeout = MODBUS_REQ_TIMEOUT; \
		modbus_req->msg_req_complete = msg_complete; \
		modbus_req->msg_req_timeout = msg_timeout; \
	} else {\
		svc_debug_print(str, sprintf(str, "MODBUS: out of memory (addr=%d, func=%d)\r\n", addr, FUNC_READ_HOLD_REGS)); \
	}\

#define	MODBUS_READ_INPUT_REGS(modbus_req, addr, reg, qty, msg_complete, msg_timeout) \
	if(modbus_req) {\
		modbus_req->txbuf[0] = (addr); \
		modbus_req->txbuf[1] = FUNC_READ_INPUT_REGS; \
		modbus_req->txbuf[2] = ((reg) >> 8) & 0xff; \
		modbus_req->txbuf[3] = (reg) & 0xff; \
		modbus_req->txbuf[4] = ((qty) >> 8) & 0xff; \
		modbus_req->txbuf[5] = (qty) & 0xff; \
		modbus_req->txlen = 6; \
		modbus_req->rxlen = 0; \
		modbus_req->timeout = MODBUS_REQ_TIMEOUT; \
		modbus_req->msg_req_complete = msg_complete; \
		modbus_req->msg_req_timeout = msg_timeout; \
	} else {\
		svc_debug_print(str, sprintf(str, "MODBUS: out of memory (addr=%d, func=%d)\r\n", addr, FUNC_READ_INPUT_REGS)); \
	}\

#define	MODBUS_WRITE_MANY_HOLD_REGS(modbus_req, addr, reg, qty, data, data_len, msg_complete, msg_timeout) \
	if(modbus_req) {\
		modbus_req->txbuf[0] = (addr); \
		modbus_req->txbuf[1] = FUNC_WRITE_MANY_HOLD_REGS; \
		modbus_req->txbuf[2] = ((reg) >> 8) & 0xff; \
		modbus_req->txbuf[3] = (reg) & 0xff; \
		modbus_req->txbuf[4] = ((qty) >> 8) & 0xff; \
		modbus_req->txbuf[5] = (qty) & 0xff; \
		modbus_req->txbuf[6] = (data_len); \
		memcpy(modbus_req->txbuf+7, (data), (data_len)); \
		modbus_req->txlen = 7+(data_len); \
		modbus_req->rxlen = 0; \
		modbus_req->timeout = MODBUS_REQ_TIMEOUT; \
		modbus_req->msg_req_complete = msg_complete; \
		modbus_req->msg_req_timeout = msg_timeout; \
	} else {\
		svc_debug_print(str, sprintf(str, "MODBUS: out of memory (addr=%d, func=%d)\r\n", addr, FUNC_WRITE_MANY_HOLD_REGS)); \
	}\


#define	MODBUS_WRITE_ONE_HOLD_REG(modbus_req, addr, reg, data, msg_complete, msg_timeout) \
	if(modbus_req) {\
		modbus_req->txbuf[0] = (addr); \
		modbus_req->txbuf[1] = FUNC_WRITE_ONE_HOLD_REG; \
		modbus_req->txbuf[2] = ((reg) >> 8) & 0xff; \
		modbus_req->txbuf[3] = (reg) & 0xff; \
		modbus_req->txbuf[4] = ((data) >> 8) & 0xff; \
		modbus_req->txbuf[5] = (data) & 0xff; \
		modbus_req->txlen = 6; \
		modbus_req->rxlen = 0; \
		modbus_req->timeout = MODBUS_REQ_TIMEOUT; \
		modbus_req->msg_req_complete = msg_complete; \
		modbus_req->msg_req_timeout = msg_timeout; \
	} else {\
		svc_debug_print(str, sprintf(str, "MODBUS: out of memory (addr=%d, func=%d)\r\n", addr, FUNC_WRITE_ONE_HOLD_REG)); \
	}\


typedef struct _MODBUS_RESPONSE {
        int msg_response;
        char txbuf[MODBUS_TX_BUF_SIZE];
        char rxbuf[MODBUS_RX_BUF_SIZE];
        int txlen;
        int rxlen;
        int registered_func;
        int registered_reg_start;
        int registered_reg_end;
} MODBUS_RESPONSE;


#define	MODBUS_CREATE_RESPONDER(modbus_resp, msg, func, reg_start, reg_end) \
	memset(modbus_resp, 0, sizeof(MODBUS_RESPONSE)); \
	(modbus_resp)->msg_response = msg; \
	(modbus_resp)->registered_func = func; \
	(modbus_resp)->registered_reg_start = reg_start; \
	(modbus_resp)->registered_reg_end = reg_end; \

#define	MODBUS_GET_REQ_FUNC(modbus_resp) (unsigned int)((modbus_resp)->rxbuf[1])
#define	MODBUS_GET_REQ_QTY(modbus_resp) (unsigned int)(((modbus_resp)->rxbuf[4] << 8) | modbus_resp->rxbuf[5])
#define	MODBUS_GET_REQ_REG(modbus_resp) (unsigned int)(((modbus_resp)->rxbuf[2] << 8) | modbus_resp->rxbuf[3])
#define	MODBUS_GET_REQ_REG_BUF(modbus_resp, offset) (char*)(((modbus_resp)->rxbuf+7+(offset)))
#define	MODBUS_GET_REQ_REG_BUF_SIZE(modbus_resp) (uint8_t)(((modbus_resp)->rxbuf[6]))
#define	MODBUS_GET_REQ_REG_VAL(modbus_resp, index) (unsigned int)(((modbus_resp)->rxbuf[index*2+7] << 8) | modbus_resp->rxbuf[index*2+8])

#define	MODBUS_GET_RESP_BUF(modbus_resp) (char*)(((modbus_resp)->txbuf+7))

#define MODBUS_RESPONSE_SET_BUF_SIZE(modbus_resp, size) \
	(modbus_resp)->txbuf[2] = size; \
	(modbus_resp)->txlen += size; \

#define MODBUS_RESPONSE_ERROR(modbus_resp, func, error) \
	(modbus_resp)->txbuf[1] = func | 0x80; \
	(modbus_resp)->txbuf[2] = error; \
	(modbus_resp)->txlen = 3; \

#define MODBUS_RESPONSE_OK(modbus_resp, func) \
	(modbus_resp)->txbuf[1] = func | 0x80; \
	(modbus_resp)->txbuf[2] = ERR_FUNC_OK; \
	(modbus_resp)->txlen = 3; \

 
#define MODBUS_RESPONSE_START(modbus_resp, func) \
	(modbus_resp)->txbuf[0] = 0; \
	(modbus_resp)->txbuf[1] = func; \
	(modbus_resp)->txbuf[2] = 0; \
	(modbus_resp)->txlen = 3;

#define MODBUS_RESPONSE_ADD_BYTE(modbus_resp, val) \
	(modbus_resp)->txbuf[2] += 1; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 1; \

#define MODBUS_RESPONSE_ADD_WORD(modbus_resp, val) \
	(modbus_resp)->txbuf[2] += 2; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen] = (((uint32_t)val) >> 8) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 2; \

#define MODBUS_RESPONSE_ADD_DWORD(modbus_resp, val) \
	(modbus_resp)->txbuf[2] += 4; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen] = (((uint32_t)val) >> 24) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = (((uint32_t)val) >> 16) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+2] = (((uint32_t)val) >> 8) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+3] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 4; \



#define MODBUS_WRITE_RESPONSE_START(modbus_resp, func) \
	(modbus_resp)->txbuf[0] = 0; \
	(modbus_resp)->txbuf[1] = func; \
	(modbus_resp)->txlen = 2;

#define MODBUS_WRITE_RESPONSE_ADD_BYTE(modbus_resp, val) \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 1; \

#define MODBUS_WRITE_RESPONSE_ADD_WORD(modbus_resp, val) \
	(modbus_resp)->txbuf[(modbus_resp)->txlen] = (((uint32_t)val) >> 8) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 2; \

#define MODBUS_WRITE_RESPONSE_ADD_DWORD(modbus_resp, val) \
	(modbus_resp)->txbuf[(modbus_resp)->txlen] = (((uint32_t)val) >> 24) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+1] = (((uint32_t)val) >> 16) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+2] = (((uint32_t)val) >> 8) & 0xff; \
	(modbus_resp)->txbuf[(modbus_resp)->txlen+3] = ((uint32_t)val) & 0xff; \
	(modbus_resp)->txlen += 4; \

#define	MODBUS_WRITE_RESPONSE_BUF(modbus_resp) (char*)(((modbus_resp)->txbuf+8))
#define	MODBUS_WRITE_RESPONSE_BUF_SIZE(modbus_resp, size) (modbus_resp)->txlen = (size+8)
	

#endif


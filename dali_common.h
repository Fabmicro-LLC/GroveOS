/*


DALI commands are 16 bits long, an address byte followed by a data byte. The address byte specifies a target device or a special command addressed to all devices.

When addressing a device, the least significant bit of the address byte specifies the interpretation of the data byte, with "0" meaning a lighting level byte follows, and "1" meaning a command follows.

When sending a special command, the data byte is a parameter. An important special command saves the data byte to a "data transfer register" which is then used as a parameter by subsequent addressed commands.

Address byte forms:

    0AAAAAAS: Target device 0 ≤ A < 64.
    100AAAAS: Target group 0 ≤ A < 16. Each device may be a member of any or all groups.
    101CCCC1: Special commands 256–271
    110CCCC1: Special commands 272–287
    111xxxx1: Reserved (x ≠ 15)
    1111111S: Broadcast to all devices.

*/


#ifndef _DALI_COMMON_H_
#define _DALI_COMMON_H_

#define	DALI_NONE		0
#define	DALI_MASTER		1
#define	DALI_SLAVE		2

#define	DALI_MAX_REQ		32			// Request queue length
#define	DALI_MAX_RESP		4			// Max number of responders 
#define	DALI_TX_BUF_SIZE	4			// Bytes	
#define	DALI_RX_BUF_SIZE	34			// Chirps: (2 bytes * 8 bit + 1 start bit) * 2 chirps = 34
#define	DALI_REQ_TIMEOUT	100			// Wait for response this amount of ms

// DALI commands
#define DALI_BROADCAST_LIGHT		0b11111110
#define DALI_BROADCAST_CMD		0b11111111
#define DALI_CMD_ON_DP			0b11111110
#define DALI_CMD_OFF_DP			0b00000000
#define DALI_CMD_OFF_C			0b00000000
#define DALI_CMD_ON_C			0b00000101
#define	DALI_CMD_RESET			0b00100000		// Reset parameters to defaults 
#define	DALI_CMD_ADD_GROUP_0		0b01100000
#define	DALI_CMD_ADD_GROUP_1		0b01100001
//...
#define	DALI_CMD_REMOVE_GROUP_0		0b01110000
#define	DALI_CMD_REMOVE_GROUP_1		0b01110001
//...
#define DALI_REQ_STATUS			0b10010000
#define DALI_REQ_BALLAST		0b10010001
#define DALI_REQ_FAILURE		0b10010010
#define DALI_REQ_POWER_ON		0b10010011
#define DALI_REQ_LIMIT_ERROR		0b10010100
#define DALI_REQ_RESET_STATUS		0b10010101
#define DALI_REQ_MISSING_SHORT		0b10010110
#define DALI_REQ_VERSION		0b10010111
#define DALI_REQ_DTR			0b10011000
#define	DALI_CMD_TERMINATE		0b10100001		// second byte mustbe zero
#define DALI_REQ_CUR_LIGHT_LEVEL	0b10100000
#define DALI_REQ_MAX_LIGHT_LEVEL	0b10100001
#define DALI_REQ_PING			0b10100001
#define	DALI_CMD_INITIALIZE		0b10100101
#define	DALI_CMD_RANDOMIZE		0b10100111	
#define	DALI_REQ_COMPARE		0b10101001
#define	DALI_CMD_WITHDRAW		0b10101011
#define	DALI_CMD_SEARCH_HB		0b10110001
#define	DALI_CMD_SEARCH_MB		0b10110011
#define	DALI_CMD_SEARCH_LB		0b10110100
#define	DALI_CMD_SET_SHORT		0b10110111		// (addr << 1) | 1
#define	DALI_REQ_GROUP_MAP1		0b11000000	// Request bit pattern of groups (0-7) 
#define	DALI_REQ_GROUP_MAP2		0b11000001	// Request bit pattern of groups (8-15) 
#define	DALI_REQ_HB			0b11000010	
#define	DALI_REQ_MB			0b11000011	
#define	DALI_REQ_LB			0b11000100	



typedef struct _DALI_REQUEST {
	int msg_req_complete;
	int msg_req_timeout;
	char txbuf[DALI_TX_BUF_SIZE];
	char rxbuf[DALI_TX_BUF_SIZE];
	int need_response;
	int delay;
	int timeout;
	int txlen;
	int rxlen;
} DALI_REQUEST;


typedef struct _DALI_RESPONSE {
        int msg_response;
        char txbuf[DALI_TX_BUF_SIZE];
        char rxbuf[DALI_TX_BUF_SIZE];
        int txlen;
        int rxlen;
	int cmd;
} DALI_RESPONSE;


#define DALI_TRANSMIT_CMD(dali_req, cmd1, cmd2, _delay, _msg_complete) \
        if(dali_req) {\
                (dali_req)->txbuf[0] = (cmd1); \
                (dali_req)->txbuf[1] = (cmd2); \
                (dali_req)->txlen = 2; \
                (dali_req)->rxlen = 0; \
                (dali_req)->delay = (_delay); \
                (dali_req)->need_response = 0; \
                (dali_req)->msg_req_complete = (_msg_complete); \
        } else {\
                /* svc_debug_print(str, sprintf(str, "DALI: out of memory (cmd1=0x%02X, cmd2=0x%02X)\r\n", cmd1, cmd2)); */ \
        }\


#define DALI_TRANSMIT_REQUEST(dali_req, cmd1, cmd2, _timeout, _msg_complete, _msg_timeout) \
        if(dali_req) {\
                (dali_req)->txbuf[0] = (cmd1); \
                (dali_req)->txbuf[1] = (cmd2); \
                (dali_req)->txlen = 2; \
                (dali_req)->rxlen = 0; \
                (dali_req)->need_response = 1; \
                (dali_req)->timeout = (_timeout); \
                (dali_req)->msg_req_complete = (_msg_complete); \
                (dali_req)->msg_req_timeout = (_msg_timeout); \
        } else {\
               /* svc_debug_print(str, sprintf(str, "DALI: out of memory (cmd1=0x%02X, cmd2=0x%02X)\r\n", cmd1, cmd2)); */ \
        }\

#define ABS(X) ((X) >= 0 ? (X) : -(X))

inline int dali_decode_manchester(char *dst_buf, char *chirps, char *times, int num_of_chirps, int polarity)
{
	int bytes_idx = 0;
	int cur_bit_val;
	int bytes_mask = 0x100; // one MSB bit will be skipped !!!

	for(int i = 0; i < num_of_chirps; ) {
		//print("SEARCH FOR SYNC\r\n");


		while(i < num_of_chirps) {
			char time_a = times[i];
			char time_b = times[i+1];

		//print("I: %d, C: %d, time_a: %d, time_b: %d\r\n", i, chirps[i], time_a, time_b);
			if((time_a >= 3 && time_a <= 5) && 
			    (time_b >= 3 && time_b <= 10) &&
			     chirps[i] == (1 ^ polarity))
				break; // synced 

			// Reset state
			bytes_idx = 0;
			bytes_mask = 0x100;

			// Continue searching
			i++;

		} 

		if(i == num_of_chirps)
			break; // END

		//print("SYNCED\r\n");

		cur_bit_val = 1;

		while(i < num_of_chirps) {

			char time_a = times[i];
			char time_b = times[i+1];

			i++;

			if(time_a < 3 || time_a > 10)
				break; // out of sync

			if(time_a > 5) { // Bit value flipped
				if(cur_bit_val)
					cur_bit_val = 0;
				else
					cur_bit_val = 1;
			} else { // same bit value folows
				if(time_b <= 5)
					i++;
			}


			//print("MANCHDEC: time_a = %d, time_b = %d, delta = %d, cur_bit_val = %d\r\n", time_a, time_b, ABS(time_b - time_a), cur_bit_val);

			if(cur_bit_val)
				dst_buf[bytes_idx] |= bytes_mask;
			else
				dst_buf[bytes_idx] &= ~bytes_mask;
			
			bytes_mask >>= 1;

			if(bytes_mask == 0) {
				bytes_idx++;
				bytes_mask = 0x80;
			}
		}

		i++;
	}

	//print("XXX: bytes_idx = %d\r\n", bytes_idx);
	//print("XXX: data: 0x%02X, 0x%02X\r\n", dst_buf[0], dst_buf[1]);

	return bytes_idx; // number of completely decoded octets 
}


#ifdef SVC_CLIENT
// DALI PORT1 stuff 
int svc_dali1_enqueue_request(DALI_REQUEST* dali_req);
int svc_dali1_register_responder(DALI_RESPONSE* dali_resp);
int svc_dali1_unregister_responder(DALI_RESPONSE* dali_resp);
int svc_dali1_submit_response(DALI_RESPONSE* dali_resp);

// DALI PORT2 stuff 
int svc_dali2_enqueue_request(DALI_REQUEST* dali_req);
int svc_dali2_register_responder(DALI_RESPONSE* dali_resp);
int svc_dali2_unregister_responder(DALI_RESPONSE* dali_resp);
int svc_dali2_submit_response(DALI_RESPONSE* dali_resp);
#endif //SVC_CLIENT
#endif //_DALI_H_


#ifdef SVC_CLIENT_IMPL

#ifndef _DALI_H_IMPL_
#define _DALI_H_IMPL_

#include "svc.h"
// DALI PORT1 operations
__attribute__ ((noinline)) int svc_dali1_enqueue_request(DALI_REQUEST* dali_req)
{
        svc(SVC_DALI1_ENQUEUE_REQUEST);
}

__attribute__ ((noinline)) int svc_dali1_register_responder(DALI_RESPONSE* dali_resp)
{
        svc(SVC_DALI1_REGISTER_RESPONDER);
}

__attribute__ ((noinline)) int svc_dali1_unregister_responder(DALI_RESPONSE* dali_resp)
{
        svc(SVC_DALI1_UNREGISTER_RESPONDER);
}

__attribute__ ((noinline)) int svc_dali1_submit_response(DALI_RESPONSE* dali_resp)
{
        svc(SVC_DALI1_SUBMIT_RESPONSE);
}

// DALI PORT1 operations
__attribute__ ((noinline)) int svc_dali2_enqueue_request(DALI_REQUEST* dali_req)
{
        svc(SVC_DALI2_ENQUEUE_REQUEST);
}

__attribute__ ((noinline)) int svc_dali2_register_responder(DALI_RESPONSE* dali_resp)
{
        svc(SVC_DALI2_REGISTER_RESPONDER);
}

__attribute__ ((noinline)) int svc_dali2_unregister_responder(DALI_RESPONSE* dali_resp)
{
        svc(SVC_DALI2_UNREGISTER_RESPONDER);
}

__attribute__ ((noinline)) int svc_dali2_submit_response(DALI_RESPONSE* dali_resp)
{
        svc(SVC_DALI2_SUBMIT_RESPONSE);
}

#endif //_DALI_H_IMPL_
#endif //SVC_CLIENT_IMPL

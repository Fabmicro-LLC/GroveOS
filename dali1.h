#ifndef __DALI1_H__
#define __DALI1_H__

#include "dali_common.h"

void dali1_init(void);
void dali1_transmit_one_bit(void);
int dali1_tx(char *buf, int len);
void dali1_rx(void); // read serial buffers input
void dali1_msg(int msg, int p1, int p2); // process modbus related events
int dali1_enqueue_request(DALI_REQUEST* modbus_req);
int dali1_dequeue_range(int msg_start, int msg_end);
void dali1_check_queue(void);
int dali1_register_responder(DALI_RESPONSE* resp);
int dali1_unregister_responder(DALI_RESPONSE* resp);
int dali1_unregister_range(int msg_start, int msg_end);
int dali1_submit_response(DALI_RESPONSE* resp);


#endif


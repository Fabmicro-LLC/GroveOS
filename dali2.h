#ifndef __DALI2_H__
#define __DALI2_H__

void dali2_rx(void); // read serial buffers input
void dali2_msg(int msg, int p1, int p2); // process modbus related events
int dali2_enqueue_request(DALI_REQUEST* modbus_req);
int dali2_dequeue_range(int msg_start, int msg_end);
void dali2_check_queue(void);
int dali2_register_responder(DALI_RESPONSE* resp);
int dali2_unregister_responder(DALI_RESPONSE* resp);
int dali2_unregister_range(int msg_start, int msg_end);
int dali2_submit_response(DALI_RESPONSE* resp);


#endif

#ifndef __MODBUS_EXT2_H__
#define __MODBUS_EXT2_H__

void modbus_rx_ext2(void); // read serial buffers input
void modbus_msg_ext2(int msg, int p1, int p2); // process modbus related events
int modbus_enqueue_request_ext2(MODBUS_REQUEST* modbus_req);
int modbus_dequeue_range_ext2(int msg_start, int msg_end);
void modbus_check_queue_ext2(void);
int modbus_register_responder_ext2(MODBUS_RESPONSE* resp);
int modbus_unregister_responder_ext2(MODBUS_RESPONSE* resp);
int modbus_unregister_range_ext2(int msg_start, int msg_end);
int modbus_submit_response_ext2(MODBUS_RESPONSE* resp);


#endif


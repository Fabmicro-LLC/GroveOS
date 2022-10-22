/*
	GroveOS is a tiny single-threaded operating system for ARM Cortex-M4F based microcontrollers

	Written by Ruslan Zalata and Evgeny Korolenko
	
	Copyright (c) 2016-2022, Fabmicro, LLC., Tyumen, Russia.
	All rights reserved.

	Email: info@fabmicro.ru

	SPDX-License-Identifier: BSD-2-Clause

*/
	
#include "msg.h"
#include "utils.h"

#define MSGSET_SIZE             100
#define MAIN_QUEUE_MAX_SIZE     100

int main_queue_enabled = 0;

static uint8_t msgset[MSGSET_SIZE];
static MSG main_queue_array[MAIN_QUEUE_MAX_SIZE];
static MSG* main_queue[MAIN_QUEUE_MAX_SIZE];
static int main_queue_head;
static int main_queue_tail;


static void print_stat();

int main_queue_enqueue(MSG *msg);
MSG* main_queue_dequeue();
int main_queue_isempty();
int main_queue_isfull();
MSG* main_queue_get_head();
MSG* main_queue_get_tail();
void main_queue_clear();


void main_queue_init() {
	main_queue_tail = main_queue_head = 0;
	main_queue_changed = 0;
	main_queue_wait_counter = 0;
	memset(msgset, 0, sizeof(uint8_t)*MSGSET_SIZE);
	error_main_queue_isfull = 0;
	main_queue_enabled = 1;
}


int main_queue_enqueue(MSG* msg) {
        if(main_queue_isfull()) {
                return -1;
        }

        main_queue[main_queue_tail % MAIN_QUEUE_MAX_SIZE] = msg;
        main_queue_tail++;
        main_queue_changed = 1;
        return 0;
}

MSG* main_queue_dequeue() {
        if(main_queue_isempty()) {
                return NULL;
        } else {
                MSG* result=main_queue[main_queue_head % MAIN_QUEUE_MAX_SIZE];
                main_queue_head++;
                main_queue_changed = 1;
                return result;
        }
}

void main_queue_clear() {
        main_queue_head = main_queue_tail = 0;
        main_queue_changed =1;
}

MSG* main_queue_get_head() {
        if(main_queue_isempty()) {
                return NULL;
        } else {
                return &main_queue_array[main_queue_head % MAIN_QUEUE_MAX_SIZE];
        }
}

MSG* main_queue_get_tail() {
        if(main_queue_isfull()) {
                return NULL;
        } else {
                return &main_queue_array[main_queue_tail % MAIN_QUEUE_MAX_SIZE];
        }
}
int main_queue_isempty() {
        return (main_queue_head == main_queue_tail);
}
int main_queue_isfull() {
        return ((main_queue_tail - MAIN_QUEUE_MAX_SIZE) == main_queue_head);
}

int main_queue_len() {
	return (main_queue_tail - main_queue_head);
}


int PostMessage(int message,  int unique, int p1, int p2) {

	//print("PostMessage main_queue_tail = %d, main_queue_head = %d\n", main_queue_tail, main_queue_head);

	if(!main_queue_enabled)
		return -3;

	 __disable_irq();
	int ret = PostMessageIRQ(message, unique, p1, p2);
	 __enable_irq();

	return ret;
	
}

int PostMessageIRQ(int message,  int unique, int p1, int p2) {

	//print("PostMessage main_queue_tail = %d, main_queue_head = %d\n", main_queue_tail, main_queue_head);

	if(!main_queue_enabled)
		return -3;

	if(unique && message>=0 && message<MSGSET_SIZE && msgset[message]) {
		//already in queue	
	} else {
		MSG* msg=main_queue_get_tail();
		if(msg) {
			msg->message = message;
			msg->p1 = p1;
			msg->p2 = p2;
			if(message>=0 && message<MSGSET_SIZE) msgset[message] = 1;
			return main_queue_enqueue(msg);
		} else {
			error_main_queue_isfull++;
			//print("PostMessage queue is full, msg=%d, p1=%d, p2=%d\r\n", message, p1, p2);
			//print_stat();
		}
	}

	return -1;;
	
}

int GetMessage(MSG* msg) {
	if(main_queue_isempty()) return -2;

	 __disable_irq();
	MSG *m=main_queue_dequeue();

	if(m==NULL)  {
		 __enable_irq();
		return -1;
	}

	msg->message = m->message;
	msg->p1 = m->p1;
	msg->p2 = m->p2;

	if(m->message>=0 && m->message<MSGSET_SIZE) {
		msgset[m->message] = 0;
	}
	 __enable_irq();

	return 0;
}

void print_stat() {
	int stat_keys[5]={0};
        int stat_values[5]={0};
        for(int i=main_queue_head; i<main_queue_tail; i++) {
        	int msg = main_queue[(i % MAIN_QUEUE_MAX_SIZE) ]->message;
                for(int j=0; j<5; j++) {
                	if(stat_keys[j]==0 || stat_keys[j] == msg) {
                        	stat_keys[j] = msg;
                                stat_values[j]++;
                                break;
                        }
                }
         }

         for(int j=0; j<5; j++) {
         	if(stat_keys[j] == 0) break;
                //print("MAIN_QUEUE msg: %d count: %d\r\n",stat_keys[j],stat_values[j]);
         }
}

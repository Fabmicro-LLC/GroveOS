#include "softtimer.h"
#include "utils.h"
#include "msg.h"


SOFTTIMER softtimers[SOFTTIMER_MAX_TIMERS];

void softtimer_init(void)
{
	memset(&softtimers, 0, sizeof(softtimers));
}

void softtimer_check(void)
{
	for(int i = 0; i < SOFTTIMER_MAX_TIMERS; i++) {
		if(softtimers[i].id !=0) {
			if(--softtimers[i].count_down == 0) {
				PostMessageIRQ(softtimers[i].id, 1, softtimers[i].p1, softtimers[i].p2);				
				softtimers[i].id = 0;
			}
		}
	}
}

void softtimer_clear_range(int from, int to)
{
	for(int i = 0; i < SOFTTIMER_MAX_TIMERS; i++) {
		if(softtimers[i].id >= from && softtimers[i].id < to) {
			softtimers[i].id = 0;
		}
	}
}

int softtimer_run_timer(int id, unsigned int timeout, int p1, int p2)
{
	int free_slot = -1;
	for(int i = 0; i < SOFTTIMER_MAX_TIMERS; i++) {
		if(softtimers[i].id == 0) {
			free_slot = i;
		}
		if(softtimers[i].id == id) { // update existing timer
			softtimers[i].count_down = timeout;
			softtimers[i].p1 = p1;
			softtimers[i].p2 = p2;
			return i;
		}
	}

	if(free_slot != -1) { // add new timer
		softtimers[free_slot].id = id;
		softtimers[free_slot].count_down = timeout;
		softtimers[free_slot].p1 = p1;
		softtimers[free_slot].p2 = p2;
		return free_slot;
	}

	return -1; // no more space for timers
}


int softtimer_stop_timer(int id)
{
	for(int i = 0; i < SOFTTIMER_MAX_TIMERS; i++) {
		if(softtimers[i].id == id) {
			softtimers[i].id = 0 ;
			return i;
		}
	}

	return -1;
}


#ifndef __SOFTTIMER_H__
#define __SOFTTIMER_H__

#define	SOFTTIMER_MAX_TIMERS	128	

typedef struct _SOFTTIMER {
	int		id;
	unsigned int	count_down;
	int		p1;
	int		p2;

} SOFTTIMER;

extern SOFTTIMER softtimer_timers[];

void softtimer_init(void);
void softtimer_check(void);
int softtimer_run_timer(int id, unsigned int timeout, int p1, int p2);
int softtimer_stop_timer(int id);
void softtimer_clear_range(int from, int to);

#endif
#ifndef __R_TIMER_H__
#define __R_TIMER_H__

#include <time.h>
#include <sys/time.h>

#include "common.h"

enum R_TIMER_TYPE {
	R_TIMER_ONCE = 0,
	R_TIMER_LOOP,
};

struct r_timer {
	u_int32     msg_id;
	u_int32     tm_type;
	time_t      s_start;
	suseconds_t  ms_start;
	u_int32     tm_val;  /* in micro second */
	timer_func  func;
	void* data;
};

typedef struct r_queue timer_queue;

int r_timer_init();
void *timer_scan_thread(void *);
int set_timer(u_int32 msg_id, u_int32 , u_int32 msec, timer_func f, void *data);

#endif


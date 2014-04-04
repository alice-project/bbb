#ifndef __R_MESSAGE_H__
#define __R_MESSAGE_H__

#include "common.h"
#include "r_queue.h"

/* Message Priority 0~3:
	0 : the highest priority;
	3 : the lowest priority;
 */
enum R_MESSAGE_PRIORITY {
	R_MESSAGE_PRIORITY_0 = 0,
	R_MESSAGE_PRIORITY_1,
	R_MESSAGE_PRIORITY_2,
	R_MESSAGE_PRIORITY_3,
	R_MESSAGE_PRIORITY_HIGHEST = R_MESSAGE_PRIORITY_3,
};

typedef struct r_queue message_queue;

typedef struct r_queue message_node;

struct r_msg {
	u_int32 msg_id;
	void *data;
	msg_func func;
};

message_node *r_get_message();

int r_message_init();

#endif


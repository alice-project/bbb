#include "common.h"
#include "r_message.h"
#include "r_event.h"

message_queue *r_msg_head[R_MESSAGE_PRIORITY_HIGHEST];

int r_message_init()
{
	int queue;
	for(queue = R_MESSAGE_PRIORITY_0;queue < R_MESSAGE_PRIORITY_HIGHEST + 1;queue++)
	{
		queue_init(r_msg_head[queue]);
	}

	return 0;
}

message_node* r_get_message()
{
	int queue;
	message_node *node;

	for(queue = R_MESSAGE_PRIORITY_0;queue < R_MESSAGE_PRIORITY_HIGHEST + 1;queue++)
	{
		if(!queue_is_empty(r_msg_head[queue]))
		{
			node = queue_remove_node(&r_msg_head[queue]);
			return node;
		}
	}

	return NULL;
}

int r_send_message(u_int32 msg_id, u_int32 priority, void *body)
{
	message_node *node;

	if((priority < R_MESSAGE_PRIORITY_0) || (priority > R_MESSAGE_PRIORITY_HIGHEST))
	{
		return -1;
	}

	node = (message_node *)malloc(sizeof(message_node));
	if(node == NULL)
	{
		return -1;
	}
	node->data = body;

	queue_append(r_msg_head[priority], node);

	return 0;
}




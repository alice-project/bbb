#include <stdio.h>
#include <unistd.h>
#include <syslog.h>

#include "r_timer.h"
#include "r_queue.h"
#include "r_message.h"

timer_queue *r_tm_head;

int r_timer_init()
{
    queue_init(r_tm_head);

    return 0;
}

int insert_timer(timer_queue *node)
{
	struct r_queue *queue;
	struct r_timer *t1, *t2;
	
	queue = r_tm_head;
	while(queue != NULL)
	{
		t1 = (struct r_timer *)queue->data;
		t2 = (struct r_timer *)node->data;
		if((t1->s_start + t1->ms_start) > (t2->s_start + t2->ms_start))
		{
			break;
		}
		queue = queue->next;
	}
	queue_prepend(r_tm_head, node);
	if(queue == r_tm_head)
	{
		r_tm_head = node;  /* insert at the beginning */
	}
	return 0;
}

int free_timer(timer_queue *node)
{
	if(node)
	{
		if(node->data)
			free(node->data);

		free(node);
	}
}

int set_timer(u_int32 msg_id, u_int32 type, u_int32 msec, timer_func f, void *data)
{
	struct r_timer *tm, *t;
	struct r_queue *node;
	struct timeval current;

	if((type != R_TIMER_ONCE) && (type != R_TIMER_LOOP))
	{
		return -1;
	}

	tm = (struct r_timer *)malloc(sizeof(struct r_timer));
	if(tm == NULL)
	{
		syslog(LOG_EMERG, "Timer malloc failed (msg_id = 0x%lx)\n", msg_id);
		return -1;
	}

	gettimeofday(&current, NULL);
	tm->s_start  = current.tv_sec;
	tm->ms_start = current.tv_usec;
	tm->tm_val   = msec*1000;
	tm->tm_type  = type;
	tm->func     = f;
	tm->data = data;

	node = (struct r_queue *)malloc(sizeof(struct r_queue));
	if(node == NULL)
	{
		syslog(LOG_EMERG, "Timer malloc failed (msg_id = 0x%lx)\n", msg_id);
		free(tm);
		return -1;
	}
	node->data = tm;

	insert_timer(node);

	return 0;
}


void *timer_scan_thread(void *data)
{
	struct r_queue *node;
	struct r_timer *tm, *t;
	struct timeval current;

	while(1)
	{
		if(r_tm_head == NULL)
		{
			usleep(10);
			continue;
		}
		gettimeofday(&current, NULL);
		t = (struct r_timer *)r_tm_head->data;
		while((r_tm_head != NULL) && ((t->s_start + (t->ms_start + t->tm_val)/1000000) < current.tv_sec + current.tv_usec/1000000))
		{
			node = queue_remove_node(&r_tm_head);
			tm = (struct r_timer *)(node->data);
			if(tm->func != NULL)
			{
				tm->func(tm->data);
				if(tm->tm_type == R_TIMER_LOOP)
				{
					tm->s_start = current.tv_sec;
					tm->ms_start = current.tv_usec;
					insert_timer(node);
				}
				else
				{
					free_timer(node);
				}
			}
			usleep(1);
			gettimeofday(&current, NULL);
		}
		usleep(10);
	}
	return NULL;
}



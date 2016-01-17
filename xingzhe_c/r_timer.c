#include <stdio.h>
#include <unistd.h>
#include <syslog.h>

#include "r_list.h"
#include "r_message.h"
#include "r_timer.h"

struct r_timer_list r_tm_head;

int r_timer_init()
{
    INIT_LIST_HEAD(&r_tm_head.list);

    return 0;
}

int insert_timer(struct r_timer_list *node)
{
    struct r_list       *queue;
    struct r_timer_list *entry=NULL;

    if(list_is_empty(&r_tm_head.list))
    {
        list_add(&node->list, &r_tm_head.list);
        return 0;
    }

    list_for_each(queue, &r_tm_head.list) {    
        entry = list_entry(queue, struct r_timer_list, list);
		if((entry->data.s_start + (entry->data.ms_start + entry->data.tm_val)/1000000) > 
           (node->data.s_start + (node->data.ms_start + node->data.tm_val)/1000000))
		{
			break;
		}
    }
    if(entry)
        list_add(&node->list, &entry->list);

	return 0;
}

int free_timer(struct r_timer_list *node)
{
	if(node)
	{
		if(node->data.data)
			free(node->data.data);

		free(node);
	}
}

int set_timer(u_int32 msg_id, u_int32 type, u_int32 msec, timer_func f, void *data)
{
	struct r_timer_list *node;
	struct timeval current;

	if((type != R_TIMER_ONCE) && (type != R_TIMER_LOOP))
	{
		return -1;
	}

	node = (struct r_timer_list *)malloc(sizeof(struct r_timer_list));
	if(node == NULL)
	{
		syslog(LOG_EMERG, "Timer malloc failed (msg_id = 0x%x)\n", msg_id);
		return -1;
	}
	gettimeofday(&current, NULL);
	node->data.s_start  = current.tv_sec;
	node->data.ms_start = current.tv_usec;
	node->data.tm_val   = msec*1000;
	node->data.tm_type  = type;
	node->data.func     = f;
	node->data.data = data;

	insert_timer(node);

	return 0;
}


void *timer_scan_thread(void *data)
{
	struct r_list *node;
	struct r_list *header;
	struct r_timer *tm;
    struct r_timer_list *entry;
	struct timeval current;

	while(1)
	{
		if(list_is_empty(&r_tm_head.list))
		{
			usleep(10);
			continue;
		}
        header = r_tm_head.list.next;
		gettimeofday(&current, NULL);
        entry = list_entry(header, struct r_timer_list, list);
		if(((entry->data.s_start + (entry->data.ms_start + entry->data.tm_val)/1000000) < current.tv_sec + current.tv_usec/1000000))
		{
			node = list_remove_node(header);
            entry = list_entry(node, struct r_timer_list, list);
			tm = (struct r_timer *)(&entry->data);
			if(tm->func != NULL)
			{
				tm->func(tm->data);
				if(tm->tm_type == R_TIMER_LOOP)
				{
					tm->s_start = current.tv_sec;
					tm->ms_start = current.tv_usec;
					insert_timer(entry);
				}
				else
				{
					free_timer(entry);
				}
			}
		}
		usleep(10);
	}
	return NULL;
}

void r_timer_safe_exit()
{
	struct r_list *node;
    struct r_timer_list *entry;

    while(!list_is_empty(&r_tm_head.list)) {
        node = r_tm_head.list.next;
        list_del(node);
        entry = list_entry(node, struct r_timer_list, list);
		if(entry->data.data)
		{
            free(entry->data.data);
            free(node);
		}
    }
}



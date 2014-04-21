#ifndef __R_QUEUE_H__
#define __R_QUEUE_H__

#include <stdlib.h>

#include "common.h"

struct r_queue {
	struct r_queue *next;
	void   *data;
};

static inline void queue_init(struct r_queue *queue)
{
	queue = NULL;
}

static inline int queue_is_empty(struct r_queue *queue)
{
	return queue == NULL;
}

static inline void queue_append(struct r_queue *queue, struct r_queue *new_link)
{
	queue->next = new_link;
	new_link->next = NULL;
}

static inline void queue_prepend(struct r_queue *queue, struct r_queue *new_link)
{
	new_link->next = queue;
}

static inline struct r_queue* queue_remove_node(struct r_queue **link)
{
	struct r_queue *node;

	if(queue_is_empty(*link))
		return NULL;

	node = *link;
	*link = (*link)->next;

	return node;
}

#endif


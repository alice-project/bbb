#ifndef __R_LIST_H__
#define __R_LIST_H__

#include <stdlib.h>

#include "common.h"

struct r_list {
	struct r_list *prev, *next;
	void   *data;
};

static inline void list_init(struct r_list *list)
{
	list->next = list;
	list->prev = list;
}

static inline int list_is_empty(struct r_list *list)
{
	return list->next == list;
}

static inline void list_insert(struct r_list *link, struct r_list *new_link)
{
	new_link->prev		= link->prev;
	new_link->next		= link;
	new_link->prev->next	= new_link;
	new_link->next->prev	= new_link;
}

static inline void list_append(struct r_list *list, struct r_list *new_link)
{
	list_insert((struct r_list *)list, new_link);
}

static inline void list_prepend(struct r_list *list, struct r_list *new_link)
{
	list_insert(list->next, new_link);
}

static inline struct r_list* list_remove_node(struct r_list **link)
{
	struct r_list *node;

	if(list_is_empty(*link))
		return NULL;

	node = *link;

	node->prev->next = node->next;
	node->next->prev = node->prev;
	
	*link = (*link)->next;

	return node;
}

static inline void list_delete_node(struct r_list *link)
{
	if(link->data)
		free(link->data);
	link->prev->next = link->next;
	link->next->prev = link->prev;
}

#endif


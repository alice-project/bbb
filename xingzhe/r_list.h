#ifndef __R_LIST_H__
#define __R_LIST_H__


#include <stdlib.h>
#include <unistd.h>

#include "common.h"

struct r_list {
	struct r_list *prev, *next;
};


#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
        struct r_list name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct r_list *list)
{
    list->next = list;
    list->prev = list;
}

static inline int list_is_empty(struct r_list *list)
{
	return list->next == list;
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int list_is_last(const struct r_list *list,
                                const struct r_list *head)
{
    return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const struct r_list *head)
{
    return head->next == head;
}

static inline int list_is_singular(const struct r_list *head)
{
    return !list_empty(head) && (head->next == head->prev);
}

static inline void __list_add(struct r_list *new_node,
                              struct r_list *prev,
                              struct r_list *next)
{
    next->prev = new_node;
    new_node->next = next;
    new_node->prev = prev;
    prev->next = new_node;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add(struct r_list *new_node, struct r_list *head)
{
    __list_add(new_node, head, head->next);
}


/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct r_list *new_node, struct r_list *head)
{
    __list_add(new_node, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct r_list * prev, struct r_list * next)
{
    next->prev = prev;
    prev->next = next;
}

static inline void list_del(struct r_list *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

static inline struct r_list *list_remove_node(struct r_list *list)
{
    struct r_list *node;

    if(list_empty(list))
        return NULL;
    
    node = list;
    list_del(list);
    return node;
}

static inline void list_replace(struct r_list *old,
                               struct r_list *new_node)
{
    new_node->next = old->next;
    new_node->next->prev = new_node;
    new_node->prev = old->prev;
    new_node->prev->next = new_node;
}

/**
* list_move - delete from one list and add as another's head
* @list: the entry to move
 * @head: the head that will precede our entry
 */
static inline void list_move(struct r_list *list, struct r_list *head)
{
    __list_del(list->prev, list->next);
    list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: the entry to move
 * @head: the head that will follow our entry
 */
static inline void list_move_tail(struct r_list *list,
                                  struct r_list *head)
{
     __list_del(list->prev, list->next);
     list_add_tail(list, head);
}

static inline void list_free_all(struct r_list *head)
{
    struct r_list *node;
    if(list_is_empty(head))
        return;

    node = head->next;
    list_del(head);
    free(head);
    
    list_free_all(node);
}


/**
 * list_entry - get the struct for this entry
 * @ptr:        the &struct r_list pointer.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the list_struct within the struct.
 */
/*#define container_of(ptr, type, member) ( { \
const typeof(((type *)0)->member ) *__mptr = (ptr); \
(type *)( (char *)__mptr - offsetof(type,member) ); } )


#define list_entry(ptr, type, member) \
        container_of(ptr, type, member)
*/
#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))
/**
 * list_first_entry - get the first element from a list
 * @ptr:        the list head to take the element from.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
        list_entry((ptr)->next, type, member)

/**
 * list_for_each        -       iterate over a list
 * @pos:        the &struct r_list to use as a loop cursor.
 * @head:       the head for your list.
 */
#define list_for_each(pos, head) \
        for (pos = (head)->next; (pos->next), pos != (head); \
                pos = pos->next)

/**
 * __list_for_each      -       iterate over a list
 * @pos:        the &struct r_list to use as a loop cursor.
 * @head:       the head for your list.
 *
 * This variant differs from list_for_each() in that it's the
 * simplest possible list iteration code, no prefetching is done.
 * Use this for code that knows the list to be very short (empty
 * or  entry) most of the time.
 */
#define __list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev   -       iterate over a list backwards
 * @pos:        the &struct r_list to use as a loop cursor.
 * @head:       the head for your list.
 */
#define list_for_each_prev(pos, head) \
        for (pos = (head)->prev; (pos->prev), pos != (head); \
                pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal of list entry
 * @pos:        the &struct r_list to use as a loop cursor.
 * @n:          another &struct r_list to use as temporary storage
 * @head:       the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); \
                pos = n, n = pos->next)

/**
 * list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @pos:        the &struct r_list to use as a loop cursor.
 * @n:          another &struct r_list to use as temporary storage
 * @head:       the head for your list.
 */
#define list_for_each_prev_safe(pos, n, head) \
        for (pos = (head)->prev, n = pos->prev; \
             (pos->prev), pos != (head); \
             pos = n, n = pos->prev)
 
/**
 * list_for_each_entry  -       iterate over list of given type
 * @pos:        the type * to use as a loop cursor.
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member)                          \
        for (pos = list_entry((head)->next, typeof(*pos), member);      \
             (pos->member.next), &pos->member != (head);        \
             pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_reverse - iterate backwards over list of given type.
 * @pos:        the type * to use as a loop cursor.
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 */
#define list_for_each_entry_reverse(pos, head, member)                  \
        for (pos = list_entry((head)->prev, typeof(*pos), member);      \
             (pos->member.prev), &pos->member != (head);        \
             pos = list_entry(pos->member.prev, typeof(*pos), member))
 
/**
 * list_prepare_entry - prepare a pos entry for use in list_for_each_entry_continue()
 * @pos:        the type * to use as a start point
 * @head:       the head of the list
 * @member:     the name of the list_struct within the struct.
 *
 * Prepares a pos entry for use as a start point in list_for_each_entry_continue().
 */
#define list_prepare_entry(pos, head, member) \
       ((pos) ? : list_entry(head, typeof(*pos), member))
 
/**
 * list_for_each_entry_continue - continue iteration over list of given type
 * @pos:        the type * to use as a loop cursor.
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define list_for_each_entry_continue(pos, head, member)                 \
        for (pos = list_entry(pos->member.next, typeof(*pos), member);  \
             (pos->member.next), &pos->member != (head);        \
             pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @pos:        the type * to use as a loop cursor.
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define list_for_each_entry_continue_reverse(pos, head, member)         \
        for (pos = list_entry(pos->member.prev, typeof(*pos), member);  \
             (pos->member.prev), &pos->member != (head);        \
             pos = list_entry(pos->member.prev, typeof(*pos), member))

/**
 * list_for_each_entry_from - iterate over list of given type from the current point
 * @pos:        the type * to use as a loop cursor.
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define list_for_each_entry_from(pos, head, member)                     \
        for (; (pos->member.next), &pos->member != (head);      \
              pos = list_entry(pos->member.next, typeof(*pos), member))
 
/**
 * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:        the type * to use as a loop cursor.
 * @n:          another type * to use as temporary storage
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 */
#define list_for_each_entry_safe(pos, n, head, member)                  \
         for (pos = list_entry((head)->next, typeof(*pos), member),      \
             n = list_entry(pos->member.next, typeof(*pos), member); \
             &pos->member != (head);                                    \
             pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_continue
 * @pos:        the type * to use as a loop cursor.
 * @n:          another type * to use as temporary storage
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define list_for_each_entry_safe_continue(pos, n, head, member)                 \
        for (pos = list_entry(pos->member.next, typeof(*pos), member),          \
            n = list_entry(pos->member.next, typeof(*pos), member);         \
            &pos->member != (head);                                            \
            pos = n, n = list_entry(n->member.next, typeof(*n), member))
 
/**
 * list_for_each_entry_safe_from
 * @pos:        the type * to use as a loop cursor.
 * @n:          another type * to use as temporary storage
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define list_for_each_entry_safe_from(pos, n, head, member)                     \
         for (n = list_entry(pos->member.next, typeof(*pos), member);            \
         &pos->member != (head);                                            \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * list_for_each_entry_safe_reverse
 * @pos:        the type * to use as a loop cursor.
 * @n:          another type * to use as temporary storage
 * @head:       the head for your list.
 * @member:     the name of the list_struct within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define list_for_each_entry_safe_reverse(pos, n, head, member)          \
        for (pos = list_entry((head)->prev, typeof(*pos), member),      \
             n = list_entry(pos->member.prev, typeof(*pos), member); \
             &pos->member != (head);                                    \
             pos = n, n = list_entry(n->member.prev, typeof(*n), member))



#endif


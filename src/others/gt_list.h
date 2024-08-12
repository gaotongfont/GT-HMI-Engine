/**
 * @file gt_list.h
 * @brief double linked list
 */
#ifndef _GT_LIST_H_
#define _GT_LIST_H_

#include "gt_defs.h"
#ifdef __cplusplus
extern "C" {
#endif

#if 0
    #define _gt_always_inline
#else
	#define _gt_always_inline inline
#endif

/**
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */

struct _gt_list_head {
	struct _gt_list_head *next, *prev;
};

#define _GT_LIST_HEAD_INIT(name) { &(name), &(name) }

#define _GT_LIST_HEAD_DEF(name) \
    struct _gt_list_head name = _GT_LIST_HEAD_INIT(name)

static _gt_always_inline void _GT_INIT_LIST_HEAD(struct _gt_list_head *list)
{
	list->next = list;
	list->prev = list;
}

/**
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static _gt_always_inline void __gt_list_add(struct _gt_list_head *newl,
                                       struct _gt_list_head *prev,
                                       struct _gt_list_head *next)
{
	next->prev = newl;
	newl->next = next;
	newl->prev = prev;
	prev->next = newl;
}

/**
 * _gt_list_add - add a new entry
 * @param new: new entry to be added
 * @param head list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static _gt_always_inline void _gt_list_add(struct _gt_list_head *newl, struct _gt_list_head *head)
{
	__gt_list_add(newl, head, head->next);
}


/**
 * _gt_list_add_tail - add a new entry
 * @param new: new entry to be added
 * @param head list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static _gt_always_inline void _gt_list_add_tail(struct _gt_list_head *newl, struct _gt_list_head *head)
{
	__gt_list_add(newl, head->prev, head);
}

/**
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static _gt_always_inline void __gt_list_del(struct _gt_list_head *prev, struct _gt_list_head *next)
{
	next->prev = prev;
	prev->next = next;
}

/**
 * _gt_list_del - deletes entry from list.
 * @param entry the element to delete from the list.
 * Note: _gt_list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static _gt_always_inline void __gt_list_del_entry(struct _gt_list_head *entry)
{
	__gt_list_del(entry->prev, entry->next);
}

static _gt_always_inline void _gt_list_del(struct _gt_list_head *entry)
{
	__gt_list_del(entry->prev, entry->next);
	entry->next = entry;
	entry->prev = entry;
}

/**
 * _gt_list_replace - replace old entry by new one
 * @old : the element to be replaced
 * @param new : the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static _gt_always_inline void _gt_list_replace(struct _gt_list_head *old, struct _gt_list_head *newl)
{
	newl->next = old->next;
	newl->next->prev = newl;
	newl->prev = old->prev;
	newl->prev->next = newl;
}

static _gt_always_inline void _gt_list_replace_init(struct _gt_list_head *old,
                                              struct _gt_list_head *newl)
{
	_gt_list_replace(old, newl);
	_GT_INIT_LIST_HEAD(old);
}

/**
 * _gt_list_del_init - deletes entry from list and reinitialize it.
 * @param entry the element to delete from the list.
 */
static _gt_always_inline void _gt_list_del_init(struct _gt_list_head *entry)
{
	__gt_list_del_entry(entry);
	_GT_INIT_LIST_HEAD(entry);
}

/**
 * _gt_list_move - delete from one list and add as another's head
 * @param list the entry to move
 * @param head the head that will precede our entry
 */
static _gt_always_inline void _gt_list_move(struct _gt_list_head *list, struct _gt_list_head *head)
{
	__gt_list_del_entry(list);
	_gt_list_add(list, head);
}

/**
 * _gt_list_move_tail - delete from one list and add as another's tail
 * @param list the entry to move
 * @param head the head that will follow our entry
 */
static _gt_always_inline void _gt_list_move_tail(struct _gt_list_head *list,
                                           struct _gt_list_head *head)
{
	__gt_list_del_entry(list);
	_gt_list_add_tail(list, head);
}

/**
 * _gt_list_is_last - tests whether @list is the last entry in list @head
 * @param list the entry to test
 * @param head the head of the list
 */
static _gt_always_inline int _gt_list_is_last(const struct _gt_list_head *list,
                                        const struct _gt_list_head *head)
{
	return list->next == head;
}

/**
 * _gt_list_empty - tests whether a list is empty
 * @param head the list to test.
 */
static _gt_always_inline int _gt_list_empty(const struct _gt_list_head *head)
{
	return head->next == head;
}

/**
 * _gt_list_empty_careful - tests whether a list is empty and not being modified
 * @param head the list to test
 *
 * Description:
 * tests whether a list is empty _and_ checks that no other CPU might be
 * in the process of modifying either member (next or prev)
 *
 * NOTE: using _gt_list_empty_careful() without synchronization
 * can only be safe if the only activity that can happen
 * to the list entry is _gt_list_del_init(). Eg. it cannot be used
 * if another CPU could re-_gt_list_add() it.
 */
static _gt_always_inline int _gt_list_empty_careful(const struct _gt_list_head *head)
{
	struct _gt_list_head *next = head->next;
	return (next == head) && (next == head->prev);
}

/**
 * _gt_list_rotate_left - rotate the list to the left
 * @param head the head of the list
 */
static _gt_always_inline void _gt_list_rotate_left(struct _gt_list_head *head)
{
	struct _gt_list_head *first;

	if (!_gt_list_empty(head)) {
		first = head->next;
		_gt_list_move_tail(first, head);
	}
}

/**
 * _gt_list_is_singular - tests whether a list has just one entry.
 * @param head the list to test.
 */
static _gt_always_inline int _gt_list_is_singular(const struct _gt_list_head *head)
{
	return !_gt_list_empty(head) && (head->next == head->prev);
}

static _gt_always_inline void __gt_list_cut_position(struct _gt_list_head *list,
                                                struct _gt_list_head *head,
                                                struct _gt_list_head *entry)
{
	struct _gt_list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

/**
 * _gt_list_cut_position - cut a list into two
 * @param list a new list to add all removed entries
 * @param head a list with entries
 * @param entry an entry within head, could be the head itself
 *  and if so we won't cut the list
 *
 * This helper moves the initial part of @head, up to and
 * including @entry, from @head to @list. You should
 * pass on @entry an element you know is on @head. @list
 * should be an empty list or a list you do not care about
 * losing its data.
 *
 */
static _gt_always_inline void _gt_list_cut_position(struct _gt_list_head *list,
                                              struct _gt_list_head *head,
                                              struct _gt_list_head *entry)
{
	if (_gt_list_empty(head))
		return;
	if (_gt_list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		_GT_INIT_LIST_HEAD(list);
	else
		__gt_list_cut_position(list, head, entry);
}

static _gt_always_inline void __gt_list_splice(const struct _gt_list_head *list,
                                          struct _gt_list_head *prev,
                                          struct _gt_list_head *next)
{
	struct _gt_list_head *first = list->next;
	struct _gt_list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

/**
 * _gt_list_splice - join two lists, this is designed for stacks
 * @param list the new list to add.
 * @param head the place to add it in the first list.
 */
static _gt_always_inline void _gt_list_splice(const struct _gt_list_head *list,
                                        struct _gt_list_head *head)
{
	if (!_gt_list_empty(list))
		__gt_list_splice(list, head, head->next);
}

/**
 * _gt_list_splice_tail - join two lists, each list being a queue
 * @param list the new list to add.
 * @param head the place to add it in the first list.
 */
static _gt_always_inline void _gt_list_splice_tail(struct _gt_list_head *list,
                                             struct _gt_list_head *head)
{
	if (!_gt_list_empty(list))
		__gt_list_splice(list, head->prev, head);
}

/**
 * _gt_list_splice_init - join two lists and reinitialise the emptied list.
 * @param list the new list to add.
 * @param head the place to add it in the first list.
 *
 * The list at @list is reinitialised
 */
static _gt_always_inline void _gt_list_splice_init(struct _gt_list_head *list,
                                             struct _gt_list_head *head)
{
	if (!_gt_list_empty(list)) {
		__gt_list_splice(list, head, head->next);
		_GT_INIT_LIST_HEAD(list);
	}
}

/**
 * _gt_list_splice_tail_init - join two lists and reinitialise the emptied list
 * @param list the new list to add.
 * @param head the place to add it in the first list.
 *
 * Each of the lists is a queue.
 * The list at @list is reinitialised
 */
static _gt_always_inline void _gt_list_splice_tail_init(struct _gt_list_head *list,
                                                  struct _gt_list_head *head)
{
	if (!_gt_list_empty(list)) {
		__gt_list_splice(list, head->prev, head);
		_GT_INIT_LIST_HEAD(list);
	}
}

/**
 * _gt_list_entry - get the struct for this entry
 * @param ptr    the &struct _gt_list_head pointer.
 * @param type   the type of the struct this is embedded in.
 * @param member the name of the _gt_list_head within the struct.
 */
#define _gt_list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * _gt_list_first_entry - get the first element from a list
 * @param ptr    the list head to take the element from.
 * @param type   the type of the struct this is embedded in.
 * @param member the name of the _gt_list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define _gt_list_first_entry(ptr, type, member) \
	_gt_list_entry((ptr)->next, type, member)

/**
 * _gt_list_last_entry - get the last element from a list
 * @param ptr    the list head to take the element from.
 * @param type   the type of the struct this is embedded in.
 * @param member the name of the _gt_list_head within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define _gt_list_last_entry(ptr, type, member) \
	_gt_list_entry((ptr)->prev, type, member)

/**
 * _gt_list_first_entry_or_null - get the first element from a list
 * @param ptr    the list head to take the element from.
 * @param type   the type of the struct this is embedded in.
 * @param member the name of the _gt_list_head within the struct.
 *
 * Note that if the list is empty, it returns NULL.
 */
#define _gt_list_first_entry_or_null(ptr, type, member) \
	(!_gt_list_empty(ptr) ? _gt_list_first_entry(ptr, type, member) : NULL)

/**
 * _gt_list_next_entry - get the next element in list
 * @param pos    the type * to cursor
 * @param member the name of the _gt_list_head within the struct.
 */
#define _gt_list_next_entry(pos, type_name, member) \
	_gt_list_entry((pos)->member.next, type_name, member)

/**
 * _gt_list_prev_entry - get the prev element in list
 * @param pos    the type * to cursor
 * @param member the name of the _gt_list_head within the struct.
 */
#define _gt_list_prev_entry(pos, type_name, member) \
	_gt_list_entry((pos)->member.prev, type_name, member)

/**
 * _gt_list_for_each    -   iterate over a list
 * @param pos    the &struct _gt_list_head to use as a loop cursor.
 * @param head   the head for your list.
 */
#define _gt_list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * _gt_list_for_each_prev   -   iterate over a list backwards
 * @param pos    the &struct _gt_list_head to use as a loop cursor.
 * @param head   the head for your list.
 */
#define _gt_list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * _gt_list_for_each_safe - iterate over a list safe against removal of list entry
 * @param pos    the &struct _gt_list_head to use as a loop cursor.
 * @param n      another &struct _gt_list_head to use as temporary storage
 * @param head   the head for your list.
 */
#define _gt_list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * _gt_list_for_each_prev_safe - iterate over a list backwards safe against removal of list entry
 * @param pos    the &struct _gt_list_head to use as a loop cursor.
 * @param n      another &struct _gt_list_head to use as temporary storage
 * @param head   the head for your list.
 */
#define _gt_list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
		 pos != (head); \
		 pos = n, n = pos->prev)

/**
 * _gt_list_for_each_entry  -   iterate over list of given type
 * @param pos    the type * to use as a loop cursor.
 * @param head   the head for your list.
 * @param member the name of the _gt_list_head within the struct.
 */
#define _gt_list_for_each_entry(pos, head, type_name, member)              \
	for (pos = _gt_list_first_entry(head, type_name, member);    \
		 &pos->member != (head);                    \
		 pos = _gt_list_next_entry(pos, type_name, member))

/**
 * _gt_list_for_each_entry_reverse - iterate backwards over list of given type.
 * @param pos    the type * to use as a loop cursor.
 * @param head   the head for your list.
 * @param member the name of the _gt_list_head within the struct.
 */
#define _gt_list_for_each_entry_reverse(pos, head, type_name, member)          \
	for (pos = _gt_list_last_entry(head, type_name, member);     \
	     &pos->member != (head);                    \
	     pos = _gt_list_prev_entry(pos, type_name, member))

/**
 * _gt_list_prepare_entry - prepare a pos entry for use in _gt_list_for_each_entry_continue()
 * @param pos    the type * to use as a start point
 * @param head   the head of the list
 * @param member the name of the _gt_list_head within the struct.
 *
 * Prepares a pos entry for use as a start point in _gt_list_for_each_entry_continue().
 */
#define _gt_list_prepare_entry(pos, head, type_name, member) \
	((pos) ? : _gt_list_entry(head, type_name, member))

/**
 * _gt_list_for_each_entry_continue - continue iteration over list of given type
 * @param pos    the type * to use as a loop cursor.
 * @param head   the head for your list.
 * @param member the name of the _gt_list_head within the struct.
 *
 * Continue to iterate over list of given type, continuing after
 * the current position.
 */
#define _gt_list_for_each_entry_continue(pos, head, type_name, member)         \
	for (pos = _gt_list_next_entry(pos, type_name, member);            \
	     &pos->member != (head);                    \
	     pos = _gt_list_next_entry(pos, type_name, member))

/**
 * _gt_list_for_each_entry_continue_reverse - iterate backwards from the given point
 * @param pos    the type * to use as a loop cursor.
 * @param head   the head for your list.
 * @param member the name of the _gt_list_head within the struct.
 *
 * Start to iterate over list of given type backwards, continuing after
 * the current position.
 */
#define _gt_list_for_each_entry_continue_reverse(pos, head, type_name, member)     \
	for (pos = _gt_list_prev_entry(pos, type_name, member);            \
	     &pos->member != (head);                    \
	     pos = _gt_list_prev_entry(pos, type_name, member))

/**
 * _gt_list_for_each_entry_from - iterate over list of given type from the current point
 * @param pos    the type * to use as a loop cursor.
 * @param head   the head for your list.
 * @param member the name of the _gt_list_head within the struct.
 *
 * Iterate over list of given type, continuing from current position.
 */
#define _gt_list_for_each_entry_from(pos, head, type_name, member)             \
	for (; &pos->member != (head);                  \
	     pos = _gt_list_next_entry(pos, type_name, member))

/**
 * _gt_list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @param pos    the type * to use as a loop cursor.
 * @param n      another type * to use as temporary storage
 * @param head   the head for your list.
 * @param member the name of the _gt_list_head within the struct.
 */
#define _gt_list_for_each_entry_safe(pos, n, head, type_name, member)          \
	for (pos = _gt_list_first_entry(head, type_name, member),    \
	    n = _gt_list_next_entry(pos, type_name, member);           \
	     &pos->member != (head);                    \
	     pos = n, n = _gt_list_next_entry(n, type_name, member))

/**
 * _gt_list_for_each_entry_safe_continue - continue list iteration safe against removal
 * @param pos    the type * to use as a loop cursor.
 * @param n      another type * to use as temporary storage
 * @param head   the head for your list.
 * @param member the name of the _gt_list_head within the struct.
 *
 * Iterate over list of given type, continuing after current point,
 * safe against removal of list entry.
 */
#define _gt_list_for_each_entry_safe_continue(pos, n, head, type_name, member)         \
	for (pos = _gt_list_next_entry(pos, type_name, member),                \
	    n = _gt_list_next_entry(pos, type_name, member);               \
	     &pos->member != (head);                        \
	     pos = n, n = _gt_list_next_entry(n, type_name, member))

/**
 * _gt_list_for_each_entry_safe_from - iterate over list from current point safe against removal
 * @param pos    the type * to use as a loop cursor.
 * @param n      another type * to use as temporary storage
 * @param head   the head for your list.
 * @param member the name of the _gt_list_head within the struct.
 *
 * Iterate over list of given type from current point, safe against
 * removal of list entry.
 */
#define _gt_list_for_each_entry_safe_from(pos, n, head, type_name, member)             \
	for (n = _gt_list_next_entry(pos, type_name, member);                  \
	     &pos->member != (head);                        \
	     pos = n, n = _gt_list_next_entry(n, type_name, member))

/**
 * _gt_list_for_each_entry_safe_reverse - iterate backwards over list safe against removal
 * @param pos    the type * to use as a loop cursor.
 * @param n      another type * to use as temporary storage
 * @param head   the head for your list.
 * @param member the name of the _gt_list_head within the struct.
 *
 * Iterate backwards over list of given type, safe against removal
 * of list entry.
 */
#define _gt_list_for_each_entry_safe_reverse(pos, n, head, type_name, member)      \
	for (pos = _gt_list_last_entry(head, type_name, member),     \
	    n = _gt_list_prev_entry(pos, type_name, member);           \
	     &pos->member != (head);                    \
	     pos = n, n = _gt_list_prev_entry(n, type_name, member))

/**
 * _gt_list_safe_reset_next - reset a stale _gt_list_for_each_entry_safe loop
 * @param pos    the loop cursor used in the _gt_list_for_each_entry_safe loop
 * @param n      temporary storage used in _gt_list_for_each_entry_safe
 * @param member the name of the _gt_list_head within the struct.
 *
 * _gt_list_safe_reset_next is not safe to use in general if the list may be
 * modified concurrently (eg. the lock is dropped in the loop body). An
 * exception to this is if the cursor element (pos) is pinned in the list,
 * and _gt_list_safe_reset_next is called after re-taking the lock and before
 * completing the current iteration of the loop body.
 */
#define _gt_list_safe_reset_next(pos, n, type_name, member)                \
	n = _gt_list_next_entry(pos, type_name, member)

#ifdef __cplusplus
}
#endif

#endif /* _GT_LIST_H_ */

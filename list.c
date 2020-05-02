#include "list.h"

void init_list_head(struct LIST_HEAD_T *list) {
    list->next = list;
    list->prev = list;
}

static void __list_add(struct LIST_HEAD_T *new, struct LIST_HEAD_T *prev, struct LIST_HEAD_T *next) {
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

void list_add(struct LIST_HEAD_T *new, struct LIST_HEAD_T *head) {
    __list_add(new, head, head->next);
}

static inline void __list_del(struct LIST_HEAD_T *prev, struct LIST_HEAD_T *next) {
    next->prev = prev;
    prev->next = next;
}

void list_del(struct LIST_HEAD_T *entry) {
    __list_del(entry->prev, entry->next);
    entry->next = NULL;
    entry->prev = NULL;
}

int list_empty(struct LIST_HEAD_T *head) {
    return head->next == head;
}

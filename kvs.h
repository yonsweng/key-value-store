#ifndef kvs_h
#define kvs_h

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

// 출처: https://www.sanfoundry.com/c-program-implement-skip-list/

#define SKIPLIST_MAX_LEVEL 6

typedef struct snode {
    int key;
    int value;
    struct snode **forward;
} snode;

typedef struct skiplist {
    pthread_rwlock_t rwlock;
    int level;
    int size;
    struct snode *header;
} skiplist;

skiplist *kv_create_db(void);

//int kv_destroy_db(skiplist *list);

int kv_put(skiplist *list, int key, int value);

snode *kv_get(skiplist *list, int key);

int kv_delete(skiplist *list, int key);

#endif /* kvs_h */

#ifndef kvs_h
#define kvs_h
#endif /* kvs_h */

#include <pthread.h>
#include "list.h"

#define USE_MUTEX   1
#define USE_RWLOCK  0

#define USE_HASH    1
#define USE_SKIP    0

#if USE_HASH
    #define MAX_HASH    69997
#endif

struct KVS_T {
#if USE_MUTEX
    pthread_mutex_t mu;
#else
    pthread_rwlock_t rwlock;
#endif
    struct LIST_HEAD_T list;
    struct LIST_HEAD_T hashtbl[MAX_HASH];
};

struct KVS_NODE_T {
    int key;
    int value;
    struct LIST_HEAD_T list;
    struct LIST_HEAD_T hash;
};

struct KVS_T *kv_create_db(void);

int kv_destroy_db(struct KVS_T *kvs);

//int kv_put(struct KVS_T *kvs, int key, int value);
int kv_put(struct KVS_T *kvs, struct KVS_NODE_T *kv_pair);

struct KVS_NODE_T *kv_get(struct KVS_T *kvs, int key);

struct KVS_NODE_T *kv_get_range(struct KVS_T *kvs, int start_key, int end_key, int *num_entries);

int kv_delete(struct KVS_T *kvs, int key);
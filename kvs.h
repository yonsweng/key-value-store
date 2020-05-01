#ifndef kvs_h
#define kvs_h
#endif /* kvs_h */

#include <pthread.h>
#include "list.h"

struct KVS_T {
    pthread_mutex_t mu;
    struct LIST_HEAD_T list;
};

struct KVS_NODE_T {
    int key;
    int value;
    struct LIST_HEAD_T list;
};

struct KVS_T *kv_create_db(void);
int kv_destroy_db(struct KVS_T *kvs);
//int kv_put(struct KVS_T *kvs, int key, int value);
int kv_put(struct KVS_T *kvs, struct KVS_NODE_T *kv_pair);
struct KVS_NODE_T *kv_get(struct KVS_T *kvs, int key);
struct KVS_NODE_T *kv_get_range(struct KVS_T *kvs, int start_key, int end_key, int *num_entries);
int kv_delete(struct KVS_T *kvs, int key);
#include "list.h"
#include "kvs.h"
#include <stdlib.h>
#include <string.h>

struct KVS_T *kv_create_db() {
    struct KVS_T *cc_kvs;

    cc_kvs = (struct KVS_T *) malloc(sizeof(struct KVS_T));
    if (cc_kvs == NULL)
        return NULL;

    init_list_head(&cc_kvs->list);

    for(int i = 0; i < MAX_HASH; i++) {
        init_list_head(&cc_kvs->hashtbl[i]);
    }

#if USE_MUTEX
    pthread_mutex_init(&cc_kvs->mu, NULL);
#else
    pthread_rwlock_init(&cc_kvs->rwlock, NULL);
#endif

    return cc_kvs;
}

int kv_destroy_db(struct KVS_T *kvs) {
    struct LIST_HEAD_T *p_entry, *tmp;
    struct KVS_NODE_T *p_kvs_node;

    if (kvs == NULL)
        return -1;

    list_for_each_safe(p_entry, tmp, &kvs->list) {
        p_kvs_node = list_entry(p_entry, struct KVS_NODE_T, list);
        list_del(&p_kvs_node->list);
        free(p_kvs_node);
    }

    free(kvs);

    return 0;
}

int kv_put(struct KVS_T *kvs, struct KVS_NODE_T *kv_pair) {
    int found = 0;
    struct LIST_HEAD_T *p_entry;
    struct KVS_NODE_T *p_kvs_node;

    if (kvs == NULL) return -1;
#if USE_MUTEX
    pthread_mutex_lock(&kvs->mu);
#else
    pthread_rwlock_wrlock(&kvs->rwlock);
#endif

#if USE_HASH
    int hash = kv_pair->key % MAX_HASH;
    list_for_each(p_entry, &kvs->hashtbl[hash]) {
        p_kvs_node = list_entry(p_entry, struct KVS_NODE_T, hash);
#else
    list_for_each(p_entry, &kvs->list) {
        p_kvs_node = list_entry(p_entry, struct KVS_NODE_T, list);
#endif
        if (p_kvs_node->key == kv_pair->key) {
            found = 1;
            p_kvs_node->value = kv_pair->value;
            break;
        }
    }

    if (!found) {
        p_kvs_node = (struct KVS_NODE_T *) malloc(sizeof(struct KVS_NODE_T));
        p_kvs_node->key = kv_pair->key;
        p_kvs_node->value = kv_pair->value;

        init_list_head(&p_kvs_node->list);
        init_list_head(&p_kvs_node->hash);

        list_add(&p_kvs_node->list, &kvs->list);
        list_add(&p_kvs_node->hash, &kvs->hashtbl[hash]);
    }
#if USE_MUTEX
    pthread_mutex_unlock(&kvs->mu);
#else
    pthread_rwlock_unlock(&kvs->rwlock);
#endif
    return found;
}

struct KVS_NODE_T *kv_get(struct KVS_T *kvs, int key) {
    int found = 0;
    struct LIST_HEAD_T *p_entry;
    struct KVS_NODE_T *p_kvs_node = NULL, *ret = NULL;

    if (kvs == NULL) return NULL;
#if USE_MUTEX
    pthread_mutex_lock(&kvs->mu);
#else
    pthread_rwlock_rdlock(&kvs->rwlock);
#endif
    ret = (struct KVS_NODE_T *) malloc(sizeof(struct KVS_NODE_T));
    if (ret == NULL) return NULL;
#if USE_HASH
    int hash = key % MAX_HASH;
    list_for_each(p_entry, &kvs->hashtbl[hash]) {
        p_kvs_node = list_entry(p_entry, struct KVS_NODE_T, hash);
#else
    list_for_each(p_entry, &kvs->list) {
        p_kvs_node = list_entry(p_entry, struct KVS_NODE_T, list);
#endif
        if (p_kvs_node->key == key) {
            found = 1;
            memcpy((void *) ret, (void *) p_kvs_node, sizeof(struct KVS_NODE_T));
            break;
        }
    }
#if USE_MUTEX
    pthread_mutex_unlock(&kvs->mu);
#else
    pthread_rwlock_unlock(&kvs->rwlock);
#endif
    if (found)
        return ret;
    else
        return NULL;
}

struct KVS_NODE_T *kv_get_range(struct KVS_T *kvs, int start_key, int end_key, int *num_entries) {
    struct LIST_HEAD_T *p_entry;
    struct KVS_NODE_T *p_kvs_node, *p_temp_node, *ret;

    if (kvs == NULL) return NULL;

    *num_entries = 0;
    ret = (struct KVS_NODE_T *) malloc(sizeof(struct KVS_NODE_T));
    init_list_head(&ret->list);

    // 1. key is in the db?
    list_for_each(p_entry, &kvs->list) {
        p_kvs_node = list_entry(p_entry, struct KVS_NODE_T, list);

        if ((start_key <= p_kvs_node->key) && (p_kvs_node->key <= end_key)) {
            p_temp_node = (struct KVS_NODE_T *) malloc(sizeof(struct KVS_NODE_T));
            p_temp_node->key = p_kvs_node->key;
            p_temp_node->value = p_kvs_node->value;
            list_add(&p_temp_node->list, &ret->list);

            *num_entries += 1;
        }
    }

    if (*num_entries == 0) {
        free(ret);
        return NULL;
    }

    return ret;
}

int kv_delete(struct KVS_T *kvs, int key) {
    int found = 0;
    struct LIST_HEAD_T *p_entry, *tmp;
    struct KVS_NODE_T *p_kvs_node;

    if (kvs == NULL)  // If key-value storage is not offered.
        return -1;

#if USE_MUTEX
    pthread_mutex_lock(&kvs->mu);
#else
    pthread_rwlock_wrlock(&kvs->rwlock);
#endif

    list_for_each_safe(p_entry, tmp, &kvs->list) {
        p_kvs_node = list_entry(p_entry, struct KVS_NODE_T, list);
        if (p_kvs_node->key == key) {
            found = 1;
            list_del(&p_kvs_node->list);
            free(p_kvs_node);
        }
    }

#if USE_MUTEX
    pthread_mutex_unlock(&kvs->mu);
#else
    pthread_rwlock_unlock(&kvs->rwlock);
#endif

    if (found)
        return 1;
    else
        return 0;
}
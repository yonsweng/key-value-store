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

    pthread_mutex_lock(&kvs->mu);

    list_for_each(p_entry, &kvs->list) {
        p_kvs_node = list_entry(p_entry, struct KVS_NODE_T, list);
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
        list_add(&p_kvs_node->list, &kvs->list);
    }

    pthread_mutex_unlock(&kvs->mu);
    return found;
}

struct KVS_NODE_T *kv_get(struct KVS_T *kvs, int key) {
    int found = 0;
    struct LIST_HEAD_T *p_entry;
    struct KVS_NODE_T *p_kvs_node = NULL, *ret = NULL;

    if (kvs == NULL) return NULL;

    pthread_mutex_lock(&kvs->mu);

    ret = (struct KVS_NODE_T *) malloc(sizeof(struct KVS_NODE_T));
    if (ret == NULL) return NULL;
    // 1. key is in the db?
    list_for_each(p_entry, &kvs->list) {
        p_kvs_node = list_entry(p_entry, struct KVS_NODE_T, list);

        if (p_kvs_node->key == key) {
            found = 1;
            break;
        }
    }

    if (found) {
        memcpy((void *) ret, (void *) p_kvs_node, sizeof(struct KVS_NODE_T));
        pthread_mutex_unlock(&kvs->mu);
        return ret;
    } else {
        pthread_mutex_unlock(&kvs->mu);
        return NULL;
    }
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

    pthread_mutex_lock(&kvs->mu);

    list_for_each_safe(p_entry, tmp, &kvs->list) {
        p_kvs_node = list_entry(p_entry, struct KVS_NODE_T, list);
        if (p_kvs_node->key == key) {
            found = 1;
            list_del(&p_kvs_node->list);
            free(p_kvs_node);
        }
    }

    if (found) {
        pthread_mutex_unlock(&kvs->mu);
        return 1;
    }
    else {
        pthread_mutex_unlock(&kvs->mu);
        return 0;
    }
}
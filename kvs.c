#include "kvs.h"

// 출처: https://www.sanfoundry.com/c-program-implement-skip-list/

skiplist *kv_create_db() {
    int i;
    snode *header = (snode *) malloc(sizeof(struct snode));
    skiplist *list = (skiplist *) malloc(sizeof(skiplist));
    list->header = header;
    header->key = INT_MAX;
    header->forward = (snode **) malloc(
            sizeof(snode *) * (SKIPLIST_MAX_LEVEL + 1));
    for (i = 0; i <= SKIPLIST_MAX_LEVEL; i++) {
        header->forward[i] = list->header;
    }

    list->level = 1;
    list->size = 0;

    return list;
}

static int rand_level() {
    int level = 1;
    while (rand() < RAND_MAX / 2 && level < SKIPLIST_MAX_LEVEL)
        level++;
    return level;

}

int kv_put(skiplist *list, int key, int value) {
    pthread_rwlock_wrlock(&list->rwlock);

    snode *update[SKIPLIST_MAX_LEVEL + 1];
    snode *x = list->header;
    int i, level;

    for (i = list->level; i >= 1; i--) {
        while (x->forward[i]->key < key)
            x = x->forward[i];
        update[i] = x;
    }
    x = x->forward[1];

    if (key == x->key) {
        x->value = value;
    } else {
        level = rand_level();

        if (level > list->level) {
            for (i = list->level + 1; i <= level; i++) {
                update[i] = list->header;
            }
            list->level = level;
        }

        x = (snode *) malloc(sizeof(snode));
        x->key = key;
        x->value = value;
        x->forward = (snode **) malloc(sizeof(snode *) * (level + 1));

        for (i = 1; i <= level; i++) {
            x->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = x;
        }
    }

    pthread_rwlock_unlock(&list->rwlock);

    return 0;
}


snode *kv_get(skiplist *list, int key) {
    pthread_rwlock_wrlock(&list->rwlock);
    snode *x = list->header;
    int i;
    for (i = list->level; i >= 1; i--) {
        while (x->forward[i]->key < key)
            x = x->forward[i];
    }

    if (x->forward[1]->key == key) {
        return x->forward[1];
    } else {
        return NULL;
    }
    pthread_rwlock_unlock(&list->rwlock);
}


static void skiplist_node_free(snode *x) {
    if (x) {
        free(x->forward);
        free(x);
    }
}

int kv_delete(skiplist *list, int key) {
    int i;
    snode *update[SKIPLIST_MAX_LEVEL + 1];
    snode *x = list->header;
    for (i = list->level; i >= 1; i--) {
        while (x->forward[i]->key < key)
            x = x->forward[i];
        update[i] = x;
    }

    x = x->forward[1];

    if (x->key == key) {
        for (i = 1; i <= list->level; i++) {
            if (update[i]->forward[i] != x)
                break;
            update[i]->forward[1] = x->forward[i];
        }
        skiplist_node_free(x);

        while (list->level > 1 && list->header->forward[list->level] == list->header)
            list->level--;

        return 0;
    }

    return 1;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "list.h"
#include "kvs.h"

#define NUM_NODES        100000
#define NUM_THREADS        10

// void *search(void *arg) {}
// void *insert(void *arg) {}
// void *delete(void *arg) {}
typedef void (*THREAD_FUNC)(void *);

struct SHARED_STATE_T {
    pthread_mutex_t mu;
    pthread_cond_t cv;
    int total;
    int num_inited;
    int num_done;
    int start;
};

struct THREAD_STATE_T {
    pthread_t tid;
    int rand_seed;     /*1: 1-1000000, 2: 1000001-2000000*/
    time_t start;
    time_t finish;
    int done;
    struct KVS_T *kvs;
};

struct THREAD_ARG_T {
    struct SHARED_STATE_T *ss;
    struct THREAD_STATE_T *ts;
    THREAD_FUNC tf;
};

void *thread_main(void *arg) {
    struct THREAD_ARG_T *thread_arg = arg;
    struct SHARED_STATE_T *ss = thread_arg->ss;
    struct THREAD_STATE_T *ts = thread_arg->ts;

    pthread_mutex_lock(&ss->mu);
    ss->num_inited++;
    if (ss->num_inited >= ss->total) {
        pthread_cond_broadcast(&ss->cv);
    }
    while (!ss->start) {
        pthread_cond_wait(&ss->cv, &ss->mu);
    }
    pthread_mutex_unlock(&ss->mu);

    ts->start = time(NULL);
    (thread_arg->tf)((void *) ts);
    ts->finish = time(NULL);

    pthread_mutex_lock(&ss->mu);
    ss->num_done++;
    if (ss->num_done >= ss->total) {
        pthread_cond_broadcast(&ss->cv);
    }
    pthread_mutex_unlock(&ss->mu);

    return NULL;
}

void do_benchmark(struct KVS_T *kvs, int num_threads, THREAD_FUNC tf) {
    int i, ret;
    double elapsed = 0, num_ops = 0;
    struct SHARED_STATE_T shared;
    struct THREAD_STATE_T *tss;
    struct THREAD_ARG_T *args;

    pthread_mutex_init(&shared.mu, NULL);
    pthread_cond_init(&shared.cv, NULL);
    shared.num_inited = 0;
    shared.num_done = 0;
    shared.start = 0;
    shared.total = num_threads;

    args = (struct THREAD_ARG_T *) malloc(sizeof(struct THREAD_ARG_T) * num_threads);
    if (args == NULL) {
        printf("args malloc failed\n");
        return;
    }
    memset((void *) args, 0x00, sizeof(struct THREAD_ARG_T) * num_threads);

    tss = (struct THREAD_STATE_T *) malloc(sizeof(struct THREAD_STATE_T) * num_threads);
    if (tss == NULL) {
        printf("tss malloc failed\n");
        free(args);
        return;
    }
    memset((void *) tss, 0x00, sizeof(struct THREAD_STATE_T) * num_threads);

    for (i = 0; i < num_threads; i++) {
        args[i].ss = &shared;
        args[i].ts = &tss[i];
        args[i].ts->kvs = kvs;
        args[i].tf = tf;

        pthread_create(&args[i].ts->tid, NULL, thread_main, &args[i]);
    }

    pthread_mutex_lock(&shared.mu);
    while (shared.num_inited < num_threads) {
        pthread_cond_wait(&shared.cv, &shared.mu);
    }

    shared.start = 1;
    pthread_cond_broadcast(&shared.cv);
    while (shared.num_done < num_threads) {
        pthread_cond_wait(&shared.cv, &shared.mu);
    }
    pthread_mutex_unlock(&shared.mu);

    for (i = 0; i < num_threads; i++) {
        ret = pthread_join(args[i].ts->tid, NULL);
    }

    // results
    for (i = 0; i < num_threads; i++) {
        elapsed = difftime(tss[i].finish, tss[i].start);
        num_ops += tss[i].done;
    }

    printf("\nops = %f\n", num_ops / elapsed);

    // free
    free(tss);
    free(args);
}

void do_benchmark2(struct KVS_T *kvs, int num_threads, THREAD_FUNC tf1, THREAD_FUNC tf2, double ratio1, double ratio2) {
    int num_threads1 = (int) round(num_threads * ratio1 / (ratio1 + ratio2));
    int num_threads2 = num_threads - num_threads1;
    int i, ret;
    double elapsed = 0, num_ops = 0;
    struct SHARED_STATE_T shared;
    struct THREAD_STATE_T *tss;
    struct THREAD_ARG_T *args;

    pthread_mutex_init(&shared.mu, NULL);
    pthread_cond_init(&shared.cv, NULL);
    shared.num_inited = 0;
    shared.num_done = 0;
    shared.start = 0;
    shared.total = num_threads;

    args = (struct THREAD_ARG_T *) malloc(sizeof(struct THREAD_ARG_T) * num_threads);
    if (args == NULL) {
        printf("args malloc failed\n");
        return;
    }
    memset((void *) args, 0x00, sizeof(struct THREAD_ARG_T) * num_threads);

    tss = (struct THREAD_STATE_T *) malloc(sizeof(struct THREAD_STATE_T) * num_threads);
    if (tss == NULL) {
        printf("tss malloc failed\n");
        free(args);
        return;
    }
    memset((void *) tss, 0x00, sizeof(struct THREAD_STATE_T) * num_threads);

    for (i = 0; i < num_threads1; i++) {
        args[i].ss = &shared;
        args[i].ts = &tss[i];
        args[i].ts->kvs = kvs;
        args[i].tf = tf1;

        pthread_create(&args[i].ts->tid, NULL, thread_main, &args[i]);
    }

    for (i = num_threads1; i < num_threads1 + num_threads2; i++) {
        args[i].ss = &shared;
        args[i].ts = &tss[i];
        args[i].ts->kvs = kvs;
        args[i].tf = tf2;

        pthread_create(&args[i].ts->tid, NULL, thread_main, &args[i]);
    }

    pthread_mutex_lock(&shared.mu);
    while (shared.num_inited < num_threads) {
        pthread_cond_wait(&shared.cv, &shared.mu);
    }

    shared.start = 1;
    pthread_cond_broadcast(&shared.cv);
    while (shared.num_done < num_threads) {
        pthread_cond_wait(&shared.cv, &shared.mu);
    }
    pthread_mutex_unlock(&shared.mu);

    for (i = 0; i < num_threads; i++) {
        ret = pthread_join(args[i].ts->tid, NULL);
    }

    // results
    for (i = 0; i < num_threads; i++) {
        elapsed = difftime(tss[i].finish, tss[i].start);
        num_ops += tss[i].done;
    }

    printf("\nops = %f\n", num_ops / elapsed);

    // free
    free(tss);
    free(args);
}

void search(void *arg) {
    int i;
    struct THREAD_STATE_T *ts = arg;
    struct KVS_NODE_T *p_node;


    for (i = 0; i < NUM_NODES; i++) {
        p_node = kv_get(ts->kvs, i);

        if (p_node)
            ts->done++;

        if ((ts->done % 10000) == 0)
            printf(".");
    }
}

void insert(void *arg) {
    int i, ret;
    struct THREAD_STATE_T *ts = arg;
    struct KVS_NODE_T node;

    for (i = 0; i < NUM_NODES; i++) {
        node.key = i;
        node.value = i;
        ret = kv_put(ts->kvs, &node);

        ts->done++;

        if ((ts->done % 10000) == 0)
            printf(".");
    }
}

int main() {
    struct KVS_T *my_kvs;
    struct KVS_NODE_T node;

    // 1. create key/value store
    my_kvs = kv_create_db();
    if (my_kvs == NULL) {
        printf("kv create if failed\n");
        getchar();

        return -1;
    }

    // 사전에 key-value store에 필요한 key 개수만큼 채워라!
    for (int i = 0; i < 50000; i++) {
        node.key = i;
        node.value = i;
        kv_put(my_kvs, &node);
    }

    printf("start\n");
    do_benchmark2(my_kvs, NUM_THREADS, search, insert, 0.9, 0.1);
    printf("finish\n");

    kv_destroy_db(my_kvs);

    printf("test is done!\n");
    getchar();

    return 0;
}

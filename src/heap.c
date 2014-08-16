#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>

#include "heap.h"

#define INITIAL_MAX 256
#define HEAP_PARENT(position) (((position) - 1) >> 1)
#define HEAP_LEFT(position)   (((position) << 1) + 1)
#define HEAP_RIGHT(position)  (((position) + 1) << 1)

struct heap_t
{
    pthread_mutex_t mutex;
    void **tree;
    heap_cmp_callback_fn_t *cmp;
    heap_display_callback_fn_t *display;
    /* Then we may specify a function to deallocate data */
    heap_del_callback_fn_t *del;
    unsigned int count, max;
    int mutex_set;      /* true (i.e. 1) if heap_make_r has been
                           called instead of non-reentrant function */
};

heap_t *heap_make(heap_cmp_callback_fn_t *cmp , heap_del_callback_fn_t *del)
{
    heap_t *heap = NULL;;
    if (NULL != (heap = malloc(sizeof(heap_t)))) {
        heap->del = del;
        if (NULL == (heap->tree = calloc(INITIAL_MAX, sizeof(void *)))) {
            free(heap);
            heap = NULL;
        }
    }
    if (NULL != heap) {
        heap->max = INITIAL_MAX;
        heap->count = 0;
        heap->cmp = cmp;
        heap->mutex_set = 0;
        memset(&(heap->mutex), 0, sizeof(pthread_mutex_t));
    }

    return heap;
}

void heap_destroy(heap_t *heap)
{
    if (NULL != heap) {
        if (NULL != heap->del && NULL != heap->tree) {
            unsigned int i;
            for (i = 0; i < heap->count; i++) {
                heap->del(heap->tree[i]);
            }
        }
        free(heap->tree);
        free(heap);
    }
}

int heap_insert(heap_t *heap, void *datum)
{
    void **tmp;
    unsigned int ipos, ppos;

    if (heap->max <= heap->count) {
        /*
         * reallocation by power of 2:
         */
        unsigned int new_max;

        for (new_max = 1; new_max <= heap->max; new_max *= 2);

        tmp = realloc(heap->tree, new_max * sizeof(void *));
        if (NULL != tmp) {
            memset((tmp + (heap->count)), 0, (new_max - (heap->count + 1)) * sizeof(void *));
            heap->tree = tmp;
            heap->max = new_max;
        }
        else {
            heap->tree = NULL;
            fprintf(stderr, "allocation failed\n");
            return -1;
        }
    }

    /*
     * insertion of the datum after the last one of the tree...
     */
    heap->tree[heap->count] = datum;

    ipos = heap->count;
    ppos = HEAP_PARENT(ipos);

    while (ipos > 0 && (heap->cmp(heap->tree[ppos], heap->tree[ipos]) < 0)) {
        /*
         * Swap the value ...
         */
        tmp = heap->tree[ppos];
        heap->tree[ppos] = heap->tree[ipos];
        heap->tree[ipos] = tmp;

        ipos = ppos;
        ppos = HEAP_PARENT(ipos);
    }

    heap->count++;

    return 0;
}

void *heap_extract(heap_t *heap)
{
    void *ret = NULL, *tmp;
    unsigned int ipos, rpos, lpos, mpos;

    if ((0 != heap->count) && (NULL != heap->tree)) {
        /* keep the value to return */
        ret = heap->tree[0];
        /* It works for count == 1 too (just think about it) */
        heap->tree[0] = heap->tree[heap->count - 1];
        heap->tree[heap->count - 1] = NULL;
        heap->count--;
        ipos = 0;

        while (1) {
            lpos = HEAP_LEFT(ipos);
            rpos = HEAP_RIGHT(ipos);

            if (lpos < heap->count) {
                if (heap->cmp(heap->tree[lpos], heap->tree[ipos]) > 0) {
                    mpos = lpos;
                }
                else {
                    mpos = ipos;
                }
                if ((rpos < heap->count) && (heap->cmp(heap->tree[rpos], heap->tree[mpos])) > 0) {
                    mpos = rpos;
                }
            }
            else {
                mpos = ipos;
            }

            if (mpos != ipos) {
                /*
                 * Swap the choosen children with the current node
                 */
                tmp = heap->tree[mpos];
                heap->tree[mpos] = heap->tree[ipos];
                heap->tree[ipos] = tmp;
                ipos = mpos;
            }
            else {
                break;
            }
        }
    }

    return ret;
}

void *heap_get_nth(const heap_t *heap, unsigned int n)
{
    if ((n < heap->count) && (NULL != heap->tree))
        return heap->tree[n];
    else
        return NULL;
}

unsigned int heap_size(const heap_t *heap)
{
    return heap->count;
}

/*
 * Reentrant versions of previous functions.
 */
heap_t *heap_make_r(heap_cmp_callback_fn_t *cmp , heap_del_callback_fn_t *del)
{
    heap_t *heap;

    if (NULL != (heap = heap_make(cmp , del))) {
        if (0 > pthread_mutex_init(&(heap->mutex), NULL)) {
            free(heap->tree);
            free(heap);
            heap = NULL;
        }
    }
    heap->mutex_set = 1;

    return heap;
}

int heap_insert_r(heap_t *heap, void *datum)
{
    int rc;

    if (1 == heap->mutex_set) {
        if (0 == (rc = pthread_mutex_lock(&heap->mutex))) {
            if (0 == (rc = heap_insert(heap, datum)))
                rc = pthread_mutex_unlock(&heap->mutex);
        }
    }
    else
        rc = -1;

    return rc;
}

void *heap_extract_r(heap_t *heap)
{
    void *result;

    result = NULL;
    if (1 == heap->mutex_set) {
        if (0 == pthread_mutex_lock(&heap->mutex)) {
            if (NULL != (result = heap_extract(heap)))
                if (0 != pthread_mutex_unlock(&heap->mutex))
                    result = NULL;
        }
    }

    return result;
}

void heap_set_display_cb(heap_t *heap, heap_display_callback_fn_t display)
{
    heap->display = display;
}

void heap_display(const heap_t *heap)
{
    unsigned int i;

    for (i = 0; i < heap->count; i++) {
        if (((i - 1) % 2) == 0) {
            printf("\n");
        }
        printf("%u:", i);
        heap->display(heap->tree[i]);
        printf("\t");
    }
    printf("\n");
    fflush(stdout);
}

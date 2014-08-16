/*
 * Copyright (C) 2014  François Pesce
   *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 0 -*- */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

#include "heap.h"
#include "list.h"
#include "levenshtein.h"
#include "mmap_wrapper.h"

#define EPSILON 0.4
#define IGNORE_SIZE 4

struct cluster_t {
    list_t *words;
};
typedef struct cluster_t cluster_t;

struct word_t {
    char *word;
    size_t word_len;
    size_t idx;
    cluster_t *cluster;
};
typedef struct word_t word_t;

struct distance_t {
    int word_1, word_2;
    float value;
};
typedef struct distance_t distance_t;

static int distance_cmp(const void *data1, const void *data2)
{
    const distance_t *d1 = data1;
    const distance_t *d2 = data2;

    return (d1->value > d2->value) ? 1 : ((d1->value < d2->value) ? -1 : 0);
}

static void distance_del(void *data)
{
    distance_t *d = data;
    free(d);
}

static inline int process_file(const char *file)
{
    heap_t *heap;
    mmap_wrapper_t *mw;
    list_t *clusters;
    cluster_t *cluster;
    cell_t *cell;
    word_t *words;
    distance_t *d;
    const char *word;
    float *similarity;
    size_t i, j, idx, len, nb_words;
    int rv;

    rv = mmap_wrapper_init(&mw, file);
    if (0 != rv) {
        fprintf(stderr, "error calling mmap_wrapper_init on file %s\n", file);
        return -1;
    }

    /* First pass: count words. */
    fprintf(stderr, "first pass\n");
    for (nb_words = 0, rv = mmap_get_head(mw, &idx); 0 == rv; rv = mmap_get_next2(mw, &idx, "\r\n\t"), nb_words++) {
        word = mmap_get_line2(mw, &idx, &len, "\r\n\t");
        if (NULL == word) {
            fprintf(stderr, "error calling mmap_get_line on file %s idx: %lu\n", file, idx);
            mmap_wrapper_delete(mw);
            return -1;
        }
        if (0 == len)
            continue;
    }

    words = calloc(nb_words, sizeof(struct word_t));
    similarity = calloc(nb_words * nb_words, sizeof(float));

    /* Second pass: get words. */
    fprintf(stderr, "second pass\n");
    for (nb_words = 0, rv = mmap_get_head(mw, &idx); 0 == rv; rv = mmap_get_next2(mw, &idx, "\r\n\t"), nb_words++) {
        word = mmap_get_line2(mw, &idx, &len, "\r\n\t");
        if (NULL == word) {
            fprintf(stderr, "error calling mmap_get_line on file %s idx: %lu\n", file, idx);
            mmap_wrapper_delete(mw);
            return -1;
        }
        if(len > IGNORE_SIZE) {
            words[nb_words].word = strndup(word, len);
            words[nb_words].word_len = len;
            words[nb_words].idx = nb_words;
        } else {
            nb_words--;
        }
        if (0 == len)
            continue;
    }
    mmap_wrapper_delete(mw);

    heap = heap_make(distance_cmp, distance_del);

    /* Third pass: get words distances. */
    fprintf(stderr, "third pass\n");
    for (i = 0; i < nb_words; i++) {
        similarity[i + i * nb_words] = 1.0;
        for (j = i + 1; j < nb_words; j++) {
            d = malloc(sizeof(struct distance_t));
            similarity[i + j * nb_words] = levenshtein_norm_distance(words[i].word, words[i].word_len, words[j].word, words[j].word_len);
            similarity[j + i * nb_words] = similarity[i + j * nb_words];
            d->value = similarity[i + j *nb_words];
            d->word_1 = i;
            d->word_2 = j;
            heap_insert(heap, d);
        }
    }

    /* Start clustering */
    clusters = list_make();
    fprintf(stderr, "fourth pass\n");
    for (d = heap_extract(heap); d != NULL; d = heap_extract(heap)) {
        if (NULL == words[d->word_1].cluster) {
            if (NULL == words[d->word_2].cluster) {
                /* fprintf(stderr, "%f [%.*s] [%.*s]\n", d->value, (int) words[d->word_1].word_len, words[d->word_1].word, */
                /*                                        (int) words[d->word_2].word_len, words[d->word_2].word); */
                cluster = malloc(sizeof(struct cluster_t));
                cluster->words = list_make();
                list_enqueue_elt(cluster->words, &(words[d->word_1]));
                list_enqueue_elt(cluster->words, &(words[d->word_2]));
                words[d->word_1].cluster = cluster;
                words[d->word_2].cluster = cluster;
                list_enqueue_elt(clusters, cluster);
            } else {
                /* fprintf(stderr, "%f [%.*s] in [%.*s]\n", d->value, (int) words[d->word_1].word_len, words[d->word_1].word, */
                /*                                        (int) words[d->word_2].word_len, words[d->word_2].word); */
                list_enqueue_elt(words[d->word_2].cluster->words, &(words[d->word_1]));
                words[d->word_1].cluster = words[d->word_2].cluster;
            }
        } else if (NULL == words[d->word_2].cluster) {
            /* fprintf(stderr, "%f [%.*s] in [%.*s]\n", d->value, (int) words[d->word_2].word_len, words[d->word_2].word, */
            /*                                        (int) words[d->word_1].word_len, words[d->word_1].word); */
            list_enqueue_elt(words[d->word_1].cluster->words, &(words[d->word_2]));
            words[d->word_2].cluster = words[d->word_1].cluster;
        } else if ((words[d->word_1].cluster != words[d->word_2].cluster) && (d->value > EPSILON)) {
            /*
             * Compare each word of cluster 1 to each word of cluster 2, if the
             * maximum measured similarity is smaller than EPSILON do not merge them.
             */
            cell_t *cellword1, *cellword2;
            float max_dist;
            int mismatch;

            /* fprintf(stderr, "%f clustered [%.*s] [%.*s]\n", d->value, (int) words[d->word_1].word_len, words[d->word_1].word, */
            /*                                        (int) words[d->word_2].word_len, words[d->word_2].word); */
            max_dist = 0.0;
            mismatch = 0;
            for (cellword1 = list_first(words[d->word_1].cluster->words);
                 (cellword1 != NULL) && (mismatch == 0);
                 cellword1 = list_next(cellword1)) {
                word_t *word1;
                word1 = list_get(cellword1);
                for (cellword2 = list_first(words[d->word_2].cluster->words);
                     (cellword2 != NULL) && (mismatch == 0);
                     cellword2 = list_next(cellword2)) {
                    word_t *word2;

                    word2 = list_get(cellword2);
                    if (similarity[word1->idx + word2->idx * nb_words] < EPSILON) {
                        mismatch = 1;
                        /* fprintf(stderr, "%f mismatch cluster [%.*s](%zi)(%p) [%.*s](%zi)(%p)\n", */
                                /* similarity[word1->idx + word2->idx * nb_words], */
                                /* (int) word1->word_len, word1->word, word1->idx, word1->cluster, */
                                /* (int) word2->word_len, word2->word, word2->idx, word2->cluster); */
                    }
                }
            }
            if (mismatch == 0) {
                /* Merge */
                cluster_t *tmpcluster;

                tmpcluster = words[d->word_2].cluster;
                for (cellword2 = list_first(words[d->word_2].cluster->words);
                        (cellword2 != NULL) && (mismatch == 0);
                        cellword2 = list_next(cellword2)) {
                    word_t *word2;

                    word2 = list_get(cellword2);
                    word2->cluster = words[d->word_1].cluster;
                    list_enqueue_elt(words[d->word_1].cluster->words, word2);
                    /* fprintf(stderr, "merge [%.*s] in [%.*s]\n", (int) word2->word_len, word2->word, */
                    /*                                                   (int) words[d->word_1].word_len, words[d->word_1].word); */
                }
                list_remove(clusters, tmpcluster);
                list_delete(tmpcluster->words);
                free(tmpcluster);
            }
        }
    }

    /* List clusters */
    for (i = 0, cell = list_first(clusters); cell != NULL; cell = list_next(cell), i++) {
        cell_t *word_cell;
        cluster = list_get(cell);
        fprintf(stdout, "Cluster %zi: ", i);
        for (word_cell = list_first(cluster->words); word_cell != NULL; word_cell = list_next(word_cell)) {
            word_t *word;
            word = list_get(word_cell);
            fprintf(stdout, "[%.*s] ", (int) word->word_len, word->word);
        }
        fprintf(stdout, "\n");
    }

    return 0;
}

int main(int argc, char **argv)
{
    int rv;

    if (argc < 2) {
        fprintf(stderr, "%s requires one parameter, which is input filename.\n", argv[0]);
        return -1;
    }

    rv = process_file(argv[1]);
    if (0 != rv) {
        fprintf(stderr, "error calling process_file\n");
        return -1;
    }

    return 0;
}

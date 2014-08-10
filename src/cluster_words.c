/*
 * Copyright (C) 2011  François Pesce
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

#include "levenshtein.h"
#include "mmap_wrapper.h"

#define CLUSTER_THRESHOLD 3
#define DEFAULT_DELTA 3
#define IGNORE_SIZE 4

struct word_t {
    char *word;
    size_t word_len;
};
typedef struct word_t word_t;

static inline int process_file(const char *file)
{
    mmap_wrapper_t *mw;
    word_t *words;
    const char *word;
    size_t *distance_matrix;
    size_t i, j, idx, len, nb_words;
    int rv;

    rv = mmap_wrapper_init(&mw, file);
    if (0 != rv) {
        fprintf(stderr, "error calling mmap_wrapper_init on file %s\n", file);
        return -1;
    }

    /* First pass: count words. */
    fprintf(stderr, "first pass\n");
    for (nb_words = 0, rv = mmap_get_head(mw, &idx); 0 == rv; rv = mmap_get_next2(mw, &idx, " \r\n\t"), nb_words++) {
        word = mmap_get_line2(mw, &idx, &len, " \r\n\t");
        if (NULL == word) {
            fprintf(stderr, "error calling mmap_get_line on file %s idx: %lu\n", file, idx);
            mmap_wrapper_delete(mw);
            return -1;
        }
        if (0 == len)
            continue;
    }

    words = calloc(nb_words, sizeof(struct word_t));
    distance_matrix = calloc(nb_words * nb_words, sizeof(size_t));

    /* Second pass: get words. */
    fprintf(stderr, "second pass\n");
    for (nb_words = 0, rv = mmap_get_head(mw, &idx); 0 == rv; rv = mmap_get_next2(mw, &idx, " \r\n\t"), nb_words++) {
        word = mmap_get_line2(mw, &idx, &len, " \r\n\t");
        if (NULL == word) {
            fprintf(stderr, "error calling mmap_get_line on file %s idx: %lu\n", file, idx);
            mmap_wrapper_delete(mw);
            return -1;
        }
        if(len > IGNORE_SIZE) {
            words[nb_words].word = strndup(word, len);
            words[nb_words].word_len = len - 1;
        } else {
            nb_words--;
        }
        if (0 == len)
            continue;
    }
    mmap_wrapper_delete(mw);

    /* Third pass: get words distances. */
    fprintf(stderr, "third pass\n");
    for (i = 0; i < nb_words; i++) {
        for (j = i + 1; j < nb_words; j++) {
            distance_matrix[i + j * nb_words] = levenshtein_distance(words[i].word, words[i].word_len, words[j].word, words[j].word_len, 0);
        }
    }

    /* Fourth pass: report every words distance < DEFAULT_DELTA */
    fprintf(stderr, "fourth pass\n");
    for (i = 0; i < nb_words; i++) {
        int found = 0;
        /* Pass 1 bis, check if there's something */
        for (j = 0; (j < nb_words) && (CLUSTER_THRESHOLD > found); j++) {
            if(j != i) {
                size_t min, max;
                min = (i < j) ? i : j;
                max = (i >=j) ? i : j;
                if(distance_matrix[min + max * nb_words] < DEFAULT_DELTA) {
                    found++;
                }
            }
        }
        if (CLUSTER_THRESHOLD <= found) {
            fprintf(stdout, "%s: ", words[i].word);
            for (j = 0; j < nb_words; j++) {
                if(j != i) {
                    size_t min, max;
                    min = (i < j) ? i : j;
                    max = (i >=j) ? i : j;
                    if(distance_matrix[min + max * nb_words] < DEFAULT_DELTA) {
                        fprintf(stdout, "%s ", words[j].word);
                    }
                }
            }
            fprintf(stdout, "\n");
        }
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

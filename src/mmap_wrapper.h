/*
 * Copyright (C) 2011 François Pesce : francois.pesce (at) gmail (dot) com
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

#ifndef MMAP_WRAPPER_H
#define MMAP_WRAPPER_H

#include <stdlib.h>
#include <unistd.h>

typedef struct mmap_wrapper_t mmap_wrapper_t;

int mmap_wrapper_init(mmap_wrapper_t **mw, const char *fname);

int mmap_wrapper_delete(mmap_wrapper_t *mw);

int mmap_get_head(mmap_wrapper_t *mw, size_t * idx);

int mmap_get_next(mmap_wrapper_t *mw, size_t * idx, char separator);

int mmap_get_next2(mmap_wrapper_t *mw, size_t * idx, const char *separators);

const char *mmap_get_line(mmap_wrapper_t *mw, size_t * idx, size_t * len, char separator);

const char *mmap_get_line2(mmap_wrapper_t *mw, size_t * idx, size_t * len, const char *separator);

off_t mmap_wrapper_get_abs_pos(mmap_wrapper_t *mw, size_t idx);

#endif /* MMAP_WRAPPER_H */

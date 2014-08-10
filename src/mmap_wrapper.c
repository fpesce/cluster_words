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

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "mmap_wrapper.h"

#define DEF_MULT_WIN 1024

struct mmap_wrapper_t
{
    off_t offset;
    off_t fsize;
    int fd;
    void *mm;
    size_t msize;
    long page_size;
    unsigned char last_window;
};

extern int mmap_wrapper_init(mmap_wrapper_t **mw, const char *fname)
{
    mmap_wrapper_t *result;

    *mw = result = malloc(sizeof(struct mmap_wrapper_t));
    result->fd = open(fname, O_RDONLY);
    if (0 > result->fd) {
	perror("error occured calling open");
	return -1;
    }

    result->fsize = 0;
    result->fsize = lseek(result->fd, 0, SEEK_END);
    if ((off_t) - 1 == result->fsize) {
	perror("error calling lseek");
	close(result->fd);
	free(result);
	return -1;
    }

    if (0 == result->fsize) {
	fprintf(stderr, "ignoring empty file\n");
	close(result->fd);
	free(result);
	return -1;
    }

    result->page_size = sysconf(_SC_PAGE_SIZE);
    result->offset = 0;
    result->msize = result->page_size * DEF_MULT_WIN;

    if (result->msize > result->fsize) {
	result->msize = result->fsize;
	result->last_window = 1;
    }
    else {
	result->last_window = 0;
    }

    result->mm = mmap(NULL, result->msize, PROT_READ, MAP_PRIVATE, result->fd, result->offset);
    if (result->mm == (void *) -1) {
	perror("error calling mmap");
	close(result->fd);
	free(result);
	return -1;
    }

    return 0;
}

static inline const char *mmap_wrapper_get_ptr(mmap_wrapper_t *mw)
{
    return (const char *) mw->mm;
}

static inline size_t mmap_wrapper_get_limit(const mmap_wrapper_t *mw)
{
    return mw->msize;
}

static inline unsigned char mmap_wrapper_last_window(const mmap_wrapper_t *mw)
{
    return mw->last_window;
}

static inline int mmap_wrapper_move_window(mmap_wrapper_t *mw, size_t last_needed_ref, size_t * new_equivalent_ref)
{
    off_t absolute_ref;
    int rv;

    absolute_ref = last_needed_ref + mw->offset;
    mw->offset += mw->msize;
    while (mw->offset > absolute_ref)
	mw->offset -= mw->page_size;

    *new_equivalent_ref = absolute_ref - mw->offset;

    rv = munmap(mw->mm, mw->msize);
    if (0 != rv) {
	perror("error calling munmap");
	return -1;
    }

    mw->msize = mw->page_size * DEF_MULT_WIN;
    if ((mw->msize + mw->offset) > mw->fsize) {
	mw->msize = mw->fsize - mw->offset;
	mw->last_window = 1;
    }
    else {
	mw->last_window = 0;
    }

    mw->mm = mmap(NULL, mw->msize, PROT_READ, MAP_PRIVATE, mw->fd, mw->offset);
    if (mw->mm == (void *) -1) {
	perror("error calling mmap");
	return -1;
    }

    return 0;
}

static inline int mmap_wrapper_check_overflow(mmap_wrapper_t *mw, size_t * idx, size_t inc, const char **ptr)
{
    int rv;

    if ((*idx + inc) > mmap_wrapper_get_limit(mw)) {
	size_t new_idx;

	if ((*idx + inc) > mw->fsize) {
	    fprintf(stderr, "try to read over the end of a file\n");
	    return -1;
	}

	rv = mmap_wrapper_move_window(mw, *idx, &new_idx);
	if (0 != rv) {
	    fprintf(stderr, "error calling mmap_wrapper_move_window\n");
	    return -1;
	}
	*idx = new_idx;
	*ptr = mmap_wrapper_get_ptr(mw);
    }

    return 0;
}

extern int mmap_wrapper_delete(mmap_wrapper_t *mw)
{
    int rv;

    rv = munmap(mw->mm, mw->msize);
    if (0 != rv) {
	perror("error calling munmap");
	return -1;
    }

    rv = close(mw->fd);
    if (0 != rv) {
	perror("error calling close");
	return -1;
    }

    return 0;
}

static inline size_t my_memcharcspn(const char *s, int limit, size_t len)
{
    const char *res;

    res = memchr(s, limit, len);

    return (NULL == res) ? len : (res - s);
}

extern int mmap_get_head(mmap_wrapper_t *mw, size_t * idx)
{
    if ((0 == mmap_wrapper_get_limit(mw)) && (0 != mmap_wrapper_last_window(mw))) {
	fprintf(stderr, "file is empty\n");
	return EOF;
    }
    *idx = 0;

    return 0;
}

extern int mmap_get_next(mmap_wrapper_t *mw, size_t * idx, char separator)
{
    const char *ptr;
    size_t wordlen;
    int found = 0;
    int rv;

    while ((((*idx < mmap_wrapper_get_limit(mw)) || (0 == mmap_wrapper_last_window(mw)))) && (!found)) {
	ptr = mmap_wrapper_get_ptr(mw);
	wordlen = my_memcharcspn(ptr + *idx, separator, mmap_wrapper_get_limit(mw) - *idx);
	*idx += wordlen + 1;

	if ((mmap_wrapper_get_limit(mw) <= *idx)) {
	    if (0 == mmap_wrapper_last_window(mw)) {
		size_t new_idx;

		*idx -= wordlen + 1;
		rv = mmap_wrapper_move_window(mw, *idx, &new_idx);
		if (0 != rv) {
		    fprintf(stderr, "error calling mmap_wrapper_move_window\n");
		    return -1;
		}
		*idx = new_idx;
	    }
	}
	else {
	    found = 1;
	}
    }

    if (!found) {
	return EOF;
    }

    return 0;
}

extern int mmap_get_next2(mmap_wrapper_t *mw, size_t * idx, const char *separators)
{
    const char *ptr;
    size_t wordlen;
    int found = 0;
    int rv;

    while ((((*idx < mmap_wrapper_get_limit(mw)) || (0 == mmap_wrapper_last_window(mw)))) && (!found)) {
	ptr = mmap_wrapper_get_ptr(mw);
	wordlen = strcspn(ptr + *idx, separators);
	*idx += wordlen + 1;

	if ((mmap_wrapper_get_limit(mw) <= *idx)) {
	    if (0 == mmap_wrapper_last_window(mw)) {
		size_t new_idx;

		*idx -= wordlen + 1;
		rv = mmap_wrapper_move_window(mw, *idx, &new_idx);
		if (0 != rv) {
		    fprintf(stderr, "error calling mmap_wrapper_move_window\n");
		    return -1;
		}
		*idx = new_idx;
	    }
	}
	else {
	    found = 1;
	}
    }

    if (!found) {
	return EOF;
    }

    return 0;
}

extern const char *mmap_get_line(mmap_wrapper_t *mw, size_t * idx, size_t * len, char separator)
{
    const char *ptr;
    int found = 0;
    int i;
    int rv;

    *len = 0;
    ptr = mmap_wrapper_get_ptr(mw);
    for (i = 1; ((*idx < mmap_wrapper_get_limit(mw)) || (0 == mmap_wrapper_last_window(mw))) && (!found); i++) {
	*len = my_memcharcspn(ptr + *idx, separator, mmap_wrapper_get_limit(mw) - *idx);

	if ((mmap_wrapper_get_limit(mw) == (*idx + *len))
	    && (0 == mmap_wrapper_last_window(mw))) {
	    size_t new_idx;

	    rv = mmap_wrapper_move_window(mw, *idx, &new_idx);
	    if (0 != rv) {
		fprintf(stderr, "error calling mmap_wrapper_move_window\n");
		return NULL;
	    }
	    *idx = new_idx;
	    ptr = mmap_wrapper_get_ptr(mw);
	}
	else {
	    found = 1;
	}
    }

    return ptr + *idx;
}

extern const char *mmap_get_line2(mmap_wrapper_t *mw, size_t * idx, size_t * len, const char *separators)
{
    const char *ptr;
    int found = 0;
    int i;
    int rv;

    *len = 0;
    ptr = mmap_wrapper_get_ptr(mw);
    for (i = 1; ((*idx < mmap_wrapper_get_limit(mw)) || (0 == mmap_wrapper_last_window(mw))) && (!found); i++) {
	*len = strcspn(ptr + *idx, separators);

	if ((mmap_wrapper_get_limit(mw) == (*idx + *len))
	    && (0 == mmap_wrapper_last_window(mw))) {
	    size_t new_idx;

	    rv = mmap_wrapper_move_window(mw, *idx, &new_idx);
	    if (0 != rv) {
		fprintf(stderr, "error calling mmap_wrapper_move_window\n");
		return NULL;
	    }
	    *idx = new_idx;
	    ptr = mmap_wrapper_get_ptr(mw);
	}
	else {
	    found = 1;
	}
    }

    return ptr + *idx;
}

extern off_t mmap_wrapper_get_abs_pos(mmap_wrapper_t *mw, size_t idx)
{
    off_t result;

    result = idx + mw->offset;

    return result;
}

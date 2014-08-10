#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "levenshtein.h"

static size_t levenshtein_rec(size_t * dbuf, size_t i, size_t j, const char *s1, size_t l1, const char *s2, size_t l2,
			      unsigned char *err_min)
{
    size_t cur, path_diag, path_right, path_down, i_1, j_1, pos, cur_1, min;

    cur = dbuf[l2 * i + j];
    cur_1 = cur + 1;
    i_1 = i + 1;
    j_1 = j + 1;
    path_diag = path_right = path_down = (size_t) - 1;
    pos = (l2 * i_1) + j_1;
    if ((i_1 < l1) && (j_1 < l2)) {
	if (tolower(s1[i + 1]) == tolower(s2[j + 1])) {
	    if (dbuf[pos] > cur) {
		dbuf[pos] = cur;
		path_diag = levenshtein_rec(dbuf, i + 1, j + 1, s1, l1, s2, l2, err_min);
	    }
	}
	else {
	    if (cur_1 < *err_min) {
		if (dbuf[pos] > cur_1) {
		    dbuf[pos] = cur_1;
		    path_diag = levenshtein_rec(dbuf, i + 1, j + 1, s1, l1, s2, l2, err_min);
		}
	    }
	}
    }
    else {
	if ((i_1 == l1) && (j_1 == l2)) {
	    path_diag = cur;
	    if (cur < *err_min)
		*err_min = cur;
	}
    }
    min = path_diag;
    pos = (l2 * i_1) + j;
    if ((i_1 < l1) && (j < l2)) {
	if (cur_1 < *err_min) {
	    if (dbuf[pos] > cur_1) {
		dbuf[pos] = cur_1;
		path_right = levenshtein_rec(dbuf, i + 1, j, s1, l1, s2, l2, err_min);
	    }
	}
    }
    else {
	if ((i_1 == l1) && (j == l2)) {
	    path_right = cur;
	    if (cur < *err_min)
		*err_min = cur;
	}
    }
    if (path_right < min)
	min = path_right;
    pos = (l2 * i) + j_1;
    if ((i < l1) && (j_1 < l2)) {
	if (cur_1 < *err_min) {
	    if (dbuf[pos] > cur_1) {
		dbuf[pos] = cur_1;
		path_down = levenshtein_rec(dbuf, i, j + 1, s1, l1, s2, l2, err_min);
	    }
	}
    }
    else {
	if ((i == l1) && (j_1 == l2)) {
	    path_down = cur;
	    if (cur < *err_min)
		*err_min = cur;
	}
    }
    if (path_down < min)
	min = path_down;

    return min;
}

extern size_t levenshtein_distance2(const char *s1, size_t l1, const char *s2, size_t l2, unsigned char *err_min)
{
    size_t tmpbuf[1024], *dbuf, l1l2, l2_1, l1_1;

    if (*err_min > 6) {
	l1l2 = levenshtein_distance(s1, l1, s2, l2, *err_min);
	if (l1l2 < *err_min)
	    *err_min = l1l2;
	return l1l2;
    }

    l1_1 = l1 + 1;
    l2_1 = l2 + 1;
    l1l2 = (l1_1 * l2_1);
    if (l1l2 > (sizeof(tmpbuf) / sizeof(size_t))) {
	dbuf = malloc(l1l2 * sizeof(size_t));
    }
    else {
	dbuf = tmpbuf;
    }
    memset(dbuf, (size_t) - 1, l1l2 * sizeof(size_t));

    if (*s1 == *s2)
	dbuf[0] = 0;
    else
	dbuf[0] = 1;

    return levenshtein_rec(dbuf, 0, 0, s1, l1, s2, l2, err_min);
}

#define MIN3(a,b,c) (((a)<(b))?(((a)<(c))?(a):(((b)<(c))?(b):(c))):(((b)<(c))?(b):(((a)<(c))?(a):(c))))
extern size_t levenshtein_distance(const char *s1, size_t l1, const char *s2, size_t l2, unsigned char err_min)
{
    size_t tmpbuf[1024], *dbuf, l1l2, l2_1, l1_1, i, j, cost;

    l1_1 = l1 + 1;
    l2_1 = l2 + 1;
    l1l2 = (l1_1 * l2_1);
    if (l1l2 > (sizeof(tmpbuf) / sizeof(size_t))) {
	dbuf = malloc(l1l2 * sizeof(size_t));
    }
    else {
	dbuf = tmpbuf;
    }
    memset(dbuf, 0, l1l2 * sizeof(size_t));

    for (i = 0; i < l1_1; i++)
	dbuf[l2_1 * i] = i;
    for (j = 0; j < l2_1; j++)
	dbuf[j] = j;

    for (i = 1; i < l1_1; i++)
	for (j = 1; j < l2_1; j++) {
	    if (s1[i - 1] == s2[j - 1])
		cost = 0;
	    else
		cost = 1;
	    dbuf[l2_1 * i + j] =
		MIN3(dbuf[l2_1 * (i - 1) + j] + 1, dbuf[l2_1 * i + (j - 1)] + 1, dbuf[l2_1 * (i - 1) + (j - 1)] + cost);
	}

    return dbuf[(l1l2) - 1];
}

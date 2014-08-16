#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "levenshtein.h"

#define MIN3(a,b,c) (((a)<(b))?(((a)<(c))?(a):(((b)<(c))?(b):(c))):(((b)<(c))?(b):(((a)<(c))?(a):(c))))
static inline size_t levenshtein_distance_internal(const char *s1, size_t l1, const char *s2, size_t l2, size_t *zsize)
{
    size_t tmpbuf[1024], *dbuf;
    size_t l1l2, l2_1, l1_1, cost, result;
    int i, j, k;

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

    dbuf[0] = 0;
    for (i = 1; i < l1_1; i++)
        dbuf[l2_1 * i] = i;

    for (j = 1; j < l2_1; j++) {
        dbuf[j] = j;
        for (i = 1; i < l1_1; i++) {
            size_t above, left, diag;

            if (tolower(s1[i - 1]) != tolower(s2[j - 1])) {
                above = dbuf[l2_1 * (i - 1) + j] + 1;
                left = dbuf[l2_1 * i + (j - 1)] + 1;
                diag = dbuf[l2_1 * (i - 1) + (j - 1)] + 1;
                dbuf[l2_1 * i + j] = MIN3(above, left, diag);
            } else {
                dbuf[l2_1 * i + j] = dbuf[l2_1 * (i - 1) + (j - 1)];
            }
        }
    }
    result = dbuf[(l1l2) - 1];
    /* The following code computes an optimal alignment length */
    i = l1;
    j = l2;
    *zsize = l1_1 + l2_1;
    k = 0;
    while ((i != -1) && (j != -1)) {
        cost = (tolower(s1[i - 1]) == tolower(s2[j - 1])) ? 0 : 1;
        if (dbuf[l2_1 * i + j] == (dbuf[l2_1 * (i - 1) + (j - 1)] + cost)) {
            i--, j--, k++;
        } else if (dbuf[l2_1 * i + j] == (dbuf[l2_1 * (i - 1) + j] + 1)) {
                i--, k++;
        } else {
                j--, k++;
        }
    }
    while (i != -1) {
        i--, k++;
    }
    while (j != -1) {
        j--, k++;
    }
    *zsize = k - 1;

    if(dbuf != tmpbuf)
        free(dbuf);

    return result;
}

extern size_t levenshtein_distance(const char *s1, size_t l1, const char *s2, size_t l2)
{
    size_t zsize;
    return levenshtein_distance_internal(s1, l1, s2, l2, &zsize);
}

extern float levenshtein_norm_distance(const char *s1, size_t l1, const char *s2, size_t l2)
{
    size_t lev_d;
    size_t zsize;
    float result;
    /*
     * Goal is to align 2 strings:
     * _SOURCESTR   10 (norm aligned len)
     * DST____STR   10 (norm aligned len)
     * I SDDDD    = 6
     * Differences are sum of substitution/deletion/insertion
     * Normalized distance is (norm_len - 6) / norm_len; => 40% in this example
     */

    lev_d = levenshtein_distance_internal(s1, l1, s2, l2, &zsize);

    result = (zsize - lev_d) / (float) zsize;

    return result;
}

/*
int main(int argc, const char **argv)
{
    fprintf(stdout, "a: %f\n", levenshtein_norm_distance("ERDAWCQPGKWY", strlen("ERDAWCQPGKWY"), "EAWACQGKL", strlen("EAWACQGKL")));
    fprintf(stdout, "b: %f\n", levenshtein_norm_distance("SOURCESTR", strlen("SOURCESTR"), "DSTSTR", strlen("DSTSTR")));
    fprintf(stdout, "c: %f\n", levenshtein_norm_distance("Saturday", strlen("Saturday"), "Sunday", strlen("Sunday")));
    return 0;
}
*/

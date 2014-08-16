/*
 * Copyright (C) 2011 François Pesce : francois.pesce (at) gmail (dot) com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>

#include "list.h"

struct cell_t
{
    cell_t *next;
    void *data;
};

struct list_t
{
    cell_t *head;
    cell_t *tail;
    int nb_cells;
};

extern list_t *list_make(void)
{
    list_t *list = malloc(sizeof(struct list_t));

    if (list != NULL) {
        list->head = NULL;
        list->tail = NULL;
        list->nb_cells = 0;
    }

    return list;
}

extern void list_release_container(list_t *list)
{
    free(list);
}

extern int list_cons(list_t *list, void *element)
{
    cell_t *cell;
    int rc; /** return code. */

    if (NULL != (cell = (cell_t *) malloc(sizeof(struct cell_t)))) {
        cell->data = element;
        cell->next = list->head;
        list->head = cell;
        list->nb_cells += 1;

        if (list->nb_cells == 1)
            list->tail = list->head;

        rc = 0;
    }
    else {
        rc = -1;
    }

    return rc;
}

extern void list_remove(list_t *list, void *element)
{
    cell_t *cell;

    for(cell = list->head; cell != NULL; cell = cell->next) {
        if ((NULL != cell->next) && (cell->next->data == element)) {
            if (list->tail == cell->next) {
                list->tail = cell;
            }
            cell->next = cell->next->next;
        }
    }
}

extern int list_enqueue_elt(list_t *list, void *element)
{
    cell_t *cell;
    int rc; /** return code. */

    if (NULL != (cell = (cell_t *) malloc(sizeof(struct cell_t)))) {
        cell->data = element;
        cell->next = NULL;
        if (NULL != list->tail) {
            list->tail->next = cell;
        }
        list->tail = cell;
        list->nb_cells += 1;

        if (list->nb_cells == 1)
            list->head = list->tail;

        rc = 0;
    }
    else {
        rc = -1;
    }

    return rc;
}

extern void list_cdr(list_t *list)
{
    void *cell;

    cell = list->head->next;
    list->nb_cells -= 1;
    free(list->head);
    list->head = cell;
}

extern void list_delete(list_t *list)
{
    while (list->head != NULL)
        list_cdr(list);
}

extern void list_enqueue(list_t *list, list_t *list2)
{
    list->nb_cells += list2->nb_cells;
    if (NULL != list->head) {
        list->tail->next = list2->head;
    }
    else {
        list->head = list->tail = list2->head;
    }
    list->tail = list2->tail;

    return;
}

extern cell_t *list_first(list_t *list)
{
    return list->head;
}

extern cell_t *list_last(list_t *list)
{
    return list->tail;;
}

extern cell_t *list_next(cell_t *cell)
{
    return cell->next;
}

extern void *list_get(cell_t *cell)
{
    return cell->data;
}

/*
 * Copyright (C) 2011 François Pesce : francois.pesce (at) gmail (dot) com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LIST_H
#define LIST_H

typedef struct list_t list_t;
typedef struct cell_t cell_t;

list_t *list_make(void);

void list_release_container(list_t *list);

int list_cons(list_t *list, void *element);

void list_remove(list_t *list, void *element);

int list_enqueue_elt(list_t *list, void *element);

void list_cdr(list_t *list);

void list_delete(list_t *list);

void list_enqueue(list_t *list, list_t *list2);

cell_t *list_first(list_t *list);

cell_t *list_next(cell_t *cell);

void *list_get(cell_t *cell);

#endif /* LIST_H */

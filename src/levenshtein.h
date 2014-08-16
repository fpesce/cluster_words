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

#ifndef LEVENSHTEIN_H
#define LEVENSHTEIN_H

#define MAX(a,b) (((a) > (b)) ? (a) : (b))

size_t levenshtein_distance(const char *s1, size_t l1, const char *s2, size_t l2);

float levenshtein_norm_distance(const char *s1, size_t l1, const char *s2, size_t l2);

#endif /* LEVENSHTEIN_H */

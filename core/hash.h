/*
 *   Copyright (c) 2023 Anton Kundenko <singaraiona@gmail.com>
 *   All rights reserved.

 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:

 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.

 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 */

#ifndef HASH_H
#define HASH_H

#include "rayforce.h"

#define STATE_EMPTY 0
#define STATE_OCCUPIED 1
#define STATE_DELETED 2

typedef struct bucket_t
{
    null_t *key;
    null_t *val;
    u32_t distance;
    u32_t state;
} bucket_t;

typedef struct hash_table_t
{
    u64_t (*hasher)(null_t *a);
    i32_t (*compare)(null_t *a, null_t *b);
    u64_t size;
    u64_t count;
    bucket_t *buckets;
} hash_table_t;

hash_table_t *ht_new(u64_t size, u64_t (*hasher)(null_t *a), i32_t (*compare)(null_t *a, null_t *b));
null_t ht_free(hash_table_t *table);

null_t *ht_insert(hash_table_t *table, null_t *key, null_t *val);
null_t *ht_insert_with(hash_table_t *table, null_t *key, null_t *val, null_t *(*func)(null_t *key, null_t *val, bucket_t *bucket));
bool_t ht_update(hash_table_t *table, null_t *key, null_t *val);
null_t *ht_get(hash_table_t *table, null_t *key);

#endif

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

#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "hash.h"
#include "rayforce.h"
#include "heap.h"
#include "util.h"

typedef struct b_t
{
    i64_t k;
    i64_t v;
} b_t;

#define as_bt(x) ((b_t *)as_string(x))

obj_t ht_tab(u64_t size)
{
    u64_t i;
    obj_t obj;

    size = next_power_of_two_u64(size);
    obj = vector(TYPE_I64, size * 2);

    for (i = 0; i < size; i++)
        as_bt(obj)[i].k = NULL_I64;

    return obj;
}

obj_t ht_set(u64_t size)
{
    u64_t i;
    obj_t obj;

    size = next_power_of_two_u64(size);
    obj = vector(TYPE_I64, size);

    for (i = 0; i < size; i++)
        as_i64(obj)[i] = NULL_I64;

    return obj;
}

nil_t rehash_tab(obj_t *obj, hash_f hash)
{
    printf("REHASH TAB!!!!");
    u64_t i, l, size = (*obj)->len, key, val, factor, index;
    obj_t new_obj = ht_tab(size); // will multiply size by 2

    factor = new_obj->len - 1;

    for (i = 0; i < size / 2; i++)
    {
        if (as_bt(*obj)[i].k != NULL_I64)
        {
            key = as_bt(*obj)[i].k;
            val = as_bt(*obj)[i].v;
            index = hash ? hash(key) & factor : key & factor;

            while (as_bt(new_obj)[i].k != NULL_I64)
            {
                if (index == size)
                    panic("hash tab is full!!");

                index = index + 1;
            }

            as_bt(new_obj)[index].k = key;
            as_bt(new_obj)[index].v = val;
        }
    }

    drop(*obj);

    *obj = new_obj;
}

nil_t rehash_set(obj_t *obj, hash_f hash)
{
    u64_t i, l, size = (*obj)->len, key, factor, index;
    obj_t new_obj = ht_set(size * 2);

    factor = new_obj->len - 1;

    for (i = 0; i < size; i++)
    {
        if (as_i64(*obj)[i] != NULL_I64)
        {
            key = as_bt(*obj)[i].k;
            index = hash ? hash(key) & factor : key & factor;

            while (as_i64(new_obj)[i] != NULL_I64)
            {
                if (index == size * 2)
                    panic("hash set is full!!");

                index = index + 1;
            }

            as_i64(new_obj)[index] = key;
        }
    }

    drop(*obj);

    *obj = new_obj;
}

i64_t *ht_tab_get(obj_t *obj, i64_t key)
{
    u64_t i, size;
    b_t *b;

    while (true)
    {
        size = (*obj)->len / 2;

        for (i = key & (size - 1); i < size; i++)
        {
            b = as_bt(*obj) + i;
            if (b->k == NULL_I64 || b->k == key)
                return b;
        }

        rehash_tab(obj, NULL);
    }
}

i64_t *ht_tab_get_with(obj_t *obj, i64_t key, hash_f hash, cmp_f cmp)
{
    u64_t i, size;
    b_t *b;

    while (true)
    {
        size = (*obj)->len / 2;

        for (i = hash(key) & (size - 1); i < size; i++)
        {
            b = as_bt(*obj) + i;
            if (b->k == NULL_I64 || (cmp(b->k, key) == 0))
                return b;
        }

        rehash_tab(obj, hash);
    }
}

i64_t *ht_set_get(obj_t *obj, i64_t key)
{
    u64_t i, size;
    i64_t *b;

    while (true)
    {
        size = (*obj)->len;

        for (i = key & (size - 1); i < size; i++)
        {
            b = as_i64(*obj) + i;
            if (b[0] == NULL_I64 || b[0] == key)
                return b;
        }

        rehash_set(obj, NULL);
    }
}

i64_t *ht_set_get_with(obj_t *obj, i64_t key, hash_f hash, cmp_f cmp)
{
    u64_t i, size;
    i64_t *b;

    while (true)
    {
        size = (*obj)->len;

        for (i = hash(key) & (size - 1); i < size; i++)
        {
            b = as_i64(*obj) + i;
            if (b[0] == NULL_I64 || (cmp(b[0], key) == 0))
                return b;
        }

        rehash_set(obj, hash);
    }
}

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

#include "rayforce.h"
#include "alloc.h"
#include "vector.h"
#include "util.h"

/*
 * Each vector capacity is always factor of 8
 * This allows to avoid storing capacity in vector
 */
#define CAPACITY_FACTOR 8

/*
 * Aligns x to the nearest multiple of a
 */
#define alignup(x, a) (((x) + (a)-1) & ~((a)-1))

/*
 * Calculates capacity for vector of length x
 */
#define capacity(x) (alignup(x, CAPACITY_FACTOR))

/*
 * Appends value to the end of vector (dynamically grows vector if needed)
 */
#define push(vector, type, value)                                                                    \
    i64_t len = (vector)->list.len;                                                                  \
    i64_t cap = capacity(len);                                                                       \
    if (cap == 0)                                                                                    \
        (vector)->list.ptr = rayforce_malloc(CAPACITY_FACTOR * sizeof(type));                        \
    else if (cap == len)                                                                             \
        (vector)->list.ptr = rayforce_realloc((vector)->list.ptr, capacity(len + 1) * sizeof(type)); \
    ((type *)((vector)->list.ptr))[(vector)->list.len++] = (value);

#define pop(vector, type) ((type *)((vector)->list.ptr))[(vector)->list.len--]

/*
 * Attemts to make vector from list if all elements are of the same type
 */
#define flatten(list, vec, fpush, mem)            \
    {                                             \
        rf_object_t *member;                      \
        vec = vector_##mem(0);                    \
                                                  \
        for (u64_t i = 0; i < list.list.len; i++) \
        {                                         \
            member = &as_list(&list)[i];          \
                                                  \
            if (member->type != type)             \
            {                                     \
                value_free(&vec);                 \
                return list;                      \
            }                                     \
                                                  \
            fpush(&vec, member->mem);             \
        }                                         \
    }

extern rf_object_t vector(i8_t type, u8_t size_of_val, i64_t len)
{
    rf_object_t v = {
        .type = type,
        .list = {
            .ptr = NULL,
            .len = len,
        },
    };

    if (len == 0)
        return v;

    v.list.ptr = rayforce_malloc(size_of_val * capacity(len));

    return v;
}

extern u64_t vector_i64_push(rf_object_t *vector, i64_t value)
{
    push(vector, i64_t, value);
    return vector->list.len;
}

extern u64_t vector_i64_pop(rf_object_t *vector)
{
    pop(vector, i64_t);
    return vector->list.len;
}

extern u64_t vector_f64_push(rf_object_t *vector, f64_t value)
{
    push(vector, f64_t, value);
    return vector->list.len;
}

extern f64_t vector_f64_pop(rf_object_t *vector)
{
    return pop(vector, f64_t);
}

extern u64_t list_push(rf_object_t *list, rf_object_t value)
{
    push(list, rf_object_t, value);
    return list->list.len;
}

extern rf_object_t list_pop(rf_object_t *list)
{
    return pop(list, rf_object_t);
}

extern u64_t vector_push(rf_object_t *vector, rf_object_t value)
{
    i8_t type = vector->type;

    switch (type)
    {
    case TYPE_I64:
        vector_i64_push(vector, value.i64);
        break;
    case TYPE_F64:
        vector_f64_push(vector, value.f64);
        break;
    case TYPE_LIST:
        list_push(vector, value);
        break;
    default:
        return vector->list.len;
    }

    return vector->list.len;
}

extern u64_t vector_i64_find(rf_object_t *vector, i64_t key)
{
    i64_t *ptr = as_vector_i64(vector);

    for (u64_t i = 0; i < vector->list.len; i++)
    {
        if (ptr[i] == key)
            return i;
    }

    return vector->list.len;
}

extern u64_t vector_f64_find(rf_object_t *vector, f64_t key)
{
    f64_t *ptr = as_vector_f64(vector);

    for (u64_t i = 0; i < vector->list.len; i++)
    {
        if (ptr[i] == key)
            return i;
    }

    return vector->list.len;
}

extern u64_t list_find(rf_object_t *list, rf_object_t key)
{
    rf_object_t *ptr = as_list(list);

    for (u64_t i = 0; i < list->list.len; i++)
    {
        if (value_eq(&ptr[i], &key))
            return i;
    }

    return list->list.len;
}

extern u64_t vector_find(rf_object_t *vector, rf_object_t key)
{
    i8_t type = vector->type;

    switch (type)
    {
    case TYPE_I64:
        return vector_i64_find(vector, key.i64);
    case TYPE_F64:
        return vector_f64_find(vector, key.f64);
    case TYPE_SYMBOL:
        return vector_i64_find(vector, key.i64);
    default:
        return list_find(vector, key);
    }
}

/*
 * Try to flatten list in a vector if all elements are of the same type
 */
extern rf_object_t list_flatten(rf_object_t list)
{
    if (list.type != TYPE_LIST)
        return list;

    u64_t len = list.list.len;
    i8_t type;
    rf_object_t vec;

    if (len == 0)
        return list;

    type = as_list(&list)[0].type;

    // Only scalar types can be flattened
    if (type > -1)
        return list;

    switch (type)
    {
    case -TYPE_I64:
        flatten(list, vec, vector_i64_push, i64);
        break;
    case -TYPE_F64:
        flatten(list, vec, vector_f64_push, f64);
        break;
    case -TYPE_SYMBOL:
        flatten(list, vec, vector_i64_push, i64);
        if (vec.type == TYPE_I64)
            vec.type = TYPE_SYMBOL;
        break;
    default:
        return list;
    }

    value_free(&list);

    return vec;
}
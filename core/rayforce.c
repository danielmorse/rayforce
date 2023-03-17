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

#include <stdio.h>
#include "rayforce.h"
#include "format.h"
#include "alloc.h"
#include "string.h"
#include "vector.h"
#include "dict.h"

extern rf_object_t error(i8_t code, str_t message)
{
    rf_object_t err = dict(vector_symbol(0), list(0));
    dict_set(&err, symbol("code"), i64(code));
    dict_set(&err, symbol("message"), string_from_str(message));

    err.type = TYPE_ERROR;

    return err;
}

extern rf_object_t i64(i64_t object)
{
    rf_object_t scalar = {
        .type = -TYPE_I64,
        .i64 = object,
    };

    return scalar;
}

extern rf_object_t f64(f64_t object)
{
    rf_object_t scalar = {
        .type = -TYPE_F64,
        .f64 = object,
    };

    return scalar;
}

extern rf_object_t symbol(str_t ptr)
{
    // Do not allocate new string - it would be done by symbols_intern (if needed)
    rf_object_t string = str(ptr, strlen(ptr));
    string.list.ptr = ptr;
    i64_t id = symbols_intern(&string);
    rf_object_t sym = {
        .type = -TYPE_SYMBOL,
        .i64 = id,
    };

    return sym;
}

extern rf_object_t null()
{
    rf_object_t list = {
        .type = TYPE_LIST,
        .list = {
            .ptr = NULL,
            .len = 0,
        },
    };

    return list;
}

extern rf_object_t table(rf_object_t keys, rf_object_t vals)
{
    if (keys.type != TYPE_SYMBOL || vals.type != 0)
        return error(ERR_TYPE, "Keys must be a symbol vector and objects must be list");

    if (keys.list.len != vals.list.len)
        return error(ERR_LENGTH, "Keys and objects must have the same length");

    // rf_object_t *v = as_list(&vals);
    // i64_t len = 0;

    // for (i64_t i = 0; i < v.list.len; i++)
    // {
    //     if (v[i].type < 0)
    //         return error(ERR_TYPE, "Values must be scalars");
    // }

    rf_object_t table = list(2);

    as_list(&table)[0] = keys;
    as_list(&table)[1] = vals;

    table.type = TYPE_TABLE;

    return table;
}

extern rf_object_t object_clone(rf_object_t *object)
{
    switch (object->type)
    {
    case TYPE_I64:
        return *object;
    case TYPE_F64:
        return *object;
    default:
    {
        // printf("** Clone: Invalid type\n");
        return *object;
    }
    }
}

extern i8_t object_eq(rf_object_t *a, rf_object_t *b)
{
    i64_t i;

    if (a->type != b->type)
        return 0;

    if (a->type == -TYPE_I64 || a->type == -TYPE_SYMBOL)
        return a->i64 == b->i64;
    else if (a->type == -TYPE_F64)
        return a->f64 == b->f64;
    else if (a->type == TYPE_I64 || a->type == TYPE_SYMBOL)
    {
        if (as_vector_i64(a) == as_vector_i64(b))
            return 1;
        if (a->list.len != b->list.len)
            return 0;
        for (i = 0; i < a->list.len; i++)
        {
            if (as_vector_i64(a)[i] != as_vector_i64(b)[i])
                return 0;
        }
        return 1;
    }
    else if (a->type == TYPE_F64)
    {
        if (as_vector_f64(a) == as_vector_f64(b))
            return 1;
        if (a->list.len != b->list.len)
            return 0;
        for (i = 0; i < a->list.len; i++)
        {
            if (as_vector_f64(a)[i] != as_vector_f64(b)[i])
                return 0;
        }
        return 1;
    }

    // printf("** Eq: Invalid type\n");
    return 0;
}

extern null_t object_free(rf_object_t *object)
{
    switch (object->type)
    {
    case TYPE_I64:
    {
        rayforce_free(object->list.ptr);
        break;
    }
    case TYPE_F64:
    {
        rayforce_free(object->list.ptr);
        break;
    }
    default:
    {
        // printf("** Free: Invalid type\n");
        break;
    }
    }
}

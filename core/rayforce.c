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
#include <stdatomic.h>
#include "rayforce.h"
#include "format.h"
#include "alloc.h"
#include "string.h"
#include "vector.h"
#include "dict.h"
#include "util.h"
#include "string.h"
#include "runtime.h"

CASSERT(sizeof(struct object_t) == 32, rayforce_h)

object_t atom(type_t type)
{
    object_t a = (object_t)rf_malloc(sizeof(struct object_t));

    a->type = -type;
    a->rc = 1;

    return a;
}

object_t list(i64_t len, ...)
{
    i32_t i;
    object_t l = (object_t)rf_malloc(sizeof(struct object_t));

    l->type = TYPE_LIST;
    l->rc = 1;
    l->ptr = rf_malloc(sizeof(object_t) * len);

    va_list args;
    va_start(args, len);

    for (i = 0; i < len; i++)
        l->ptr[i] = va_arg(args, object_t);

    va_end(args);

    return l;
}

object_t error(i8_t code, str_t message)
{
    object_t err = rf_malloc(sizeof(struct object_t));

    err->type = TYPE_ERROR;
    err->len = strlen(message);
    err->ptr = message;

    return err;
}

object_t null()
{
    return NULL;
}

object_t bool(bool_t val)
{
    object_t b = atom(TYPE_BOOL);
    b->bool = val;
    return b;
}

object_t i64(i64_t val)
{
    object_t i = atom(TYPE_vector_i64);
    i->i64 = val;
    return i;
}

object_t f64(f64_t val)
{
    object_t f = atom(TYPE_vector_f64);
    f->f64 = val;
    return f;
}

object_t symbol(str_t s)
{
    i64_t id = intern_symbol(s, strlen(s));
    object_t a = atom(TYPE_SYMBOL);
    a->i64 = id;

    return a;
}

object_t symboli64(i64_t id)
{
    object_t a = atom(TYPE_SYMBOL);
    a->i64 = id;

    return a;
}

object_t guid(u8_t data[])
{

    // if (data == NULL)
    //     return guid;

    // guid_t *g = (guid_t *)rf_malloc(sizeof(struct guid_t));
    // memcpy(g->data, data, sizeof(guid_t));

    // guid.guid = g;

    // return guid;

    return NULL;
}

object_t schar(char_t c)
{
    object_t s = atom(TYPE_CHAR);
    s->schar = c;
    return s;
}

object_t timestamp(i64_t val)
{
    object_t t = atom(TYPE_TIMESTAMP);
    t->i64 = val;
    return t;
}

object_t table(object_t keys, object_t vals)
{
    object_t t = list(2, keys, vals);
    t->type = TYPE_TABLE;

    return t;
}

object_t dict(object_t keys, object_t vals)
{
    object_t dict;

    if (!is_vector(keys) || !is_vector(vals))
        return error(ERR_TYPE, "Keys and Values must be lists");

    if (keys->len != vals->len)
        return error(ERR_LENGTH, "Keys and Values must have the same length");

    dict = list(2, keys, vals);
    dict->type = TYPE_DICT;

    return dict;
}

bool_t is_null(object_t object)
{
    return (object->type == TYPE_LIST && object->ptr == NULL) ||
           (object->type == -TYPE_vector_i64 && object->i64 == NULL_vector_i64) ||
           (object->type == -TYPE_SYMBOL && object->i64 == NULL_vector_i64) ||
           (object->type == -TYPE_vector_f64 && object->f64 == NULL_vector_f64) ||
           (object->type == -TYPE_TIMESTAMP && object->i64 == NULL_vector_i64) ||
           (object->type == -TYPE_CHAR && object->schar == '\0');
}

bool_t object_t_eq(object_t a, object_t b)
{
    i64_t i, l;

    if (a == b)
        return true;

    if (a->type != b->type)
        return 0;

    if (a->type == -TYPE_vector_i64 || a->type == -TYPE_SYMBOL || a->type == TYPE_UNARY || a->type == TYPE_BINARY || a->type == TYPE_VARY)
        return a->i64 == b->i64;
    else if (a->type == -TYPE_vector_f64)
        return a->f64 == b->f64;
    else if (a->type == TYPE_vector_i64 || a->type == TYPE_SYMBOL)
    {
        if (as_vector_i64(a) == as_vector_i64(b))
            return true;
        if (a->len != b->len)
            return false;

        l = a->len;
        for (i = 0; i < l; i++)
        {
            if (as_vector_i64(a)[i] != as_vector_i64(b)[i])
                return false;
        }
        return 1;
    }
    else if (a->type == TYPE_vector_f64)
    {
        if (as_vector_f64(a) == as_vector_f64(b))
            return 1;
        if (a->len != b->len)
            return false;

        l = a->len;
        for (i = 0; i < l; i++)
        {
            if (as_vector_f64(a)[i] != as_vector_f64(b)[i])
                return false;
        }
        return true;
    }

    return false;
}

/*
 * Increment the reference count of an object_t
 */
object_t __attribute__((hot)) clone(object_t object)
{
    i64_t i, l;
    u16_t slaves = runtime_get()->slaves;

    if (slaves)
        __atomic_fetch_add(&((object)->rc), 1, __ATOMIC_RELAXED);
    else
        (object)->rc += 1;

    switch (object->type)
    {
    case TYPE_BOOL:
    case TYPE_vector_i64:
    case TYPE_vector_f64:
    case TYPE_SYMBOL:
    case TYPE_TIMESTAMP:
    case TYPE_CHAR:
        return object;
    case TYPE_LIST:
        l = object->len;
        for (i = 0; i < l; i++)
            clone(&as_list(object)[i]);
        return object;
    case TYPE_DICT:
    case TYPE_TABLE:
        clone(&as_list(object)[0]);
        clone(&as_list(object)[1]);
        return object;
    case TYPE_LAMBDA:
        return object;
    case TYPE_ERROR:
        return object;
    default:
        panic(str_fmt(0, "clone: invalid type: %d", object->type));
    }
}

/*
 * Free an object_t
 */
null_t __attribute__((hot)) drop(object_t object)
{
    i64_t i, rc, l;
    u16_t slaves = runtime_get()->slaves;

    if (slaves)
        rc = __atomic_sub_fetch(&((object)->rc), 1, __ATOMIC_RELAXED);
    else
    {
        (object)->rc -= 1;
        rc = (object)->rc;
    }

    switch (object->type)
    {
    case TYPE_BOOL:
    case TYPE_vector_i64:
    case TYPE_vector_f64:
    case TYPE_SYMBOL:
    case TYPE_TIMESTAMP:
    case TYPE_CHAR:
        if (rc == 0)
            rf_free(object->ptr);
        return;
    case TYPE_LIST:
        l = object->len;
        for (i = 0; i < l; i++)
            drop(as_list(object)[i]);
        if (rc == 0)
            rf_free(object->ptr);
        return;
    case TYPE_TABLE:
    case TYPE_DICT:
        drop(&as_list(object)[0]);
        drop(&as_list(object)[1]);
        if (rc == 0)
            rf_free(object->ptr);
        return;
    case TYPE_LAMBDA:
        if (rc == 0)
        {
            drop(&as_lambda(object)->constants);
            drop(&as_lambda(object)->args);
            drop(&as_lambda(object)->locals);
            drop(&as_lambda(object)->code);
            debuginfo_free(&as_lambda(object)->debuginfo);
            rf_free(object->ptr);
        }
        return;
    case TYPE_ERROR:
        if (rc == 0)
            rf_free(object->ptr);
        return;
    default:
        panic(str_fmt(0, "free: invalid type: %d", object->type));
    }
}

/*
 * Copy on write rf_
 */
object_t cow(object_t object)
{
    i64_t i, l;
    object_t new = NULL;

    // TODO: implement copy on write
    return object;

    // if (object_t_rc(object) == 1)
    //     return clone(object);

    // switch (object->type)
    // {
    // case TYPE_BOOL:
    //     new = vector_bool(object->len);
    //     new.adt->attrs = object->attrs;
    //     memcpy(as_vector_bool(&new), as_vector_bool(object), object->len);
    //     return new;
    // case TYPE_vector_i64:
    //     new = vector_i64(object->len);
    //     new.adt->attrs = object->attrs;
    //     memcpy(as_vector_i64(&new), as_vector_i64(object), object->len * sizeof(i64_t));
    //     return new;
    // case TYPE_vector_f64:
    //     new = vector_f64(object->len);
    //     new.adt->attrs = object->attrs;
    //     memcpy(as_vector_f64(&new), as_vector_f64(object), object->len * sizeof(f64_t));
    //     return new;
    // case TYPE_SYMBOL:
    //     new = vector_symbol(object->len);
    //     new.adt->attrs = object->attrs;
    //     memcpy(as_vector_symbol(&new), as_vector_symbol(object), object->len * sizeof(i64_t));
    //     return new;
    // case TYPE_TIMESTAMP:
    //     new = vector_timestamp(object->len);
    //     new.adt->attrs = object->attrs;
    //     memcpy(as_vector_timestamp(&new), as_vector_timestamp(object), object->len * sizeof(i64_t));
    //     return new;
    // case TYPE_CHAR:
    //     new = string(object->len);
    //     new.adt->attrs = object->attrs;
    //     memcpy(as_string(&new), as_string(object), object->len);
    //     return new;
    // case TYPE_LIST:
    //     l = object->len;
    //     new = list(l);
    //     new.adt->attrs = object->attrs;
    //     for (i = 0; i < l; i++)
    //         as_list(&new)[i] = cow(&as_list(object)[i]);
    //     return new;
    // case TYPE_DICT:
    //     as_list(object)[0] = cow(&as_list(object)[0]);
    //     as_list(object)[1] = cow(&as_list(object)[1]);
    //     new.adt->attrs = object->attrs;
    //     return new;
    // case TYPE_TABLE:
    //     new = table(cow(&as_list(object)[0]), cow(&as_list(object)[1]));
    //     new.adt->attrs = object->attrs;
    //     return new;
    // case TYPE_LAMBDA:
    //     return *object;
    // case TYPE_ERROR:
    //     return *object;
    // default:
    //     panic(str_fmt(0, "cow: invalid type: %d", object->type));
    // }
}

/*
 * Get the reference count of an rf_
 */
i64_t rc(object_t object)
{
    i64_t rc;
    u16_t slaves = runtime_get()->slaves;

    if (slaves)
        rc = __atomic_load_n(&((object)->rc), __ATOMIC_RELAXED);
    else
        rc = (object)->rc;

    return rc;
}
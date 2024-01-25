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

#include "lambda.h"
#include "string.h"
#include "heap.h"
#include "eval.h"
#include "util.h"
#include "group.h"

obj_t lambda(obj_t args, obj_t body, obj_t nfo)
{
    obj_t obj = heap_alloc(sizeof(struct obj_t) + sizeof(lambda_t));
    lambda_t *f = (lambda_t *)obj->arr;

    f->name = NULL;
    f->args = args;
    f->body = body;
    f->nfo = nfo;

    obj->type = TYPE_LAMBDA;
    obj->rc = 1;

    return obj;
}

obj_t lambda_map(obj_t f, u64_t l, obj_t *x, u64_t n)
{
    u64_t i, j;
    obj_t v, res;

    if (n == 0 || l == 0)
        return null(0);

    for (j = 0; j < n; j++)
        stack_push(at_idx(x[j], 0));

    v = call(f, n);

    if (is_error(v))
    {
        res = v;
        goto cleanup;
    }

    res = v->type < 0 ? vector(v->type, l) : list(l);

    ins_obj(&res, 0, v);

    for (i = 1; i < l; i++)
    {
        for (j = 0; j < n; j++)
            stack_push(at_idx(x[j], i));

        v = call(f, n);

        if (is_error(v))
        {
            res->len = i;
            drop(res);
            res = v;
            goto cleanup;
        }

        ins_obj(&res, i, v);
    }

// cleanup stack
cleanup:
    for (j = 0; j < n; j++)
        stack_pop();

    return res;
}

obj_t lambda_call(u8_t attrs, obj_t f, obj_t *x, u64_t n)
{
    u64_t i, l;
    obj_t v;

    l = ops_count(x[0]);

    // Collect groups and map lambda over each group
    if (attrs & FN_GROUP_MAP)
    {
        for (i = 0; i < n; i++)
        {
            v = x[i];
            if (v->type == TYPE_GROUPMAP)
            {
                x[i] = group_collect(v);
                drop(v);
            }
        }

        return lambda_map(f, l, x, n);
    }

    return call(f, n);
}
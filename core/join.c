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

#include "join.h"
#include "heap.h"
#include "util.h"
#include "items.h"
#include "rel.h"
#include "binary.h"
#include "compose.h"

obj_t lj_column(obj_t left_col, obj_t right_col, i64_t ids[])
{
    u64_t i, l;
    obj_t v, res;
    i64_t idx;

    // there is no such column in the right table
    if (is_null(right_col))
        return clone(left_col);

    l = right_col->len;

    if ((!is_null(left_col)) && (right_col->type != left_col->type))
        emit(ERR_TYPE, "join_column: incompatible types");

    res = vector(right_col->type, l);

    for (i = 0; i < l; i++)
    {
        idx = ids[i];
        if (idx != NULL_I64)
            v = at_idx(right_col, idx);
        else
            v = at_idx(left_col, i);

        ins_obj(&res, i, v);
    }

    return res;
}

obj_t build_idx(obj_t lcols, obj_t rcols)
{
    u64_t i, j, n, l;
    obj_t ht, res;
    i64_t key, idx;

    n = rcols->len;
    l = as_list(rcols)[0]->len;
    ht = ht_tab(l, TYPE_I64);

    for (i = 0; i < n; i++)
    {
        key = as_i64(as_list(rcols)[0])[i];
        for (j = 1; j < l; j++)
        {
            key ^= as_i64(as_list(rcols)[j])[i];
            key *= 31;
        }

        // idx = ht_
    }

    // res = clone(as_list(ids)[0]);
    // drop(ids);

    return res;
}

obj_t ray_lj(obj_t *x, u64_t n)
{
    i64_t i, j, l;
    obj_t k1, k2, c1, c2, un, col, cols, vals, ids, rescols, resvals;

    if (n != 3)
        emit(ERR_LENGTH, "lj");

    if (x[0]->type != TYPE_SYMBOL)
        emit(ERR_TYPE, "lj: first argument must be a symbol vector");

    if (x[1]->type != TYPE_TABLE)
        emit(ERR_TYPE, "lj: second argument must be a table");

    if (x[2]->type != TYPE_TABLE)
        emit(ERR_TYPE, "lj: third argument must be a table");

    k1 = ray_at(x[1], x[0]);
    if (is_error(k1))
        return k1;
    k2 = ray_at(x[2], x[0]);
    if (is_error(k2))
    {
        drop(k1);
        return k2;
    }

    ids = build_idx(k1, k2);
    drop(k2);

    un = ray_union(as_list(x[1])[0], as_list(x[2])[0]);
    if (is_error(un))
    {
        drop(k1);
        return un;
    }

    // exclude columns that we are joining on
    cols = ray_except(un, x[0]);
    drop(un);

    if (is_error(cols))
    {
        drop(k1);
        drop(ids);
        return cols;
    }

    l = cols->len;

    // resulting columns
    vals = vector(TYPE_LIST, l);

    // process each column
    for (i = 0; i < l; i++)
    {
        c1 = null(0);
        c2 = null(0);

        for (j = 0; j < (i64_t)as_list(x[1])[0]->len; j++)
        {
            if (as_symbol(as_list(x[1])[0])[j] == as_symbol(cols)[i])
            {
                c1 = as_list(as_list(x[1])[1])[j];
                break;
            }
        }

        for (j = 0; j < (i64_t)as_list(x[2])[0]->len; j++)
        {
            if (as_symbol(as_list(x[2])[0])[j] == as_symbol(cols)[i])
            {
                c2 = as_list(as_list(x[2])[1])[j];
                break;
            }
        }

        col = lj_column(c1, c2, as_i64(ids));
        if (is_error(col))
        {
            drop(k1);
            drop(cols);
            drop(vals);
            return col;
        }

        as_list(vals)[i] = col;
    }

    drop(ids);
    rescols = ray_concat(x[0], cols);
    drop(cols);
    resvals = ray_concat(k1, vals);
    drop(vals);
    drop(k1);

    return table(rescols, resvals);
}
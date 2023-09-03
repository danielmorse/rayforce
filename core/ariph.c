/*
 *   Copyright (c) 2023 Anton Kundenko <singaraiona@gmail.com>
 *   All rights reserved.

 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, modlicense, and/or sell
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

#include "util.h"
#include "ops.h"

obj_t ray_add(obj_t x, obj_t y)
{
    u64_t i, l = 0;
    obj_t vec;
    i64_t *xids = NULL, *yids = NULL, *xivals = NULL, *yivals = NULL;
    f64_t *xfvals = NULL, *yfvals = NULL;

dispatch:
    switch (mtype2(x->type, y->type))
    {
    case mtype2(-TYPE_I64, -TYPE_I64):
        return i64(addi64(x->i64, y->i64));
    case mtype2(-TYPE_I64, -TYPE_F64):
        return i64(addi64(x->i64, (i64_t)y->f64));
    case mtype2(-TYPE_F64, -TYPE_F64):
        return f64(addf64(x->f64, y->f64));
    case mtype2(-TYPE_F64, -TYPE_I64):
        return f64(addf64(x->f64, (f64_t)y->i64));
    case mtype2(-TYPE_TIMESTAMP, -TYPE_I64):
        return timestamp(addi64(x->i64, y->i64));
    case mtype2(-TYPE_I64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addi64(x->i64, yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = addi64(x->i64, yivals[i]);

        return vec;
    case mtype2(-TYPE_I64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addf64((f64_t)x->i64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = addf64((f64_t)x->i64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(x->f64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = addf64(x->f64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(x->f64, (f64_t)yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = addf64(x->f64, (f64_t)yivals[i]);

        return vec;
    case mtype2(TYPE_I64, -TYPE_I64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addi64(xivals[xids[i]], y->i64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = addi64(xivals[i], y->i64);

        return vec;
    case mtype2(TYPE_I64, -TYPE_F64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addf64((f64_t)xivals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = addf64((f64_t)xivals[i], y->f64);

        return vec;
    case mtype2(TYPE_I64, TYPE_I64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addi64(xivals[xids[i]], yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addi64(xivals[xids[i]], yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addi64(xivals[i], yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addi64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_I64, TYPE_F64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addf64((f64_t)xivals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addf64((f64_t)xivals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addf64((f64_t)xivals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = addi64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, -TYPE_F64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(xfvals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = addf64(xfvals[i], y->f64);

        return vec;
    case mtype2(TYPE_F64, -TYPE_I64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(xfvals[xids[i]], (f64_t)y->i64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = addf64(xfvals[i], (f64_t)y->i64);

        return vec;
    case mtype2(TYPE_F64, TYPE_F64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(xfvals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(xfvals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(xfvals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(xfvals[i], yfvals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, TYPE_I64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(xfvals[xids[i]], (f64_t)yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(xfvals[xids[i]], (f64_t)yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(xfvals[i], (f64_t)yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = addf64(xfvals[i], (f64_t)yivals[i]);
        }

        return vec;
    default:
        if ((x->type == TYPE_VECMAP) && (y->type == TYPE_VECMAP))
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];

            if (l != as_list(y)[1]->len)
                raise(ERR_LENGTH, "add: vectors must be of the same length");

            yids = as_i64(as_list(y)[1]);
            y = as_list(y)[0];

            goto dispatch;
        }
        else if (x->type == TYPE_VECMAP)
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];
            goto dispatch;
        }
        else if (y->type == TYPE_VECMAP)
        {
            yids = as_i64(as_list(y)[1]);
            l = as_list(y)[1]->len;
            y = as_list(y)[0];
            goto dispatch;
        }

        raise(ERR_TYPE, "add: unsupported types: %d %d", x->type, y->type);
    }
}

obj_t ray_sub(obj_t x, obj_t y)
{
    u64_t i, l = 0;
    obj_t vec;
    i64_t *xids = NULL, *yids = NULL, *xivals = NULL, *yivals = NULL;
    f64_t *xfvals = NULL, *yfvals = NULL;

dispatch:
    switch (mtype2(x->type, y->type))
    {
    case mtype2(-TYPE_I64, -TYPE_I64):
        return i64(subi64(x->i64, y->i64));
    case mtype2(-TYPE_I64, -TYPE_F64):
        return i64(subi64(x->i64, (i64_t)y->f64));
    case mtype2(-TYPE_F64, -TYPE_F64):
        return f64(subf64(x->f64, y->f64));
    case mtype2(-TYPE_F64, -TYPE_I64):
        return f64(subf64(x->f64, (f64_t)y->i64));
    case mtype2(-TYPE_TIMESTAMP, -TYPE_I64):
        return timestamp(subi64(x->i64, y->i64));
    case mtype2(-TYPE_I64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subi64(x->i64, yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = subi64(x->i64, yivals[i]);

        return vec;
    case mtype2(-TYPE_I64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subf64((f64_t)x->i64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = subf64((f64_t)x->i64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(x->f64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = subf64(x->f64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(x->f64, (f64_t)yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = subf64(x->f64, (f64_t)yivals[i]);

        return vec;
    case mtype2(TYPE_I64, -TYPE_I64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subi64(xivals[xids[i]], y->i64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = subi64(xivals[i], y->i64);

        return vec;
    case mtype2(TYPE_I64, -TYPE_F64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subf64((f64_t)xivals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = subf64((f64_t)xivals[i], y->f64);

        return vec;
    case mtype2(TYPE_I64, TYPE_I64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subi64(xivals[xids[i]], yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subi64(xivals[xids[i]], yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subi64(xivals[i], yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subi64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_I64, TYPE_F64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subf64((f64_t)xivals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subf64((f64_t)xivals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subf64((f64_t)xivals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = subi64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, -TYPE_F64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(xfvals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = subf64(xfvals[i], y->f64);

        return vec;
    case mtype2(TYPE_F64, -TYPE_I64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(xfvals[xids[i]], (f64_t)y->i64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = subf64(xfvals[i], (f64_t)y->i64);

        return vec;
    case mtype2(TYPE_F64, TYPE_F64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(xfvals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(xfvals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(xfvals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(xfvals[i], yfvals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, TYPE_I64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(xfvals[xids[i]], (f64_t)yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(xfvals[xids[i]], (f64_t)yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(xfvals[i], (f64_t)yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = subf64(xfvals[i], (f64_t)yivals[i]);
        }

        return vec;
    default:
        if ((x->type == TYPE_VECMAP) && (y->type == TYPE_VECMAP))
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];

            if (l != as_list(y)[1]->len)
                raise(ERR_LENGTH, "sub: vectors must be of the same length");

            yids = as_i64(as_list(y)[1]);
            y = as_list(y)[0];

            goto dispatch;
        }
        else if (x->type == TYPE_VECMAP)
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];
            goto dispatch;
        }
        else if (y->type == TYPE_VECMAP)
        {
            yids = as_i64(as_list(y)[1]);
            l = as_list(y)[1]->len;
            y = as_list(y)[0];
            goto dispatch;
        }

        raise(ERR_TYPE, "sub: unsupported types: %d %d", x->type, y->type);
    }
}

obj_t ray_mul(obj_t x, obj_t y)
{
    u64_t i, l = 0;
    obj_t vec;
    i64_t *xids = NULL, *yids = NULL, *xivals = NULL, *yivals = NULL;
    f64_t *xfvals = NULL, *yfvals = NULL;

dispatch:
    switch (mtype2(x->type, y->type))
    {
    case mtype2(-TYPE_I64, -TYPE_I64):
        return i64(muli64(x->i64, y->i64));
    case mtype2(-TYPE_I64, -TYPE_F64):
        return i64(muli64(x->i64, (i64_t)y->f64));
    case mtype2(-TYPE_F64, -TYPE_F64):
        return f64(subf64(x->f64, y->f64));
    case mtype2(-TYPE_F64, -TYPE_I64):
        return f64(mulf64(x->f64, (f64_t)y->i64));
    case mtype2(-TYPE_I64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = muli64(x->i64, yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = muli64(x->i64, yivals[i]);

        return vec;
    case mtype2(-TYPE_I64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = mulf64((f64_t)x->i64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = mulf64((f64_t)x->i64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(x->f64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = mulf64(x->f64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(x->f64, (f64_t)yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = mulf64(x->f64, (f64_t)yivals[i]);

        return vec;
    case mtype2(TYPE_I64, -TYPE_I64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = muli64(xivals[xids[i]], y->i64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = muli64(xivals[i], y->i64);

        return vec;
    case mtype2(TYPE_I64, -TYPE_F64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = mulf64((f64_t)xivals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = mulf64((f64_t)xivals[i], y->f64);

        return vec;
    case mtype2(TYPE_I64, TYPE_I64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = muli64(xivals[xids[i]], yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = muli64(xivals[xids[i]], yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = muli64(xivals[i], yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = muli64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_I64, TYPE_F64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = mulf64((f64_t)xivals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = mulf64((f64_t)xivals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = mulf64((f64_t)xivals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = muli64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, -TYPE_F64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(xfvals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = mulf64(xfvals[i], y->f64);

        return vec;
    case mtype2(TYPE_F64, -TYPE_I64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(xfvals[xids[i]], (f64_t)y->i64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = mulf64(xfvals[i], (f64_t)y->i64);

        return vec;
    case mtype2(TYPE_F64, TYPE_F64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(xfvals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(xfvals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(xfvals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(xfvals[i], yfvals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, TYPE_I64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(xfvals[xids[i]], (f64_t)yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(xfvals[xids[i]], (f64_t)yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(xfvals[i], (f64_t)yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = mulf64(xfvals[i], (f64_t)yivals[i]);
        }

        return vec;
    default:
        if ((x->type == TYPE_VECMAP) && (y->type == TYPE_VECMAP))
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];

            if (l != as_list(y)[1]->len)
                raise(ERR_LENGTH, "mul: vectors must be of the same length");

            yids = as_i64(as_list(y)[1]);
            y = as_list(y)[0];

            goto dispatch;
        }
        else if (x->type == TYPE_VECMAP)
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];
            goto dispatch;
        }
        else if (y->type == TYPE_VECMAP)
        {
            yids = as_i64(as_list(y)[1]);
            l = as_list(y)[1]->len;
            y = as_list(y)[0];
            goto dispatch;
        }

        raise(ERR_TYPE, "mul: unsupported types: %d %d", x->type, y->type);
    }
}

obj_t ray_div(obj_t x, obj_t y)
{
    u64_t i, l = 0;
    obj_t vec;
    i64_t *xids = NULL, *yids = NULL, *xivals = NULL, *yivals = NULL;
    f64_t *xfvals = NULL, *yfvals = NULL;

dispatch:
    switch (mtype2(x->type, y->type))
    {
    case mtype2(-TYPE_I64, -TYPE_I64):
        return i64(divi64(x->i64, y->i64));
    case mtype2(-TYPE_I64, -TYPE_F64):
        return i64(divi64(x->i64, (i64_t)y->f64));
    case mtype2(-TYPE_F64, -TYPE_F64):
        return f64(divf64(x->f64, y->f64));
    case mtype2(-TYPE_F64, -TYPE_I64):
        return f64(divf64(x->f64, (f64_t)y->i64));
    case mtype2(-TYPE_I64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divi64(x->i64, yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = divi64(x->i64, yivals[i]);

        return vec;
    case mtype2(-TYPE_I64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divf64((f64_t)x->i64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = divf64((f64_t)x->i64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(x->f64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = divf64(x->f64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(x->f64, (f64_t)yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = divf64(x->f64, (f64_t)yivals[i]);

        return vec;
    case mtype2(TYPE_I64, -TYPE_I64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divi64(xivals[xids[i]], y->i64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = divi64(xivals[i], y->i64);

        return vec;
    case mtype2(TYPE_I64, -TYPE_F64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divf64((f64_t)xivals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = divf64((f64_t)xivals[i], y->f64);

        return vec;
    case mtype2(TYPE_I64, TYPE_I64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divi64(xivals[xids[i]], yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divi64(xivals[xids[i]], yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divi64(xivals[i], yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divi64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_I64, TYPE_F64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divf64((f64_t)xivals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divf64((f64_t)xivals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divf64((f64_t)xivals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = divi64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, -TYPE_F64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(xfvals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = divf64(xfvals[i], y->f64);

        return vec;
    case mtype2(TYPE_F64, -TYPE_I64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(xfvals[xids[i]], (f64_t)y->i64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = divf64(xfvals[i], (f64_t)y->i64);

        return vec;
    case mtype2(TYPE_F64, TYPE_F64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(xfvals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(xfvals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(xfvals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(xfvals[i], yfvals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, TYPE_I64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(xfvals[xids[i]], (f64_t)yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(xfvals[xids[i]], (f64_t)yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(xfvals[i], (f64_t)yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = divf64(xfvals[i], (f64_t)yivals[i]);
        }

        return vec;
    default:
        if ((x->type == TYPE_VECMAP) && (y->type == TYPE_VECMAP))
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];

            if (l != as_list(y)[1]->len)
                raise(ERR_LENGTH, "div: vectors must be of the same length");

            yids = as_i64(as_list(y)[1]);
            y = as_list(y)[0];

            goto dispatch;
        }
        else if (x->type == TYPE_VECMAP)
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];
            goto dispatch;
        }
        else if (y->type == TYPE_VECMAP)
        {
            yids = as_i64(as_list(y)[1]);
            l = as_list(y)[1]->len;
            y = as_list(y)[0];
            goto dispatch;
        }

        raise(ERR_TYPE, "div: unsupported types: %d %d", x->type, y->type);
    }
}

obj_t ray_fdiv(obj_t x, obj_t y)
{
    u64_t i, l = 0;
    obj_t vec;
    i64_t *xids = NULL, *yids = NULL, *xivals = NULL, *yivals = NULL;
    f64_t *xfvals = NULL, *yfvals = NULL;

dispatch:
    switch (mtype2(x->type, y->type))
    {
    case mtype2(-TYPE_I64, -TYPE_I64):
        return f64(fdivi64(x->i64, (f64_t)y->i64));
    case mtype2(-TYPE_I64, -TYPE_F64):
        return f64(fdivi64(x->i64, y->f64));
    case mtype2(-TYPE_F64, -TYPE_F64):
        return f64(fdivf64(x->f64, y->f64));
    case mtype2(-TYPE_F64, -TYPE_I64):
        return f64(fdivi64(x->f64, y->i64));
    case mtype2(-TYPE_I64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(x->i64, yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = fdivi64(x->i64, yivals[i]);

        return vec;
    case mtype2(-TYPE_I64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(x->i64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = fdivi64(x->i64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivf64(x->f64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = fdivf64(x->f64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(x->f64, yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = fdivi64(x->f64, yivals[i]);

        return vec;
    case mtype2(TYPE_I64, -TYPE_I64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xivals[xids[i]], y->i64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = fdivi64(xivals[i], y->i64);

        return vec;
    case mtype2(TYPE_I64, -TYPE_F64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = fdivf64((f64_t)xivals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = fdivi64(xivals[i], y->f64);

        return vec;
    case mtype2(TYPE_I64, TYPE_I64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xivals[xids[i]], yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xivals[xids[i]], yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xivals[i], yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_I64, TYPE_F64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xivals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xivals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xivals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, -TYPE_F64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivf64(xfvals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = fdivf64(xfvals[i], y->f64);

        return vec;
    case mtype2(TYPE_F64, -TYPE_I64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xfvals[xids[i]], y->i64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = fdivi64(xfvals[i], y->i64);

        return vec;
    case mtype2(TYPE_F64, TYPE_F64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivf64(xfvals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivf64(xfvals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivf64(xfvals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivf64(xfvals[i], yfvals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, TYPE_I64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xfvals[xids[i]], yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xfvals[xids[i]], yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xfvals[i], yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = fdivi64(xfvals[i], yivals[i]);
        }

        return vec;
    default:
        if ((x->type == TYPE_VECMAP) && (y->type == TYPE_VECMAP))
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];

            if (l != as_list(y)[1]->len)
                raise(ERR_LENGTH, "fdiv: vectors must be of the same length");

            yids = as_i64(as_list(y)[1]);
            y = as_list(y)[0];

            goto dispatch;
        }
        else if (x->type == TYPE_VECMAP)
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];
            goto dispatch;
        }
        else if (y->type == TYPE_VECMAP)
        {
            yids = as_i64(as_list(y)[1]);
            l = as_list(y)[1]->len;
            y = as_list(y)[0];
            goto dispatch;
        }

        raise(ERR_TYPE, "add: unsupported types: %d %d", x->type, y->type);
    }
}

obj_t ray_mod(obj_t x, obj_t y)
{
    u64_t i, l = 0;
    obj_t vec;
    i64_t *xids = NULL, *yids = NULL, *xivals = NULL, *yivals = NULL;
    f64_t *xfvals = NULL, *yfvals = NULL;

dispatch:
    switch (mtype2(x->type, y->type))
    {
    case mtype2(-TYPE_I64, -TYPE_I64):
        return i64(modi64(x->i64, y->i64));
    case mtype2(-TYPE_I64, -TYPE_F64):
        return i64(modi64(x->i64, (i64_t)y->f64));
    case mtype2(-TYPE_F64, -TYPE_F64):
        return i64(modf64(x->f64, y->f64));
    case mtype2(-TYPE_F64, -TYPE_I64):
        return i64(modf64(x->f64, (f64_t)y->i64));
    case mtype2(-TYPE_I64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modi64(x->i64, yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = modi64(x->i64, yivals[i]);

        return vec;
    case mtype2(-TYPE_I64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modf64((f64_t)x->i64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = modf64((f64_t)x->i64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_F64):
        if (yids)
        {
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modf64(x->f64, yfvals[yids[i]]);

            return vec;
        }

        l = y->len;
        yfvals = as_f64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = modf64(x->f64, yfvals[i]);

        return vec;
    case mtype2(-TYPE_F64, TYPE_I64):
        if (yids)
        {
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modf64(x->f64, (f64_t)yivals[yids[i]]);

            return vec;
        }

        l = y->len;
        yivals = as_i64(y);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = modf64(x->f64, (f64_t)yivals[i]);

        return vec;
    case mtype2(TYPE_I64, -TYPE_I64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modi64(xivals[xids[i]], y->i64);

            return vec;
        }
        l = x->len;
        xivals = as_i64(x);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = modi64(xivals[i], y->i64);

        return vec;
    case mtype2(TYPE_I64, -TYPE_F64):
        if (xids)
        {
            xivals = as_i64(x);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modf64((f64_t)xivals[xids[i]], y->f64);

            return vec;
        }

        l = x->len;
        xivals = as_i64(x);
        vec = vector_i64(l);
        for (i = 0; i < l; i++)
            as_i64(vec)[i] = modf64((f64_t)xivals[i], y->f64);

        return vec;
    case mtype2(TYPE_I64, TYPE_I64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modi64(xivals[xids[i]], yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modi64(xivals[xids[i]], yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modi64(xivals[i], yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modi64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_I64, TYPE_F64):
        if (xids && yids)
        {
            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modi64(xivals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modf64((f64_t)xivals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xivals = as_i64(x);
            yfvals = as_f64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modf64((f64_t)xivals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xivals = as_i64(x);
            yivals = as_i64(y);
            vec = vector_i64(l);
            for (i = 0; i < l; i++)
                as_i64(vec)[i] = modi64(xivals[i], yivals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, -TYPE_F64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = modf64(y->f64, xfvals[xids[i]]);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = modf64(xfvals[i], y->f64);

        return vec;
    case mtype2(TYPE_F64, -TYPE_I64):
        if (xids)
        {
            xfvals = as_f64(x);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = modi64(xfvals[xids[i]], y->i64);

            return vec;
        }

        l = x->len;
        xfvals = as_f64(x);
        vec = vector_f64(l);
        for (i = 0; i < l; i++)
            as_f64(vec)[i] = modi64(xfvals[i], y->i64);

        return vec;
    case mtype2(TYPE_F64, TYPE_F64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = modf64(xfvals[xids[i]], yfvals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = modf64(xfvals[xids[i]], yfvals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = modf64(xfvals[i], yfvals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xfvals = as_f64(x);
            yfvals = as_f64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = modf64(xfvals[i], yfvals[i]);
        }

        return vec;
    case mtype2(TYPE_F64, TYPE_I64):
        if (xids && yids)
        {
            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = modi64(xfvals[xids[i]], yivals[yids[i]]);
        }
        else if (xids)
        {
            if (l != y->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = modi64(xfvals[xids[i]], yivals[i]);
        }
        else if (yids)
        {
            if (l != x->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = modi64(xfvals[i], yivals[yids[i]]);
        }
        else
        {
            l = x->len;
            if (l != y->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            xfvals = as_f64(x);
            yivals = as_i64(y);
            vec = vector_f64(l);
            for (i = 0; i < l; i++)
                as_f64(vec)[i] = modi64(xfvals[i], yivals[i]);
        }

        return vec;
    default:
        if ((x->type == TYPE_VECMAP) && (y->type == TYPE_VECMAP))
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];

            if (l != as_list(y)[1]->len)
                raise(ERR_LENGTH, "mod: vectors must be of the same length");

            yids = as_i64(as_list(y)[1]);
            y = as_list(y)[0];

            goto dispatch;
        }
        else if (x->type == TYPE_VECMAP)
        {
            xids = as_i64(as_list(x)[1]);
            l = as_list(x)[1]->len;
            x = as_list(x)[0];
            goto dispatch;
        }
        else if (y->type == TYPE_VECMAP)
        {
            yids = as_i64(as_list(y)[1]);
            l = as_list(y)[1]->len;
            y = as_list(y)[0];
            goto dispatch;
        }

        raise(ERR_TYPE, "add: unsupported types: %d %d", x->type, y->type);
    }
}
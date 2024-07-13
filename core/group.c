/*
 *   Copyright (c) 2024 Anton Kundenko <singaraiona@gmail.com>
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

#include "group.h"
#include "error.h"
#include "ops.h"
#include "util.h"
#include "index.h"
#include "aggr.h"
#include "items.h"
#include "unary.h"
#include "eval.h"
#include "string.h"

obj_p group_bins(obj_p obj, obj_p tab, obj_p filter)
{
    obj_p bins, v;

    switch (obj->type)
    {
    case TYPE_B8:
    case TYPE_U8:
    case TYPE_C8:
        return index_group_i8(obj, filter);
    case TYPE_I64:
    case TYPE_SYMBOL:
    case TYPE_TIMESTAMP:
        return index_group_i64(obj, filter);
    // case TYPE_F64:
    //     return index_group_i64((i64_t *)as_f64(obj), ids, l);
    case TYPE_GUID:
        return index_group_guid(obj, filter);
    case TYPE_ENUM:
        return index_group_i64(enum_val(obj), filter);
    case TYPE_LIST:
        return index_group_obj(obj, filter);
    case TYPE_ANYMAP:
        v = ray_value(obj);
        bins = index_group_obj(v, filter);
        drop_obj(v);
        return bins;
    default:
        throw(ERR_TYPE, "'group index' unable to group by: %s", type_name(obj->type));
    }
}

obj_p group_bins_list(obj_p obj, obj_p tab, obj_p filter)
{
    u64_t i, c, n, l;
    i64_t *ids;
    obj_p bins;

    // if (ops_count(obj) == 0)
    //     return error(ERR_LENGTH, "group index: empty source");

    // if (filter != NULL_OBJ)
    // {
    //     l = filter->len;
    //     ids = as_i64(filter);
    // }
    // else
    // {
    //     l = ops_count(as_list(obj)[0]);
    //     ids = NULL;
    // }

    // if (l > ops_count(tab))
    //     throw(ERR_LENGTH, "'group index': groups count: %lld can't be greater than source length: %lld", l, ops_count(tab));

    // n = obj->len;

    // c = ops_count(as_list(obj)[0]);
    // for (i = 1; i < n; i++)
    // {
    //     if (ops_count(as_list(obj)[i]) != c)
    //         throw(ERR_LENGTH, "'group index': source length: %lld must be equal to groups count: %lld", ops_count(as_list(obj)[i]), c);
    // }

    // bins = index_group_list(obj, ids, l);

    return bins;
}

/*
 * group index is a list:
 * [0] - groups number
 * [1] - bins
 * [2] - count of each group
 * --
 * result is a list
 * [0] - indexed object
 * [1] - group index (see above)
 * [2] - filters
 */
obj_p group_map(obj_p x, obj_p y, obj_p z)
{
    u64_t i, l;
    obj_p v, res;

    switch (x->type)
    {
    case TYPE_TABLE:
        l = as_list(x)[1]->len;
        res = vector(TYPE_LIST, l);
        for (i = 0; i < l; i++)
        {
            v = as_list(as_list(x)[1])[i];
            as_list(res)[i] = group_map(v, y, z);
        }

        return table(clone_obj(as_list(x)[0]), res);

    default:
        res = vn_list(3, clone_obj(x), clone_obj(y), clone_obj(z));
        res->type = TYPE_GROUPMAP;
        return res;
    }
}

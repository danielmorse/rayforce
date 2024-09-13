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
#include "eval.h"
#include "unary.h"
#include "binary.h"
#include "vary.h"
#include "heap.h"
#include "string.h"
#include "format.h"
#include "util.h"
#include "runtime.h"
#include "error.h"
#include "group.h"
#include "ops.h"
#include "io.h"

obj_p vary_call_atomic(vary_f f, obj_p *x, u64_t n) {
    u64_t i, j, l;
    obj_p v, res;

    if (n == 0)
        return NULL_OBJ;

    l = ops_rank(x, n);
    if (l == 0xfffffffffffffffful)
        THROW(ERR_LENGTH, "vary: arguments have different lengths");

    for (j = 0; j < n; j++)
        stack_push(at_idx(x[j], 0));

    v = f(x + n, n);

    if (IS_ERROR(v)) {
        res = v;
    }

    res = v->type < 0 ? vector(v->type, l) : LIST(l);

    ins_obj(&res, 0, v);

    for (i = 1; i < l; i++) {
        for (j = 0; j < n; j++)
            stack_push(at_idx(x[j], i));

        v = f(x + n, n);

        // cleanup stack
        for (j = 0; j < n; j++)
            drop_obj(stack_pop());

        if (IS_ERROR(v)) {
            res->len = i;
            drop_obj(res);
            return v;
        }

        ins_obj(&res, i, v);
    }

    return res;
}

obj_p vary_call(u8_t attrs, vary_f f, obj_p *x, u64_t n) {
    if ((attrs & FN_ATOMIC) || (attrs & FN_GROUP_MAP))
        return vary_call_atomic(f, x, n);
    else
        return f(x, n);
}

obj_p ray_do(obj_p *x, u64_t n) {
    u64_t i;
    obj_p res = NULL_OBJ;

    for (i = 0; i < n; i++) {
        drop_obj(res);
        res = eval(x[i]);
        if (IS_ERROR(res))
            return res;
    }

    return res;
}

obj_p ray_gc(obj_p *x, u64_t n) {
    UNUSED(x);
    UNUSED(n);
    return i64(heap_gc());
}

obj_p ray_format(obj_p *x, u64_t n) { return obj_fmt_n(x, n); }

obj_p ray_print(obj_p *x, u64_t n) {
    obj_p s = obj_fmt_n(x, n);

    if (s == NULL_OBJ)
        return error_str(ERR_TYPE, "malformed format string");

    printf("%.*s", (i32_t)s->len, AS_C8(s));
    drop_obj(s);

    return NULL_OBJ;
}

obj_p ray_println(obj_p *x, u64_t n) {
    obj_p s = obj_fmt_n(x, n);

    if (s == NULL_OBJ)
        return error_str(ERR_TYPE, "malformed format string");

    printf("%.*s\n", (i32_t)s->len, AS_C8(s));
    drop_obj(s);

    return NULL_OBJ;
}

obj_p ray_args(obj_p *x, u64_t n) {
    UNUSED(x);
    UNUSED(n);
    return clone_obj(runtime_get()->args);
}

obj_p ray_exit(obj_p *x, u64_t n) {
    i64_t code;

    if (n == 0)
        code = 0;
    else
        code = (x[0]->type == -TYPE_I64) ? x[0]->i64 : (i64_t)n;

    poll_exit(runtime_get()->poll, code);

    return NULL_OBJ;
}

obj_p ray_set_splayed(obj_p *x, u64_t n) {
    switch (n) {
        case 2:
            return ray_set(x[0], x[1]);
        case 3:
            return io_set_table_splayed(x[0], x[1], x[2]);
        default:
            THROW(ERR_LENGTH, "set splayed: expected 2, 3 arguments, got %d", n);
    }
}

obj_p ray_set_parted(obj_p *x, u64_t n) {
    switch (n) {
        case 2:
            return ray_set(x[0], x[1]);
        default:
            THROW(ERR_LENGTH, "set parted: expected 2, 3 arguments, got %d", n);
    }
}

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
#include "cc.h"
#include "vm.h"
#include "heap.h"
#include "format.h"
#include "util.h"
#include "string.h"
#include "env.h"
#include "runtime.h"
#include "unary.h"
#include "vary.h"
#include "binary.h"
#include "lambda.h"
#include "ops.h"
#include "compose.h"
#include "items.h"

#if defined(_WIN32) || defined(__CYGWIN__)
#define stack_malloc(size) _alloca(size)
#else
#define stack_malloc(size) alloca(size)
#endif

#define cc_push_u8(c, x)    \
    do                      \
    {                       \
        u8_t _x = x;        \
        push_raw((c), &_x); \
    } while (0)

#define cc_push_const(c, k)                        \
    do                                             \
    {                                              \
        obj_t _k = k;                              \
        lambda_t *_f = as_lambda((c)->lambda);     \
        cc_push_u8(&_f->code, _f->constants->len); \
        push_raw(&_f->constants, &_k);             \
    } while (0)

#define cc_push_opcode(c, k, v, x)               \
    do                                           \
    {                                            \
        nfo_t *d = (c)->nfo;                     \
        nfo_t *p = &as_lambda((c)->lambda)->nfo; \
        span_t u = nfo_get(d, (i64_t)k);         \
        nfo_insert(p, (u32_t)((*v))->len, u);    \
        cc_push_u8(v, x);                        \
    } while (0)

#define cc_push_un(c, x, n)                           \
    do                                                \
    {                                                 \
        u64_t _l = (*c)->len;                         \
        str_t _p = align8(as_string(*c) + _l);        \
        n _o = _p - (as_string(*c) + _l) + sizeof(n); \
        resize((c), _l + _o);                         \
        _p = align8(as_string(*c) + _l);              \
        *(n *)_p = (n)x;                              \
    } while (0)

#define cc_push_obj(c, x)                                     \
    do                                                        \
    {                                                         \
        u64_t _l = (*c)->len;                                 \
        str_t _p = align8(as_string(*c) + _l);                \
        u64_t _o = _p - (as_string(*c) + _l) + sizeof(obj_t); \
        resize((c), _l + _o);                                 \
        _p = align8(as_string(*c) + _l);                      \
        *(obj_t *)_p = x;                                     \
    } while (0)

#define cc_push_u32(c, x) cc_push_un(c, x, u32_t)

#define cc_push_u64(c, x) cc_push_un(c, x, u64_t)

#define cerr(c, i, t, ...)                        \
    {                                             \
        str_t _m = str_fmt(0, __VA_ARGS__);       \
        nfo_t *_d = (c)->nfo;                     \
        span_t _u = nfo_get(_d, (i64_t)i);        \
        drop((c)->lambda);                        \
        (c)->lambda = error(t, _m);               \
        heap_free(_m);                            \
        *(span_t *)&as_list((c)->lambda)[2] = _u; \
        return CC_ERROR;                          \
    }

cc_result_t cc_compile_quote(bool_t has_consumer, cc_t *cc, obj_t obj)
{
    obj_t car = as_list(obj)[0];
    lambda_t *func = as_lambda(cc->lambda);
    obj_t *code = &func->code;

    if (!has_consumer)
        return CC_NULL;

    cc_push_opcode(cc, car, code, OP_PUSH);
    cc_push_obj(code, as_list(obj)[1]);

    return CC_OK;
}

cc_result_t cc_compile_time(cc_t *cc, obj_t obj, u32_t arity)
{
    cc_result_t res;
    obj_t car = as_list(obj)[0];
    lambda_t *func = as_lambda(cc->lambda);
    obj_t *code = &func->code;

    if (arity != 1)
        cerr(cc, car, ERR_LENGTH, "'time' takes one argument");

    cc_push_opcode(cc, car, code, OP_TIMER_SET);
    res = cc_compile_expr(false, cc, as_list(obj)[1]);

    if (res == CC_ERROR)
        return CC_ERROR;

    cc_push_opcode(cc, car, code, OP_TIMER_GET);

    return CC_OK;
}

cc_result_t cc_compile_set(bool_t has_consumer, cc_t *cc, obj_t obj, u32_t arity)
{
    cc_result_t res;
    obj_t car = as_list(obj)[0];
    lambda_t *func = as_lambda(cc->lambda);
    obj_t *code = &func->code;

    if (arity != 2)
        cerr(cc, car, ERR_LENGTH, "'set' takes two arguments");

    cc_push_opcode(cc, car, code, OP_PUSH);
    cc_push_obj(code, as_list(obj)[1]);
    res = cc_compile_expr(true, cc, as_list(obj)[2]);

    if (res == CC_ERROR)
        return CC_ERROR;

    cc_push_opcode(cc, car, code, OP_CALL2);
    cc_push_u8(code, 0);
    cc_push_u64(code, ray_set);

    if (!has_consumer)
        cc_push_opcode(cc, car, code, OP_POP);

    return CC_OK;
}

cc_result_t cc_compile_let(bool_t has_consumer, cc_t *cc, obj_t obj, u32_t arity)
{
    cc_result_t res;
    obj_t car = as_list(obj)[0];
    lambda_t *func = as_lambda(cc->lambda);
    obj_t *code = &func->code;

    if (arity != 2)
        cerr(cc, car, ERR_LENGTH, "'let' takes two arguments");

    if (as_list(obj)[1]->type != -TYPE_SYMBOL)
        cerr(cc, car, ERR_TYPE, "'let' first argument must be a symbol");

    cc_push_opcode(cc, car, code, OP_PUSH);
    cc_push_obj(code, as_list(obj)[1]);

    res = cc_compile_expr(true, cc, as_list(obj)[2]);

    if (res == CC_ERROR)
        return CC_ERROR;

    cc_push_opcode(cc, car, code, OP_LSET);

    if (!has_consumer)
        cc_push_opcode(cc, car, code, OP_POP);

    return CC_OK;
}

cc_result_t cc_compile_fn(cc_t *cc, obj_t obj, u32_t arity)
{
    obj_t fun, body;
    lambda_t *func = as_lambda(cc->lambda);
    obj_t *code = &func->code;

    if (arity < 2)
        cerr(cc, obj, ERR_LENGTH, "'fn' expects vector of symbols with lambda arguments and a list body");

    arity -= 1;
    body = ray_list(as_list(obj) + 2, arity);
    body->attrs = ATTR_MULTIEXPR;
    fun = cc_compile_lambda("anonymous", clone(*(as_list(obj) + 1)), body, cc->nfo);
    if (fun->type == TYPE_ERROR)
    {
        drop(cc->lambda);
        cc->lambda = fun;
        return CC_ERROR;
    }

    cc_push_opcode(cc, obj, code, OP_PUSH_CONST);
    cc_push_const(cc, fun);
    func->stack_size++;

    return CC_OK;
}

cc_result_t cc_compile_cond(bool_t has_consumer, cc_t *cc, obj_t obj, u32_t arity)
{
    cc_result_t res;
    i64_t lbl1, lbl2;
    obj_t car = as_list(obj)[0];
    lambda_t *func = as_lambda(cc->lambda);
    obj_t *code = &func->code;

    if (arity < 2 || arity > 3)
        cerr(cc, car, ERR_LENGTH, "'if' expects 2 .. 3 arguments");

    res = cc_compile_expr(true, cc, as_list(obj)[1]);

    if (res == CC_ERROR)
        return CC_ERROR;

    cc_push_opcode(cc, car, code, OP_JNE);
    cc_push_u64(code, 0);
    lbl1 = (*code)->len - sizeof(u64_t);

    // true branch
    res = cc_compile_expr(has_consumer, cc, as_list(obj)[2]);

    if (res == CC_ERROR)
        return CC_ERROR;

    // there is else branch
    if (arity == 3)
    {
        cc_push_opcode(cc, car, code, OP_JMP);
        cc_push_u64(code, 0);
        lbl2 = (*code)->len - sizeof(u64_t);
        *(u64_t *)(as_string(*code) + lbl1) = (*code)->len;

        // false branch
        res = cc_compile_expr(has_consumer, cc, as_list(obj)[3]);

        if (res == CC_ERROR)
            return CC_ERROR;

        *(u64_t *)(as_string(*code) + lbl2) = (*code)->len;
    }
    else
        *(u64_t *)(as_string(*code) + lbl1) = (*code)->len;

    return CC_OK;
}

cc_result_t cc_compile_try(bool_t has_consumer, cc_t *cc, obj_t obj, u32_t arity)
{
    cc_result_t res;
    i64_t lbl1, lbl2;
    obj_t car = as_list(obj)[0];
    lambda_t *func = as_lambda(&cc->lambda);
    obj_t *code = &func->code;

    if (arity != 2)
        cerr(cc, car, ERR_LENGTH, "'try': expects 2 arguments");

    cc_push_opcode(cc, car, code, OP_TRY);
    cc_push_u64(code, 0);
    lbl1 = (*code)->len - sizeof(u64_t);

    // compile expression under trap
    res = cc_compile_expr(true, cc, as_list(obj)[1]);

    if (res == CC_ERROR)
        return CC_ERROR;

    cc_push_opcode(cc, car, code, OP_JMP);
    cc_push_u64(code, 0);
    lbl2 = (*code)->len - sizeof(u64_t);

    *(u64_t *)(as_string(*code) + lbl1) = (*code)->len;

    // compile expression under catch
    res = cc_compile_expr(has_consumer, cc, as_list(obj)[2]);

    if (res == CC_ERROR)
        return CC_ERROR;

    *(u64_t *)(as_string(*code) + lbl2) = (*code)->len;

    return CC_OK;
}

cc_result_t cc_compile_throw(cc_t *cc, obj_t obj, u32_t arity)
{
    cc_result_t res;
    obj_t car = as_list(obj)[0];
    lambda_t *func = as_lambda(&cc->lambda);
    obj_t *code = &func->code;

    if (arity != 1)
        cerr(cc, car, ERR_LENGTH, "'throw': expects 1 argument");

    res = cc_compile_expr(true, cc, as_list(obj)[1]);

    if (res == CC_ERROR)
        return CC_ERROR;

    cc_push_opcode(cc, car, code, OP_THROW);

    return CC_OK;
}

type_t cc_compile_catch(cc_t *cc, obj_t obj, u32_t arity)
{
    obj_t car = as_list(obj)[0];
    lambda_t *func = as_lambda(&cc->lambda);
    obj_t *code = &func->code;

    if (arity != 0)
        cerr(cc, car, ERR_LENGTH, "'catch': expects 0 arguments");

    cc_push_opcode(cc, car, code, OP_CATCH);

    return CC_OK;
}

cc_result_t cc_compile_call(cc_t *cc, obj_t car, u8_t arity)
{
    cc_result_t res;
    lambda_t *func = as_lambda(cc->lambda);
    obj_t *code = &func->code, prim;
    i64_t i;

    // self is a special case
    if (car->type == -TYPE_SYMBOL && car->i64 == KW_SELF)
    {
        cc_push_opcode(cc, car, code, OP_PUSH);
        cc_push_obj(code, cc->lambda);

        cc_push_opcode(cc, car, code, OP_CALLD);
        cc_push_u8(code, arity);

        func->stack_size++;

        return CC_OK;
    }

    i = find_obj(as_list(runtime_get()->env.functions)[0], car);

    if (i == (i64_t)as_list(runtime_get()->env.functions)[0]->len)
    {
        res = cc_compile_expr(true, cc, car);

        if (res == CC_ERROR)
            return CC_ERROR;

        cc_push_opcode(cc, car, code, OP_CALLD);
        cc_push_u8(code, arity);

        func->stack_size++;

        return CC_OK;
    }

    prim = as_list(as_list(runtime_get()->env.functions)[1])[i];

    switch (prim->type)
    {
    case TYPE_UNARY:
        if (arity != 1)
            cerr(cc, car, ERR_LENGTH, "unary function expects 1 argument");

        cc_push_opcode(cc, car, code, OP_CALL1);
        cc_push_u8(code, prim->attrs);
        cc_push_u64(code, prim->i64);

        return CC_OK;
    case TYPE_BINARY:
        if (arity != 2)
            cerr(cc, car, ERR_LENGTH, "binary function expects 2 arguments");

        cc_push_opcode(cc, car, code, OP_CALL2);
        cc_push_u8(code, prim->attrs);
        cc_push_u64(code, prim->i64);

        return CC_OK;
    case TYPE_VARY:
        cc_push_opcode(cc, car, code, OP_CALLN);
        cc_push_u8(code, arity);
        cc_push_u8(code, prim->attrs);
        cc_push_u64(code, prim->i64);

        return CC_OK;
    default:
        cerr(cc, car, ERR_NOT_FOUND, "cc compile call: function not found");
    }
}

nil_t find_used_symbols(obj_t lst, obj_t *syms)
{
    i64_t i, l;

    switch (lst->type)
    {
    case -TYPE_SYMBOL:
        if (lst->i64 > 0)
            push_raw(syms, &lst->i64);
        return;
    case TYPE_LIST:
        l = lst->len;
        if (l == 0)
            return;
        for (i = 0; i < l; i++)
            find_used_symbols(as_list(lst)[i], syms);
        return;
    default:
        return;
    }
}

obj_t at_sym(obj_t x, obj_t y)
{
    u64_t i, j, xl, yl;
    obj_t cols;

    switch (mtype2(x->type, y->type))
    {
    case mtype2(TYPE_TABLE, TYPE_SYMBOL):
        xl = as_list(x)[1]->len;
        yl = y->len;
        cols = vector(TYPE_LIST, yl);
        for (i = 0; i < yl; i++)
        {
            for (j = 0; j < xl; j++)
            {
                if (as_symbol(as_list(x)[0])[j] == as_symbol(y)[i])
                {
                    as_list(cols)[i] = clone(as_list(as_list(x)[1])[j]);
                    break;
                }
            }

            if (j == xl)
            {
                cols->len = i;
                drop(cols);
                emit(ERR_INDEX, "at: column '%s' has not found in a table", symtostr(as_symbol(y)[i]));
            }
        }

        return table(clone(y), cols);

    default:
        emit(ERR_TYPE, "at: unsupported types");
    }
}

cc_result_t cc_compile_select(bool_t has_consumer, cc_t *cc, obj_t obj, u32_t arity)
{
    cc_result_t res;
    i64_t i, l;
    obj_t car, params, key, val, cols, k, v;
    lambda_t *func = as_lambda(cc->lambda);
    obj_t *code = &func->code, take = null(0);
    bool_t groupby = false, map = false;

    car = as_list(obj)[0];

    if (arity == 0)
        cerr(cc, car, ERR_LENGTH, "'select' takes at least two arguments");

    params = as_list(obj)[1];

    if (params->type != TYPE_DICT)
        cerr(cc, car, ERR_LENGTH, "'select' takes dict of params");

    l = as_list(params)[0]->len;

    // compile table
    key = symboli64(KW_FROM);
    val = at_obj(params, key);

    if (is_null(val))
        cerr(cc, car, ERR_LENGTH, "'select' expects 'from' param");

    res = cc_compile_expr(true, cc, val);
    dropn(2, key, val);

    if (res == CC_ERROR)
        return CC_ERROR;

    // determine which of columns are used in select and which names will be used for result columns
    cols = vector_symbol(0);

    // first check by because it is special case in mapping
    key = symboli64(KW_BY);
    val = at_obj(params, key);

    if (!is_null(val))
    {
        groupby = true;
        if (val->type == -TYPE_SYMBOL)
            push_obj(&cols, clone(val));
        else
            push_sym(&cols, "x");
    }

    dropn(2, key, val);

    for (i = 0; i < l; i++)
    {
        k = at_idx(as_list(params)[0], i);

        if (k->i64 != KW_FROM && k->i64 != KW_WHERE)
        {
            v = at_obj(params, k);

            if (k->i64 == KW_TAKE)
            {
                drop(k);
                take = v;
                continue;
            }

            if (k->i64 == KW_BY)
            {
                dropn(2, k, v);
                continue;
            }

            push_obj(&cols, k);
            drop(v);
            map = true;
        }
        else
            drop(k);
    }

    // compile filters
    key = symboli64(KW_WHERE);
    val = at_obj(params, key);
    drop(key);

    if (!is_null(val))
    {
        cc_push_opcode(cc, car, code, OP_LPUSH);

        res = cc_compile_expr(true, cc, val);

        if (res == CC_ERROR)
        {
            dropn(2, cols, val);
            return CC_ERROR;
        }

        cc_push_opcode(cc, obj, code, OP_CALL1);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_where);
        cc_push_opcode(cc, val, code, OP_LPOP);

        // create vecmaps over the table
        cc_push_opcode(cc, val, code, OP_SWAP);
        cc_push_opcode(cc, val, code, OP_CALL2);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_vecmap);
        drop(val);
    }

    // compile take (if any)
    if (!is_null(take))
    {
        cc_push_opcode(cc, car, code, OP_PUSH_CONST);
        cc_push_const(cc, take);
        cc_push_opcode(cc, car, code, OP_SWAP);
        cc_push_opcode(cc, car, code, OP_CALL2);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_take);
    }

    if (map || groupby)
        cc_push_opcode(cc, car, code, OP_LPUSH);

    // Group?
    key = symboli64(KW_BY);
    val = at_obj(params, key);
    drop(key);

    if (!is_null(val))
    {
        res = cc_compile_expr(true, cc, val);
        drop(val);

        if (res == CC_ERROR)
        {
            drop(cols);
            return CC_ERROR;
        }

        cc_push_opcode(cc, obj, code, OP_CALL1);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_group);

        cc_push_opcode(cc, obj, code, OP_DUP);
        cc_push_opcode(cc, obj, code, OP_CALL1);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_value);
        cc_push_opcode(cc, car, code, OP_LPOP);

        // remove column used for grouping from result
        cc_push_opcode(cc, obj, code, OP_DUP);
        cc_push_opcode(cc, car, code, OP_CALL1);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_key);
        cc_push_opcode(cc, car, code, OP_PUSH_CONST);
        cc_push_const(cc, at_idx(cols, 0));
        cc_push_opcode(cc, car, code, OP_CALL2);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_except);
        cc_push_opcode(cc, car, code, OP_CALL2);
        cc_push_u8(code, 0);
        cc_push_u64(code, at_sym);

        cc_push_opcode(cc, car, code, OP_SWAP);

        // apply grouping
        cc_push_opcode(cc, car, code, OP_CALL2);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_listmap);

        cc_push_opcode(cc, car, code, OP_LPUSH);

        cc_push_opcode(cc, obj, code, OP_CALL1);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_key);

        if (!map)
            cc_push_opcode(cc, car, code, OP_LPOP);
    }

    // compile mappings (if specified)
    if (map)
    {
        cc_push_opcode(cc, car, code, OP_PUSH_CONST);
        cc_push_const(cc, cols);

        if (groupby)
            cc_push_opcode(cc, car, code, OP_SWAP);

        for (i = 0; i < l; i++)
        {
            k = at_idx(as_list(params)[0], i);
            if (k->i64 != KW_FROM && k->i64 != KW_WHERE && k->i64 != KW_BY && k->i64 != KW_TAKE)
            {
                v = at_idx(as_list(params)[1], i);
                res = cc_compile_expr(true, cc, v);
                dropn(2, k, v);

                if (res == CC_ERROR)
                    return CC_ERROR;
            }
            else
                drop(k);
        }

        cc_push_opcode(cc, car, code, OP_CALLN);
        cc_push_u8(code, cols->len);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_list);

        cc_push_opcode(cc, car, code, OP_CALL2);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_table);
    }
    else
    {
        drop(cols);
        cc_push_opcode(cc, obj, code, OP_CALL1);
        cc_push_u8(code, 0);
        cc_push_u64(code, ray_value);
    }

    if (!has_consumer)
        cc_push_opcode(cc, car, code, OP_POP);

    return CC_OK;
}

cc_result_t cc_compile_return(cc_t *cc, obj_t obj, u32_t arity)
{
    cc_result_t res;
    obj_t car = as_list(obj)[0];
    lambda_t *func = as_lambda(cc->lambda);
    obj_t *code = &func->code;

    switch (arity)
    {
    case 0:
        cc_push_opcode(cc, car, code, OP_PUSH);
        cc_push_obj(code, null(0));
        break;
    case 1:
        res = cc_compile_expr(true, cc, as_list(obj)[1]);
        if (res == CC_ERROR)
            return res;
        break;
    default:
        cerr(cc, obj, ERR_LENGTH, "'return' takes at most one argument");
    }

    cc_push_opcode(cc, car, code, OP_RET);

    return CC_OK;
}

cc_result_t cc_compile_special_forms(bool_t has_consumer, cc_t *cc, obj_t obj, u32_t arity)
{
    switch (as_list(obj)[0]->i64)
    {
    case KW_QUOTE:
        return cc_compile_quote(has_consumer, cc, obj);
    case KW_TIME:
        return cc_compile_time(cc, obj, arity);
    case KW_SET:
        return cc_compile_set(has_consumer, cc, obj, arity);
    case KW_LET:
        return cc_compile_let(has_consumer, cc, obj, arity);
    case KW_FN:
        return cc_compile_fn(cc, obj, arity);
    case KW_IF:
        return cc_compile_cond(has_consumer, cc, obj, arity);
    case KW_TRY:
        return cc_compile_try(has_consumer, cc, obj, arity);
    case KW_THROW:
        return cc_compile_throw(cc, obj, arity);
    case KW_SELECT:
        return cc_compile_select(has_consumer, cc, obj, arity);
    // case KW_EVAL:
    //     return cc_compile_eval(has_consumer, cc, obj, arity);
    // case KW_LOAD:
    //     return cc_compile_load(has_consumer, cc, obj, arity);
    case KW_RETURN:
        return cc_compile_return(cc, obj, arity);
    default:
        return CC_NULL;
    }
}

cc_result_t cc_compile_expr(bool_t has_consumer, cc_t *cc, obj_t obj)
{
    u32_t i, arity;
    i64_t id;
    lambda_t *func = as_lambda(cc->lambda);
    obj_t *code = &func->code;
    cc_result_t res = CC_NULL;

    if (!obj)
    {
        if (!has_consumer)
            return CC_NULL;

        // then in a local or global env
        cc_push_opcode(cc, obj, code, OP_PUSH);
        cc_push_obj(code, obj);
        func->stack_size++;

        return CC_OK;
    }

    switch (obj->type)
    {
    case -TYPE_SYMBOL:
        if (!has_consumer)
            return CC_NULL;

        // Symbol is quoted
        if (obj->attrs & ATTR_QUOTED)
        {
            cc_push_opcode(cc, obj, code, OP_PUSH_CONST);
            cc_push_const(cc, symboli64(obj->i64));
            func->stack_size++;

            return CC_OK;
        }

        // internal function
        if (obj->i64 < 0)
        {
            cc_push_opcode(cc, obj, code, OP_PUSH);

            id = find_obj(as_list(runtime_get()->env.functions)[0], obj);

            if (id == (i64_t)as_list(runtime_get()->env.functions)[0]->len)
                cerr(cc, obj, ERR_LENGTH, "function not found");

            cc_push_obj(code, as_list(as_list(runtime_get()->env.functions)[1])[id]);
            func->stack_size++;

            return CC_OK;
        }

        // try to search in the lambda args
        id = find_obj(func->args, obj);

        if (id < (i64_t)func->args->len)
        {
            cc_push_opcode(cc, obj, code, OP_LOAD);
            cc_push_u64(code, func->args->len - id);
            func->stack_size++;

            return CC_OK;
        }

        // then in a local or global env
        cc_push_opcode(cc, obj, code, OP_PUSH);
        cc_push_obj(code, obj);
        cc_push_opcode(cc, obj, code, OP_LGET);
        func->stack_size++;

        return CC_OK;

    case TYPE_LIST:
        if (obj->len == 0)
            goto other;

        if ((as_list(obj)[0]->type != -TYPE_SYMBOL) &&
            (as_list(obj)[0]->type != TYPE_UNARY) &&
            (as_list(obj)[0]->type != TYPE_BINARY) &&
            (as_list(obj)[0]->type != TYPE_VARY) &&
            (as_list(obj)[0]->type != TYPE_LAMBDA) &&
            (as_list(obj)[0]->type != TYPE_LIST))
            cerr(cc, obj, ERR_LENGTH, "car is not callable");

        arity = obj->len - 1;

        // special forms compilation need to be done before arguments compilation
        res = cc_compile_special_forms(has_consumer, cc, obj, arity);

        if (res == CC_ERROR || res != CC_NULL)
            return res;

        // compile arguments
        for (i = 0; i < arity; i++)
        {
            res = cc_compile_expr(true, cc, as_list(obj)[i + 1]);
            if (res == CC_ERROR)
                return CC_ERROR;
        }

        res = cc_compile_call(cc, as_list(obj)[0], arity);

        if (res == CC_ERROR)
            return CC_ERROR;

        if (!has_consumer)
            cc_push_opcode(cc, obj, code, OP_POP);

        return res;

    default:
    other:
        if (!has_consumer)
            return CC_NULL;

        cc_push_opcode(cc, obj, code, OP_PUSH);
        cc_push_obj(code, obj);
        func->stack_size++;

        return CC_OK;
    }

    return CC_OK;
}

obj_t cc_compile_lambda(str_t name, obj_t args, obj_t body, nfo_t *nfo)
{
    span_t span;
    str_t msg;
    obj_t err, *code;
    nfo_t *pi, di;
    cc_result_t res;
    u64_t i = 0, len;
    lambda_t *func;

    if (nfo == NULL)
    {
        di = nfo_new("top-level", name);
        pi = &di;
    }
    else
    {
        di = nfo_new(nfo->filename, name);
        pi = nfo;
    }

    if (body->type != TYPE_LIST)
    {
        span = nfo_get(pi, (i64_t)body);
        msg = str_fmt(0, "compile '%s': expected list", "top-level");
        err = error(ERR_TYPE, msg);
        heap_free(msg);
        *(span_t *)as_list(err)[2] = span;

        return err;
    }

    len = body->len;

    cc_t cc = {
        .nfo = pi,
        .lambda = lambda(args, body, string(0), di),
    };

    func = as_lambda(cc.lambda);
    code = &func->code;

    if (len == 0)
    {
        cc_push_opcode(&cc, body, code, OP_PUSH);
        cc_push_obj(code, null(0));
        goto epilogue;
    }

    // multiexpr body
    if (body->attrs & ATTR_MULTIEXPR)
    {
        // Compile all arguments but the last one
        for (i = 0; i < len - 1; i++)
        {
            // skip const expressions
            if (as_list(body)[i]->type != TYPE_LIST)
                continue;

            res = cc_compile_expr(false, &cc, as_list(body)[i]);

            if (res == CC_ERROR)
                return cc.lambda;
        }

        // Compile last expression
        res = cc_compile_expr(true, &cc, as_list(body)[i]);
    }
    // Compile body as a single expression
    else
    {
        res = cc_compile_expr(true, &cc, body);
    }

    if (res == CC_ERROR)
        return cc.lambda;
    // --

epilogue:
    cc_push_opcode(&cc, body, code, OP_RET);

    return cc.lambda;
}

/*
 * Compile top level expression as an anonymous lambda
 */
obj_t cc_compile(obj_t body, nfo_t *nfo)
{
    return cc_compile_lambda("top-level", vector_symbol(0), body, nfo);
}

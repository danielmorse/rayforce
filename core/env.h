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

#ifndef ENV_H
#define ENV_H

#include "rayforce.h"
#include "alloc.h"
#include "vm.h"
#include "vector.h"

#define MAX_ARITY 4

/*
 *  Environment record entry
 */
typedef struct env_record_t
{
    i64_t name;
    i8_t ret;
    vm_opcode_t opcode;
    i8_t args[8];
} env_record_t;

/*
 *  Environment variables
 */
typedef struct env_t
{
    rf_object_t records; // list, containing records
} env_t;

env_t create_env();

#define get_records_len(records, i) (as_list(records)[i].adt->len)
#define get_record(records, i, j) (((env_record_t *)as_string((as_list(records) + i))) + j)

#endif

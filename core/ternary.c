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

#include <time.h>
#include "ternary.h"

rf_object_t rf_rand_i64_i64_i64(rf_object_t *x, rf_object_t *y, rf_object_t *z)
{
#define A 6364136223846793005
#define C 1442695040888963407
#define M 9223372036854775808UL // 2^63

    i64_t i, count = x->i64;
    u64_t seed, num;
    i64_t lower = y->i64, upper = z->i64, *v;
    rf_object_t vec = vector_i64(count);

    v = as_vector_i64(&vec);

    seed = time(0);

    for (i = 0; i < count; i++)
    {
        seed = (A * seed + C) % M;
        num = (seed % (upper - lower + 1)) + lower;
        v[i] = (i64_t)num;
    }

    return vec;
}
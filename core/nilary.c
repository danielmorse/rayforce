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

#include "nilary.h"
#include "runtime.h"
#include "alloc.h"

rf_object_t rf_env()
{
    return rf_object_clone(&runtime_get()->env.variables);
}

rf_object_t rf_memstat()
{
    rf_object_t keys, vals;
    memstat_t stat = rf_alloc_memstat();

    keys = vector_symbol(3);
    as_vector_symbol(&keys)[0] = symbol("total").i64;
    as_vector_symbol(&keys)[1] = symbol("used ").i64;
    as_vector_symbol(&keys)[2] = symbol("free ").i64;

    vals = list(3);
    as_list(&vals)[0] = i64(stat.total);
    as_list(&vals)[1] = i64(stat.used);
    as_list(&vals)[2] = i64(stat.free);

    return dict(keys, vals);
}

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

#ifndef ITEMS_H
#define ITEMS_H

#include "rayforce.h"

// TODO: refactor this function
obj_t ray_find_vector_i64_vector_i64(obj_t x, obj_t y, bool_t allow_null);

obj_t ray_at(obj_t x, obj_t y);
obj_t ray_find(obj_t x, obj_t y);
obj_t ray_filter(obj_t x, obj_t y);
obj_t ray_take(obj_t x, obj_t y);
obj_t ray_in(obj_t x, obj_t y);
obj_t ray_sect(obj_t x, obj_t y);
obj_t ray_except(obj_t x, obj_t y);
obj_t ray_union(obj_t x, obj_t y);
obj_t ray_first(obj_t x);
obj_t ray_last(obj_t x);
obj_t ray_key(obj_t x);
obj_t ray_value(obj_t x);
obj_t ray_where(obj_t x);

#endif // ITEMS_H
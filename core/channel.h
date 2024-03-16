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

#include "rayforce.h"

#define WRITE 1
#define READ 2
#define DESTROY 4
#define ROUND 64
#define BLOCK_SIZE (ROUND - 1)
#define CLOSED_FLAG (1ULL << 63)
#define CROSSED_FLAG CLOSED_FLAG

typedef struct slot_t
{
    u64_t state;
    raw_p message;
} *slot_p;

typedef struct block_t
{
    struct block_t *next;
    struct slot_t slots[BLOCK_SIZE];
} *block_p;

typedef struct cursor_t
{
    u64_t index;
    block_p block;
} cursor_t;

typedef struct channel_t
{
    cursor_t tail;
    cursor_t head;
} *channel_p;

block_p block_new();
nil_t block_free(block_p block, u64_t start);
block_p block_get_next(block_p block);
block_p block_wait_next(block_p block);
nil_t block_set_next(block_p block, block_p next);

channel_p channel_new();
nil_t channel_free(channel_p channel);
i32_t channel_send(channel_p channel, raw_p message);
raw_p channel_recv(channel_p channel);
i32_t channel_is_empty(channel_p channel);
nil_t channel_close(channel_p channel);
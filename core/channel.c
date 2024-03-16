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

#include "channel.h"
#include "heap.h"
#include "ops.h"
#ifdef __cplusplus
#include <atomic>
#else
#include <stdatomic.h>
#endif

#define SPIN_LIMIT 8

nil_t backoff_spin(u32_t *rounds)
{
    u64_t i, n;

    n = 1 << mini64(*rounds, SPIN_LIMIT);
    for (i = 0; i < n; i++)
        __builtin_ia32_pause();

    if (*rounds <= SPIN_LIMIT)
        *rounds += 1;
}

block_p block_new()
{
    block_p block = (block_p)heap_alloc(sizeof(struct block_t));
    memset(block, 0, sizeof(struct block_t));

    return block;
}

block_p block_get_next(block_p block)
{
    return __atomic_load_n(&block->next, __ATOMIC_ACQUIRE);
}

nil_t block_set_next(block_p block, block_p next)
{
    __atomic_exchange_n(&block->next, next, __ATOMIC_RELEASE);
}

block_p block_wait_next(block_p block)
{
    u32_t rounds = 0;
    block_p next;

    while (!(next = block_get_next(block)))
        backoff_spin(&rounds);

    return next;
}

nil_t block_free(block_p block, u64_t start)
{
    u64_t i, state;
    struct slot_t slot;

    for (i = start; i < BLOCK_SIZE - 1; i++)
    {
        slot = block->slots[i];
        if (__atomic_load_n(&slot.state, __ATOMIC_ACQUIRE) & READ == 0 &&
            __atomic_fetch_or(&slot.state, DESTROY, __ATOMIC_ACQ_REL) & READ == 0)
            return;
    }

    heap_free(block);
}

channel_p channel_new()
{
}

nil_t channel_free(channel_p channel)
{
}

i32_t channel_send(channel_p channel, raw_p message)
{
}

raw_p channel_recv(channel_p channel)
{
}

i32_t channel_is_empty(channel_p channel)
{
}

nil_t channel_close(channel_p channel)
{
}

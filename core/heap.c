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
#include <assert.h>
#include "heap.h"
#include "rayforce.h"
#include "ops.h"
#include "mmap.h"
#include "util.h"
#include "string.h"

__thread heap_p __HEAP = NULL;

#define blocksize(s) (((s) < BLOCK_HEADER_SIZE) ? MIN_ORDER : (s) + BLOCK_HEADER_SIZE)
#define bsizeof(i) (1ull << (i))
#define orderof(s) (64ull - __builtin_clzll(s - 1))
#define buddyof(p, b, i) ((raw_p)((((u64_t)(p) - (u64_t)(b)) ^ bsizeof(i)) + (u64_t)(b)))
#define addrof(b) ((raw_p)(((u64_t)(b)) + BLOCK_HEADER_SIZE))
#define blockof(p) ((block_p)(((u64_t)(p)) - BLOCK_HEADER_SIZE))

CASSERT(sizeof(struct block_t) == bsizeof(MIN_ORDER), heap_h)
// CASSERT(sizeof(struct heap_t) % PAGE_SIZE == 0, heap_h)

#ifdef SYS_MALLOC

raw_p heap_alloc(u64_t size) { return malloc(size); }
nil_t heap_free(raw_p ptr) { free(ptr); }
raw_p heap_realloc(raw_p ptr, u64_t new_size) { return realloc(ptr, new_size); }
i64_t heap_gc(nil_t) { return 0; }
nil_t heap_cleanup(nil_t) {}
nil_t heap_borrow(heap_p) {}
nil_t heap_merge(heap_p) {}
heap_p heap_init(u64_t) { return NULL; }
memstat_t heap_memstat(nil_t) { return (memstat_t){0}; }

#else

block_p heap_add_pool(u64_t order)
{
    u64_t size = bsizeof(order);
    block_p block = (block_p)mmap_malloc(size);

    if (block == NULL)
        return NULL;

    block->pool = block;
    block->pool_order = order;

    // debug("%s++ HEAP[%lld]: add pool order: %lld block: %p%s\n", GREEN, __HEAP->id, order, block, RESET);

    __HEAP->memstat.system += size;
    __HEAP->memstat.heap += size;

    return block;
}

heap_p heap_init(u64_t id)
{
    __HEAP = (heap_p)mmap_malloc(sizeof(struct heap_t));
    __HEAP->id = id;
    __HEAP->avail = 0;
    memset(__HEAP->freelist, 0, sizeof(__HEAP->freelist));

    return __HEAP;
}

inline __attribute__((always_inline)) nil_t insert_block(block_p block, u64_t order)
{
    u64_t size = bsizeof(order);

    // debug("%sInserting block: %p, order: %lld%s\n", BLUE, block, order, RESET);

    block->used = 0;
    block->order = order;
    block->next = __HEAP->freelist[order];
    block->prev = NULL;

    if (__HEAP->freelist[order] != NULL)
        __HEAP->freelist[order]->prev = block;
    else
        __HEAP->avail |= size;

    __HEAP->freelist[order] = block;
}

inline __attribute__((always_inline)) nil_t remove_block(block_p block, u64_t order)
{
    // debug("%s Removing block: %p, order: %lld%s\n", YELLOW, block, order, RESET);

    if (block->prev)
        block->prev->next = block->next;
    if (block->next)
        block->next->prev = block->prev;

    if (__HEAP->freelist[order] == block)
        __HEAP->freelist[order] = block->next;

    if (__HEAP->freelist[order] == NULL)
        __HEAP->avail &= ~bsizeof(order);

    // Update block's meta
    block->used = 1;
    block->order = order;
    block->prev = NULL;
    block->next = NULL;
}

inline __attribute__((always_inline)) nil_t split_block(block_p block, u64_t block_order, u64_t order, block_p pool, u64_t pool_order)
{
    block_p buddy;
    u64_t i = block_order;

    // split until i == order
    while ((i--) > order)
    {
        buddy = buddyof(block, pool, i);
        insert_block(buddy, i);
        buddy->pool = pool;
        buddy->pool_order = pool_order;
    }
}

raw_p __attribute__((hot)) heap_alloc(u64_t size)
{
    u64_t i, order, block_size;
    block_p block;

    if (size == 0)
        return NULL;

    block_size = blocksize(size);

    // calculate minimal order for this size
    order = orderof(block_size);

    if (order > MAX_POOL_ORDER)
        return NULL;

    // find least order block that fits
    i = (AVAIL_MASK << order) & __HEAP->avail;

    // no free block found for this size, so mmap it directly if it is bigger than pool size or
    // add a new pool and split as well
    if (i == 0)
    {
        if (order >= MAX_ORDER)
        {
            block = heap_add_pool(order);
            if (block == NULL)
                return NULL;

            block->used = 1;
            block->order = order;
            return addrof(block);
        }

        i = MAX_ORDER;

        block = heap_add_pool(i);

        if (block == NULL)
            return NULL;

        insert_block(block, i);
    }
    else
        i = __builtin_ctzll(i);

    // remove the block out of list
    block = __HEAP->freelist[i];
    block->order = order;
    block->used = 1;

    // debug("%sAllocating block: %p, order: %lld%s\n", GREEN, block, order, RESET);

    __HEAP->freelist[i] = block->next;
    if (__HEAP->freelist[i] != NULL)
        __HEAP->freelist[i]->prev = NULL;
    else
        __HEAP->avail &= ~bsizeof(i);

    split_block(block, i, order, block->pool, block->pool_order);

    return addrof(block);
}

__attribute__((hot)) nil_t heap_free(raw_p ptr)
{
    block_p block, buddy, pool;
    u64_t order, pool_order;

    if (ptr == NULL)
        return;

    block = blockof(ptr);
    pool = block->pool;
    pool_order = block->pool_order;
    order = block->order;

    // debug("%sFreeing block: %p, order: %lld Pool: %p%s\n", CYAN, block, order, pool, RESET);

    for (;; order++)
    {
        // check if we are at the root block (no buddies left)
        if (order == pool_order)
            return insert_block(block, order);

        // calculate buddy
        buddy = buddyof(block, pool, order);

        // debug("Block: %p Buddy: %p Order: %lld, Pool order: %lld Buddy order: %lld Buddy used: %d\n",
        //       block, buddy, order, pool_order, buddy->order, buddy->used);

        // buddy is used, so we can't merge
        if (buddy->order != order || buddy->used)
            return insert_block(block, order);

        // merge blocks: remove buddy from its freelist.
        remove_block(buddy, order);

        // check if buddy is lower address than block (means it is of higher order), if so, swap them
        block = (buddy > block) ? block : buddy;
    }
}

__attribute__((hot)) raw_p heap_realloc(raw_p ptr, u64_t new_size)
{
    block_p block;
    u64_t i, old_size, size, order;
    raw_p new_ptr;

    if (ptr == NULL)
        return heap_alloc(new_size);

    if (new_size == 0)
    {
        heap_free(ptr);
        return NULL;
    }

    block = blockof(ptr);
    old_size = bsizeof(block->order) - BLOCK_HEADER_SIZE;

    if (new_size == old_size)
        return ptr;

    size = blocksize(new_size);

    // grow
    if (new_size > old_size)
    {
        new_ptr = heap_alloc(new_size);

        if (new_ptr)
            memcpy(new_ptr, ptr, old_size);

        heap_free(ptr);

        return new_ptr;
    }

    // resize
    i = block->order;
    order = orderof(size);
    block->order = order;

    split_block(block, i, order, block->pool, block->pool_order);

    return ptr;
}

i64_t heap_gc(nil_t)
{
    u64_t i, size, avail_mask = __HEAP->avail, order, total = 0;
    block_p block, next;

    while (avail_mask != 0)
    {
        i = __builtin_ctzll(avail_mask);
        size = bsizeof(i);
        avail_mask &= ~size;

        block = __HEAP->freelist[i];
        while (block)
        {
            next = block->next;
            order = block->pool_order;
            if (order == i)
            {
                remove_block(block, i);
                mmap_free(block, size);

                __HEAP->memstat.heap -= size;
                __HEAP->memstat.system -= size;
                total += size;
            }

            block = next;
        }
    }

    return total;
}

nil_t heap_borrow(heap_p heap)
{
    u64_t i, size, avail_mask = __HEAP->avail;

    while (avail_mask != 0)
    {
        i = __builtin_ctzll(avail_mask);
        size = bsizeof(i);
        avail_mask &= ~size;

        // Only borrow if the source heap has a freelist[i] and it has more than one node
        if (__HEAP->freelist[i]->next == NULL)
            continue;

        heap->freelist[i] = __HEAP->freelist[i];
        __HEAP->freelist[i] = __HEAP->freelist[i]->next;
        __HEAP->freelist[i]->prev = NULL;

        heap->freelist[i]->next = NULL;
        heap->freelist[i]->prev = NULL;
        heap->avail |= size;
    }
}

nil_t heap_merge(heap_p heap)
{
    u64_t i, avail_mask = heap->avail;
    block_p block, next;

    while (avail_mask != 0)
    {
        i = __builtin_ctzll(avail_mask);
        avail_mask &= ~bsizeof(i);

        block = heap->freelist[i];

        while (block)
        {
            next = block;
            block = block->next;
            heap_free(addrof(next));
        }

        heap->freelist[i] = NULL;
    }

    // Update memory statistics
    __HEAP->memstat.heap += heap->memstat.heap;
    __HEAP->memstat.system += heap->memstat.system;

    // Clear borrowed heap
    heap->memstat.heap = 0;
    heap->memstat.system = 0;
    heap->avail = 0;
}

memstat_t heap_memstat(nil_t)
{
    u64_t i;
    block_p block;

    // heap_print_blocks(__HEAP);

    __HEAP->memstat.free = 0;

    // calculate free blocks
    for (i = MIN_ORDER; i <= MAX_POOL_ORDER; i++)
    {
        block = __HEAP->freelist[i];
        while (block)
        {
            __HEAP->memstat.free += blocksize(i);
            block = block->next;
        }
    }

    return __HEAP->memstat;
}

nil_t heap_cleanup(nil_t)
{
    u64_t i, order;
    block_p block, next;

    // All the nodes remains are pools, so just munmap them
    for (i = 0; i <= MAX_POOL_ORDER; i++)
    {
        block = __HEAP->freelist[i];
        while (block)
        {
            next = block->next;
            order = block->pool_order;
            if (order != i)
            {
                debug("%s-- HEAP[%lld]: leak order: %lld block: %p%s", RED, __HEAP->id, i, block, RESET);
                return;
            }

            mmap_free(block, bsizeof(order));
            block = next;
        }
    }

    mmap_free(__HEAP, sizeof(struct heap_t));

    __HEAP = NULL;
}

nil_t heap_print_blocks(heap_p heap)
{
    u64_t i;
    block_p block;

    printf("-- HEAP[%lld]: BLOCKS:\n", heap->id);
    for (i = 0; i <= MAX_POOL_ORDER; i++)
    {
        block = heap->freelist[i];
        printf("-- order: %lld [", i);
        while (block)
        {
            printf("%p, ", block);
            block = block->next;
        }
        printf("]\n");
    }
}

#endif

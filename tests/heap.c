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

test_result_t test_allocate_and_free()
{
    u64_t size = 1024; // size of the memory block to allocate
    nil_t *ptr = heap_alloc(size);
    TEST_ASSERT(ptr != NULL, "ptr != NULL");
    heap_free(ptr);

    PASS();
}

test_result_t test_multiple_allocations()
{
    u64_t size = 1024;
    nil_t *ptr1 = heap_alloc(size);
    nil_t *ptr2 = heap_alloc(size);
    TEST_ASSERT(ptr1 != NULL, "ptr1 != NULL");
    TEST_ASSERT(ptr2 != NULL, "ptr2 != NULL");
    TEST_ASSERT(ptr1 != ptr2, "ptr1 != ptr2");
    heap_free(ptr1);
    heap_free(ptr2);

    PASS();
}

test_result_t test_allocation_after_free()
{
    u64_t size = 1024;
    nil_t *ptr1 = heap_alloc(size);
    TEST_ASSERT(ptr1 != NULL, "ptr1 != NULL");
    heap_free(ptr1);

    nil_t *ptr2 = heap_alloc(size);
    TEST_ASSERT(ptr2 != NULL, "ptr2 != NULL");

    // the second allocation should be able to use the block freed by the first allocation
    TEST_ASSERT(ptr1 == ptr2, "ptr1 == ptr2");

    heap_free(ptr2);

    PASS();
}

test_result_t test_out_of_memory()
{
    u64_t size = 1ull << 38;
    nil_t *ptr = heap_alloc(size);
    TEST_ASSERT(ptr == NULL, "ptr == NULL");

    PASS();
}

test_result_t test_large_number_of_allocations()
{
    i64_t i, num_allocs = 10000000; // Large number of allocations
    u64_t size = 1024;
    nil_t **ptrs = heap_alloc(num_allocs * sizeof(nil_t *));
    for (i = 0; i < num_allocs; i++)
    {
        ptrs[i] = heap_alloc(size);
        TEST_ASSERT(ptrs[i] != NULL, "ptrs[i] != NULL");
    }
    // Free memory in reverse order
    for (i = num_allocs - 1; i >= 0; i--)
        heap_free(ptrs[i]);

    heap_free(ptrs);

    PASS();
}

test_result_t test_varying_sizes()
{
    u64_t size = 16;       // Start size
    u64_t num_allocs = 10; // number of allocations
    nil_t *ptrs[num_allocs];
    i32_t i;

    for (u64_t i = 0; i < num_allocs; i++)
    {
        ptrs[i] = heap_alloc(size << i); // double the size at each iteration
        TEST_ASSERT(ptrs[i] != NULL, "ptrs[i] != NULL");
    }
    // Free memory in reverse order
    for (i = num_allocs - 1; i >= 0; i--)
        heap_free(ptrs[i]);

    PASS();
}

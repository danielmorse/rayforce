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

#ifndef POOL_H
#define POOL_H

#include <pthread.h>
#include "rayforce.h"

typedef struct task_t
{
    nil_t (*fn)(raw_p);
    raw_p arg;
    u64_t len;
} *task_p;

typedef struct result_t
{
    raw_p data;
    u64_t len;
} *result_p;

typedef struct shared_t
{
    pthread_mutex_t lock;  // Mutex for condition variable
    pthread_cond_t run;    // Condition variable to signal executors
    pthread_cond_t done;   // Condition variable to signal main thread
    task_p tasks;          // Array of input tasks for executors
    result_p results;      // Array of results from executors
    u64_t tasks_remaining; // Counter to track remaining tasks
    b8_t stop;             // Flag to indicate if the pool is stopped
} *shared_p;

typedef struct executor_t
{
    u64_t id;
    pthread_t handle;
} *executor_p;

typedef struct pool_t
{
    executor_p executors;  // Array of executors
    u64_t executors_count; // Number of executors
    shared_p shared;       // Shared data
} *pool_p;

pool_p pool_new(u64_t executors_count);
nil_t pool_free(pool_p pool);
nil_t pool_run(pool_p pool);
result_p pool_wait(pool_p pool);
nil_t pool_stop(pool_p pool);

#endif // POOL_H

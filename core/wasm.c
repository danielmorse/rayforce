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
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "wasm.h"
#include "parse.h"
#include "hash.h"
#include "format.h"
#include "util.h"
#include "string.h"
#include "poll.h"
#include "heap.h"

// Use EM_JS to define JavaScript functions to be called from C
EM_JS(nil_t, js_appendOutput, (str_t text), {
    Module.appendOutput(UTF8ToString(text));
});

poll_t poll_init(i64_t port)
{
    poll_t poll = (poll_t)heap_alloc(sizeof(struct poll_t));
    poll->code = NULL_I64;

    return poll;
}

nil_t poll_cleanup(poll_t poll)
{
    heap_free(poll);
}

i64_t poll_run(poll_t poll)
{
    unused(poll);
    // emscripten_set_main_loop(wasm_repl, -1, 1);
    return 0;
}

i64_t poll_register(poll_t poll, i64_t fd, u8_t version)
{
    unused(poll);
    unused(fd);
    unused(version);
    return 0;
}

nil_t poll_deregister(poll_t poll, i64_t id)
{
    unused(poll);
    unused(id);
}

obj_t ipc_send_sync(poll_t poll, i64_t id, obj_t msg)
{
    unused(poll);
    unused(id);
    unused(msg);
    return NULL_OBJ;
}

obj_t ipc_send_async(poll_t poll, i64_t id, obj_t msg)
{
    unused(poll);
    unused(id);
    unused(msg);
    return NULL_OBJ;
}

nil_t printjs(str_t str)
{
    js_appendOutput(str);
}

// Define the wasm_repl function to be called from JavaScript
EMSCRIPTEN_KEEPALIVE nil_t wasm_repl(str_t input)
{
    obj_t src, file, res;
    str_t fmt;
    u64_t n;

    n = strlen(input);

    if (n == 0)
        return;

    src = string_from_str(input, n);
    file = string_from_str("webrepl", 7);
    res = eval_str(0, src, file);
    drop(src);
    drop(file);

    fmt = obj_fmt(res);
    js_appendOutput(fmt);
    heap_free(fmt);
    drop(res);
}

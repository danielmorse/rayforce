/*
 *   Copyright (c) 2025 Anton Kundenko <singaraiona@gmail.com>
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

#include "os.h"
#include "error.h"
#include "string.h"
#include <stdio.h>

i64_t os_get_var(lit_p name, c8_t buf[], u64_t len) {
    str_p value;

    if (!name || !buf || len == 0) {
        fprintf(stderr, "Invalid arguments.\n");
        return -1;
    }

#if defined(OS_WINDOWS)
    // Windows implementation using GetEnvironmentVariable
    DWORD result = GetEnvironmentVariableA(var_name, buffer, (DWORD)length);
    if (result == 0)
        return -1;

    if (result >= length) {
        fprintf(stderr, "Buffer too small for environment variable '%s'.\n", var_name);
        return -1;
    }
#else
    // UNIX implementation using getenv
    value = getenv(name);
    if (!value)
        return -1;

    if (strlen(value) >= len) {
        fprintf(stderr, "Buffer too small for environment variable '%s'.\n", name);
        return -1;
    }

    strncpy(buf, value, len - 1);
    buf[len - 1] = '\0';  // Ensure null termination

#endif

    return 0;
}

i64_t os_set_var(lit_p name, u64_t nlen, lit_p value, u64_t vlen) {
    c8_t var_name[256], var_value[1024];

    if (!name || !value || nlen == 0 || vlen == 0) {
        return -1;  // Invalid parameters
    }

    // Ensure name and value are null-terminated
    if (nlen >= sizeof(var_name) || vlen >= sizeof(var_value)) {
        return -2;  // Name or value too long
    }

    strncpy(var_name, name, nlen);
    var_name[nlen] = '\0';  // Null terminate

    strncpy(var_value, value, vlen);
    var_value[vlen] = '\0';  // Null terminate

#if defined(OS_WINDOWS)
    // Windows implementation using SetEnvironmentVariable
    if (!SetEnvironmentVariableA(var_name, var_value))
        return -3;
#else
    // UNIX implementation using setenv
    if (setenv(var_name, var_value, 1) != 0)
        return -3;

#endif

    return 0;  // Success
}

obj_p ray_os_get_var(obj_p x) {
    c8_t buf[1024];
    i64_t res;

    if (x->type != TYPE_C8)
        THROW(ERR_TYPE, "os-get-var: expected string");

    res = os_get_var(AS_C8(x), buf, sizeof(buf));
    if (res == -1)
        THROW(ERR_OS, "os-get-var: failed to get environment variable");

    return string_from_str(buf, strlen(buf));
}

obj_p ray_os_set_var(obj_p x, obj_p y) {
    i64_t res;

    if (x->type != TYPE_C8 || y->type != TYPE_C8)
        THROW(ERR_TYPE, "os-set-var: expected strings");

    res = os_set_var(AS_C8(x), x->len, AS_C8(y), y->len);
    if (res == -1)
        THROW(ERR_OS, "os-set-var: invalid arguments");
    if (res == -2)
        THROW(ERR_OS, "os-set-var: name or value too long");
    if (res == -3)
        THROW(ERR_OS, "os-set-var: failed to set environment variable");

    return NULL_OBJ;
}
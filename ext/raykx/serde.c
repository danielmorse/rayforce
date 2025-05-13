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

#include "serde.h"
#include "raykx.h"
#include "k.h"
#include <stdio.h>
#include <string.h>
#include "../../core/util.h"
#include "../../core/ops.h"
#include "../../core/log.h"
#include "../../core/error.h"

/*
 * Returns size (in bytes) that an obj occupy in memory via serialization into KDB+ IPC format
 */
i64_t raykx_size_obj(obj_p obj) {
    switch (obj->type) {
        case -TYPE_B8:
            return ISIZEOF(i8_t) + ISIZEOF(b8_t);
        case -TYPE_U8:
            return ISIZEOF(i8_t) + ISIZEOF(u8_t);
        case -TYPE_I16:
            return ISIZEOF(i8_t) + ISIZEOF(i16_t);
        case -TYPE_I32:
        case -TYPE_DATE:
        case -TYPE_TIME:
            return ISIZEOF(i8_t) + ISIZEOF(i32_t);

        case -TYPE_I64:
        case -TYPE_TIMESTAMP:
            return ISIZEOF(i8_t) + ISIZEOF(i64_t);

        case -TYPE_F64:
            return ISIZEOF(i8_t) + ISIZEOF(f64_t);

            // case -TYPE_SYMBOL:
            //     return ISIZEOF(i8_t) + strlen(str_from_symbol(obj->i64)) + 1;

        case -TYPE_C8:
            return ISIZEOF(i8_t) + ISIZEOF(c8_t);
        case -TYPE_GUID:
            return ISIZEOF(i8_t) + ISIZEOF(guid_t);

        case TYPE_GUID:
            return ISIZEOF(i8_t) + ISIZEOF(i64_t) + obj->len * ISIZEOF(guid_t);
        case TYPE_B8:
            return ISIZEOF(i8_t) + ISIZEOF(i64_t) + obj->len * ISIZEOF(b8_t);
        case TYPE_U8:
            return ISIZEOF(i8_t) + ISIZEOF(i64_t) + obj->len * ISIZEOF(u8_t);
        case TYPE_I64:
        case TYPE_TIMESTAMP:
            return ISIZEOF(i8_t) + ISIZEOF(i64_t) + obj->len * ISIZEOF(i64_t);
        case TYPE_F64:
            return ISIZEOF(i8_t) + ISIZEOF(i64_t) + obj->len * ISIZEOF(f64_t);
        case TYPE_C8:
            return ISIZEOF(i8_t) + 1 + ISIZEOF(i32_t) + obj->len * ISIZEOF(c8_t);
        // case TYPE_SYMBOL:
        //     l = obj->len;
        //     size = ISIZEOF(i8_t) + ISIZEOF(i32_t);
        //     for (i = 0; i < l; i++)
        //         size += strlen(str_from_symbol(AS_SYMBOL(obj)[i])) + 1;
        //     return size;
        // case TYPE_LIST:
        //     l = obj->len;
        //     size = ISIZEOF(i8_t) + ISIZEOF(i64_t);
        //     for (i = 0; i < l; i++)
        //         size += size_obj(AS_LIST(obj)[i]);
        //     return size;
        // case TYPE_TABLE:
        // case TYPE_DICT:
        //     return ISIZEOF(i8_t) + size_obj(AS_LIST(obj)[0]) + size_obj(AS_LIST(obj)[1]);
        // // case TYPE_LAMBDA:
        // //     return ISIZEOF(i8_t) + size_obj(AS_LAMBDA(obj)->args) + size_obj(AS_LAMBDA(obj)->body);
        // case TYPE_UNARY:
        // case TYPE_BINARY:
        // case TYPE_VARY:
        //     return ISIZEOF(i8_t) + strlen(env_get_internal_name(obj)) + 1;
        // case TYPE_NULL:
        //     return ISIZEOF(i8_t);
        // case TYPE_ERR:
        //     return ISIZEOF(i8_t) + ISIZEOF(i8_t) + size_obj(AS_ERROR(obj)->msg);
        default:
            return 0;
    }
}

i64_t raykx_ser_obj(u8_t *buf, i64_t len, obj_p obj) {
    UNUSED(len);
    i32_t l;

    switch (obj->type) {
        case TYPE_NULL:
            return ISIZEOF(i8_t);
        case -TYPE_B8:
            buf[0] = -KB;
            buf[1] = obj->b8;
            return ISIZEOF(i8_t) + ISIZEOF(b8_t);
        case -TYPE_I32:
            buf[0] = -KI;
            buf++;
            memcpy(buf, &obj->i32, sizeof(i32_t));
            return ISIZEOF(i8_t) + ISIZEOF(i32_t);
        case -TYPE_I64:
            buf[0] = -KJ;
            buf++;
            memcpy(buf, &obj->i64, sizeof(i64_t));
            return ISIZEOF(i8_t) + ISIZEOF(i64_t);
        case TYPE_C8:
            buf[0] = KC;
            buf++;
            buf[0] = 0;  // attrs
            buf++;
            l = (i32_t)obj->len;
            memcpy(buf, &l, sizeof(i32_t));
            buf += sizeof(i32_t);
            memcpy(buf, AS_C8(obj), obj->len);
            return ISIZEOF(i8_t) + 1 + sizeof(i32_t) + obj->len;
        default:
            return 0;
    }
}

obj_p raykx_load_obj(u8_t *buf, i64_t *len) {
    i64_t l;
    obj_p obj;
    i8_t type;

    if (*len == 0)
        return NULL_OBJ;

    type = *buf;
    buf++;
    len--;

    switch (type) {
        case -KB:
            obj = b8(buf[0]);
            buf++;
            len--;
            return obj;
        case -KJ:
            obj = i64(0);
            memcpy(&obj->i64, buf, sizeof(i64_t));
            buf += sizeof(i64_t);
            len -= sizeof(i64_t);
            return obj;
        case KC:
            buf++;  // attrs
            memcpy(&l, buf, sizeof(i32_t));
            buf += sizeof(i32_t);
            len -= sizeof(i32_t);
            obj = C8(l);
            memcpy(AS_C8(obj), buf, l);
            buf += l;
            len -= l;
            return obj;
        default:
            return NULL_OBJ;
    }
}

obj_p raykx_des_obj(u8_t *buf, i64_t len) {
    i64_t l;
    raykx_header_p header;

    // Check if buffer is large enough to contain a header
    if (len < ISIZEOF(struct raykx_header_t))
        return error_str(ERR_IO, "raykx_des_obj: buffer too small to contain header");

    header = (raykx_header_p)buf;
    LOG_DEBUG("Deserializing object of size %lld", header->size);

    // Check for reasonable size values
    if (header->size > 1000000000)  // 1GB max size
        return error_str(ERR_IO, "raykx_des_obj: unreasonable size in header, possible corruption");

    // Check for reasonable size values
    if (header->size != len)
        return error_str(ERR_IO, "raykx_des_obj: corrupted data in a buffer");

    buf += sizeof(struct raykx_header_t);
    l = header->size;

    return raykx_load_obj(buf, &l);
}

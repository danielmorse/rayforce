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

#include "ipc.h"
#include "eval.h"
#include "binary.h"
#include "symbols.h"
#include "error.h"
#include "poll.h"
#include "string.h"
#include "util.h"
#include "log.h"

// ============================================================================
// Forward Declarations
// ============================================================================

static poll_result_t ipc_read_handshake(poll_p poll, selector_p selector);
static poll_result_t ipc_read_header(poll_p poll, selector_p selector);
static poll_result_t ipc_read_msg(poll_p poll, selector_p selector);
static poll_result_t ipc_on_open(poll_p poll, selector_p selector);
static poll_result_t ipc_on_close(poll_p poll, selector_p selector);
static poll_result_t ipc_on_error(poll_p poll, selector_p selector);
static poll_result_t ipc_send_handshake(poll_p poll, selector_p selector);
static poll_result_t ipc_send_msg(poll_p poll, selector_p selector);
static poll_result_t ipc_send_header(poll_p poll, selector_p selector);

// ============================================================================
// User FD Management
// ============================================================================

nil_t poll_set_usr_fd(i64_t fd) {
    obj_p s, k, v;

    s = symbol(".z.w", 4);
    k = i64(fd);
    v = binary_set(s, k);
    drop_obj(k);
    drop_obj(v);
    drop_obj(s);
}

// ============================================================================
// Listener Management
// ============================================================================

poll_result_t ipc_listener_accept(poll_p poll, selector_p selector) {
    i64_t fd;
    struct poll_registry_t registry = ZERO_INIT_STRUCT;
    ipc_ctx_p ctx;

    LOG_TRACE("Accepting new connection on fd %lld", selector->fd);
    fd = sock_accept(selector->fd);
    LOG_DEBUG("New connection accepted on fd %lld", fd);

    if (fd != -1) {
        ctx = (ipc_ctx_p)heap_alloc(sizeof(struct ipc_ctx_t));
        ctx->name = string_from_str("ipc", 4);

        registry.fd = fd;
        registry.type = SELECTOR_TYPE_SOCKET;
        registry.events = POLL_EVENT_READ | POLL_EVENT_WRITE | POLL_EVENT_ERROR | POLL_EVENT_HUP;
        registry.open_fn = ipc_on_open;
        registry.close_fn = ipc_on_close;
        registry.error_fn = ipc_on_error;
        registry.read_fn = ipc_read_handshake;
        registry.recv_fn = sock_recv;
        registry.send_fn = sock_send;
        registry.data = ctx;

        if (poll_register(poll, &registry) == -1) {
            LOG_ERROR("Failed to register new connection in poll registry");
            heap_free(ctx);
            return POLL_ERROR;
        }

        LOG_INFO("New connection registered successfully");
    }

    return POLL_OK;
}

poll_result_t ipc_listener_close(poll_p poll, selector_p selector) {
    UNUSED(poll);
    UNUSED(selector);

    return POLL_OK;
}

poll_result_t ipc_listen(poll_p poll, i64_t port) {
    i64_t fd;
    struct poll_registry_t registry = ZERO_INIT_STRUCT;

    if (poll == NULL)
        return POLL_ERROR;

    fd = sock_listen(port);
    if (fd == -1)
        return POLL_ERROR;

    registry.fd = fd;
    registry.type = SELECTOR_TYPE_SOCKET;
    registry.events = POLL_EVENT_READ | POLL_EVENT_ERROR | POLL_EVENT_HUP;
    registry.recv_fn = NULL;
    registry.read_fn = ipc_listener_accept;
    registry.close_fn = ipc_listener_close;
    registry.error_fn = NULL;
    registry.data = NULL;

    LOG_DEBUG("Registering listener on port %lld", port);

    return poll_register(poll, &registry);
}

// ============================================================================
// User Callback Management
// ============================================================================

poll_result_t ipc_call_usr_cb(poll_p poll, selector_p selector, lit_p sym, i64_t len) {
    UNUSED(poll);
    i64_t clbnm;
    obj_p v, f, *clbfn;

    stack_push(NULL_OBJ);  // null env
    clbnm = symbols_intern(sym, len);
    clbfn = resolve(clbnm);
    stack_pop();  // null env

    // Call the callback if it's a lambda
    if (clbfn != NULL && (*clbfn)->type == TYPE_LAMBDA) {
        poll_set_usr_fd(selector->id);
        stack_push(i64(selector->id));
        v = call(*clbfn, 1);
        drop_obj(stack_pop());
        poll_set_usr_fd(0);
        if (IS_ERR(v)) {
            f = obj_fmt(v, B8_FALSE);
            fprintf(stderr, "Error in user callback: \n%.*s\n", (i32_t)f->len, AS_C8(f));
            drop_obj(f);
        }

        drop_obj(v);
    }

    return POLL_OK;
}

// ============================================================================
// Connection Management
// ============================================================================

poll_result_t ipc_open(poll_p poll, sock_addr_t *addr, i64_t timeout) {
    i64_t fd, id;
    selector_p selector;
    struct poll_registry_t registry = ZERO_INIT_STRUCT;
    ipc_ctx_p ctx;
    u8_t handshake[2] = {RAYFORCE_VERSION, 0x00};

    LOG_DEBUG("Opening connection to %s:%lld", addr->ip, addr->port);

    fd = sock_open(addr, timeout);
    LOG_DEBUG("Connection opened on fd %lld", fd);

    if (fd == -1)
        return POLL_ERROR;

    if (sock_send(fd, handshake, 2) == -1)
        return POLL_ERROR;

    if (sock_recv(fd, handshake, 2) == -1)
        return POLL_ERROR;

    LOG_TRACE("Setting socket to non-blocking mode");
    sock_set_nonblocking(fd, B8_TRUE);
    LOG_TRACE("Socket set to non-blocking mode");

    ctx = (ipc_ctx_p)heap_alloc(sizeof(struct ipc_ctx_t));
    ctx->name = string_from_str("ipc", 4);

    registry.fd = fd;
    registry.type = SELECTOR_TYPE_SOCKET;
    registry.events = POLL_EVENT_READ | POLL_EVENT_WRITE | POLL_EVENT_ERROR | POLL_EVENT_HUP;
    registry.recv_fn = sock_recv;
    registry.send_fn = sock_send;
    registry.read_fn = ipc_read_header;
    registry.close_fn = ipc_on_close;
    registry.error_fn = ipc_on_error;
    registry.data = ctx;

    LOG_DEBUG("Registering connection in poll registry");
    id = poll_register(poll, &registry);
    LOG_DEBUG("Connection registered in poll registry with id %lld", id);

    // request rx buffer
    selector = poll_get_selector(poll, id);
    if (poll_rx_buf_request(poll, selector, ISIZEOF(struct ipc_header_t)) == POLL_ERROR) {
        poll_deregister(poll, id);
        return POLL_ERROR;
    }

    return id;
}

// ============================================================================
// Message Reading
// ============================================================================

poll_result_t ipc_read_handshake(poll_p poll, selector_p selector) {
    UNUSED(poll);

    poll_buffer_p buf;
    u8_t handshake[2] = {RAYFORCE_VERSION, 0x00};

    if (selector->rx.buf == NULL) {
        LOG_DEBUG("No handshake buffer received, closing connection");
        return poll_deregister(poll, selector->id);
    }

    if (selector->rx.buf->offset > 0 && selector->rx.buf->data[selector->rx.buf->offset - 1] == '\0') {
        LOG_DEBUG("Handshake received, sending response");
        // send handshake response
        buf = poll_buf_create(ISIZEOF(handshake));
        memcpy(buf->data, handshake, ISIZEOF(handshake));
        poll_send_buf(poll, selector, buf);

        // Now we are ready for income messages and can call userspace callback (if any)
        ipc_call_usr_cb(poll, selector, ".z.po", 5);

        selector->rx.read_fn = ipc_read_header;
        LOG_DEBUG("Handshake completed, switching to header reading mode");

        poll_rx_buf_request(poll, selector, ISIZEOF(struct ipc_header_t));
        poll_rx_buf_reset(poll, selector);

        return POLL_READY;
    }

    return POLL_OK;
}

poll_result_t ipc_read_header(poll_p poll, selector_p selector) {
    UNUSED(poll);

    ipc_header_t *header;

    LOG_DEBUG("Reading header from connection %lld", selector->id);

    header = (ipc_header_t *)selector->rx.buf->data;

    LOG_TRACE("Header read: {.prefix: 0x%08x, .version: %d, .flags: %d, .endian: %d, .msgtype: %d, .size: %lld}",
              header->prefix, header->version, header->flags, header->endian, header->msgtype, header->size);

    // request the buffer for the entire message (including the header)
    LOG_DEBUG("Requesting buffer for message of size %lld", ISIZEOF(struct ipc_header_t) + header->size);
    poll_rx_buf_request(poll, selector, ISIZEOF(struct ipc_header_t) + header->size);

    LOG_DEBUG("Switching to message reading mode");
    selector->rx.read_fn = ipc_read_msg;

    return POLL_READY;
}

poll_result_t ipc_read_msg(poll_p poll, selector_p selector) {
    UNUSED(poll);

    i64_t size;
    obj_p v, res;
    ipc_ctx_p ctx;
    ipc_header_t *header;
    poll_buffer_p buf;

    if (selector == NULL || selector->rx.buf == NULL) {
        LOG_DEBUG("Connection closed or no buffer available");
        return POLL_ERROR;
    }

    ctx = (ipc_ctx_p)selector->data;
    if (ctx == NULL || ctx->name == NULL) {
        LOG_DEBUG("Invalid context or name");
        return POLL_ERROR;
    }

    LOG_DEBUG("Reading message from connection %.*s", (i32_t)ctx->name->len, AS_C8(ctx->name));

    res = de_raw(selector->rx.buf->data, selector->rx.buf->size);

    poll_set_usr_fd(selector->id);

    if (IS_ERR(res) || is_null(res))
        v = res;
    else if (res->type == TYPE_C8) {
        LOG_TRACE("Evaluating string message: %.*s", (i32_t)res->len, AS_C8(res));
        v = ray_eval_str(res, ctx->name);
        drop_obj(res);
    } else {
        LOG_TRACE("Evaluating object message");
        v = eval_obj(res);
        drop_obj(res);
    }

    LOG_TRACE_OBJ("Resulting object: ", v);

    poll_set_usr_fd(0);

    poll_rx_buf_release(poll, selector);
    poll_rx_buf_request(poll, selector, ISIZEOF(struct ipc_header_t));

    selector->rx.read_fn = ipc_read_header;

    // respond
    size = size_obj(v);
    buf = poll_buf_create(ISIZEOF(struct ipc_header_t) + size);
    ser_raw(buf->data, size, v);
    header = (ipc_header_t *)buf->data;
    header->msgtype = MSG_TYPE_RESP;
    // release the object
    drop_obj(v);

    LOG_DEBUG("Sending response message of size %lld", size);
    return poll_send_buf(poll, selector, buf);
}

poll_result_t ipc_read_msg_async(poll_p poll, selector_p selector) {
    UNUSED(poll);
    UNUSED(selector);

    return POLL_OK;
}

// ============================================================================
// Event Handlers
// ============================================================================

poll_result_t ipc_on_open(poll_p poll, selector_p selector) {
    LOG_DEBUG("Connection opened, requesting handshake buffer");
    // request the minimal handshake buffer
    return poll_rx_buf_request(poll, selector, 2);
}

poll_result_t ipc_on_error(poll_p poll, selector_p selector) {
    UNUSED(poll);
    UNUSED(selector);
    LOG_ERROR("Error occurred on connection %lld", selector->id);
    return POLL_OK;
}

poll_result_t ipc_on_close(poll_p poll, selector_p selector) {
    ipc_ctx_p ctx;

    LOG_INFO("Connection %lld closed", selector->id);

    // Clear any pending read operations
    selector->rx.read_fn = NULL;
    if (selector->rx.buf != NULL) {
        poll_rx_buf_release(poll, selector);
    }

    // Call user callback before freeing context
    ipc_call_usr_cb(poll, selector, ".z.pc", 5);

    // Free context
    ctx = (ipc_ctx_p)selector->data;
    if (ctx != NULL) {
        drop_obj(ctx->name);
        heap_free(ctx);
    }

    return POLL_OK;
}

// ============================================================================
// Message Sending
// ============================================================================

obj_p ipc_send_sync(poll_p poll, i64_t id, obj_p msg) {
    selector_p selector;
    poll_result_t result;
    poll_buffer_p buf;
    i64_t idx, size;
    obj_p res = NULL_OBJ;
    fd_set fds;
    struct timeval timeout;

    LOG_DEBUG("Starting synchronous IPC send for id %lld", id);

    // Get the selector for the given id
    idx = freelist_get(poll->selectors, id - SELECTOR_ID_OFFSET);
    if (idx == NULL_I64) {
        LOG_ERROR("Invalid socket fd %lld", id);
        return sys_error(ERR_IO, "ipc_send_sync: invalid socket fd");
    }

    selector = (selector_p)idx;
    if (selector == NULL) {
        LOG_ERROR("Invalid selector for fd %lld", id);
        return sys_error(ERR_IO, "ipc_send_sync: invalid selector for fd");
    }

    LOG_DEBUG("Setting up message buffer for fd %lld", selector->fd);
    // Set up the message in the selector's transmit buffer
    size = size_obj(msg);
    buf = poll_buf_create(ISIZEOF(struct ipc_header_t) + size);
    if (buf == NULL) {
        LOG_ERROR("Failed to create message buffer");
        return sys_error(ERR_IO, "ipc_send_sync: failed to create message buffer");
    }

    LOG_TRACE("Serializing message");
    ser_raw(buf->data, size, msg);
    ((ipc_header_t *)buf->data)->msgtype = MSG_TYPE_SYNC;

    LOG_DEBUG("Sending message on fd %lld", selector->fd);
    result = poll_send_buf(poll, selector, buf);
    if (result == POLL_ERROR) {
        LOG_ERROR("Failed to send message on fd %lld", selector->fd);
        return sys_error(ERR_IO, "ipc_send_sync: error sending message");
    }

    return NULL_OBJ;
}

obj_p ipc_send_async(poll_p poll, i64_t id, obj_p msg) {
    UNUSED(poll);
    UNUSED(id);
    UNUSED(msg);

    // idx = freelist_get(poll->selectors, id - SELECTOR_ID_OFFSET);

    // if (idx == NULL_I64)
    //     THROW(ERR_IO, "ipc_send_sync: invalid socket fd: %lld", id);

    // selector = (selector_p)idx;
    // if (selector == NULL)
    //     THROW(ERR_IO, "ipc_send_async: invalid socket fd: %lld", id);

    // queue_push(selector->tx.queue, (nil_t *)msg);

    // if (_send(poll, selector) == POLL_ERROR)
    //     THROW(ERR_IO, "ipc_send_async: error sending message");

    return NULL_OBJ;
}

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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../../core/rayforce.h"
#include "../../core/poll.h"
#include "../../core/sock.h"
#include "../../core/error.h"
#include "../../core/log.h"
#include "../../core/string.h"
#include "../../core/runtime.h"
#include "raykx.h"

// Just to test the ipc.h file
#include "../../core/ipc.h"

// Forward declarations
static option_t raykx_read_header(poll_p poll, selector_p selector);
static option_t raykx_read_msg(poll_p poll, selector_p selector);
static nil_t raykx_on_error(poll_p poll, selector_p selector);
static nil_t raykx_on_close(poll_p poll, selector_p selector);

// ============================================================================
// Connection Management
// ============================================================================

obj_p raykx_hopen(obj_p addr) {
    i64_t fd, id;
    struct poll_registry_t registry = ZERO_INIT_STRUCT;
    raykx_ctx_p ctx;
    u8_t handshake[2] = {0x00, 0x00};  // KDB+ handshake
    sock_addr_t sock_addr;
    selector_p selector;

    LOG_DEBUG("Opening KDB+ connection to %.*s", (i32_t)addr->len, AS_C8(addr));

    // Parse address string into sock_addr_t
    if (sock_addr_from_str(AS_C8(addr), addr->len, &sock_addr) == -1) {
        return error(ERR_IO, "raykx_open: invalid address format");
    }

    // Open socket connection
    fd = sock_open(&sock_addr, 5000);  // 5 second timeout
    LOG_DEBUG("Connection opened on fd %lld", fd);

    if (fd == -1)
        return error(ERR_IO, "raykx_open: failed to open connection");

    // Send handshake
    if (sock_send(fd, handshake, 2) == -1)
        return error(ERR_IO, "raykx_open: failed to send handshake");

    // Receive handshake response
    if (sock_recv(fd, handshake, 2) == -1)
        return error(ERR_IO, "raykx_open: failed to receive handshake");

    LOG_TRACE("Setting socket to non-blocking mode");
    sock_set_nonblocking(fd, B8_TRUE);
    LOG_TRACE("Socket set to non-blocking mode");

    ctx = (raykx_ctx_p)heap_alloc(sizeof(struct raykx_ctx_t));
    ctx->name = string_from_str("raykx", 6);
    ctx->msgtype = KDB_MSG_SYNC;

    registry.fd = fd;
    registry.type = SELECTOR_TYPE_SOCKET;
    registry.events = POLL_EVENT_READ | POLL_EVENT_ERROR | POLL_EVENT_HUP;
    registry.recv_fn = sock_recv;
    registry.send_fn = sock_send;
    registry.read_fn = ipc_read_header;
    registry.close_fn = ipc_on_close;
    registry.error_fn = ipc_on_error;
    registry.data = ctx;

    LOG_DEBUG("Registering connection in poll registry");
    id = poll_register(runtime_get()->poll, &registry);
    LOG_DEBUG("Connection registered in poll registry with id %lld", id);

    selector = poll_get_selector(runtime_get()->poll, id);
    poll_rx_buf_request(runtime_get()->poll, selector, 16);

    return i64(id);
}

// ============================================================================
// Message Reading
// ============================================================================

static option_t raykx_read_header(poll_p poll, selector_p selector) {
    UNUSED(poll);
    u8_t header[8];  // KDB+ message header: 1 byte msg type, 1 byte compression, 2 bytes unused, 4 bytes size

    if (sock_recv(selector->fd, header, 8) == -1) {
        LOG_ERROR("Failed to read KDB+ message header");
        return option_error(sys_error(ERR_IO, "raykx_read_header: failed to read header"));
    }

    i32_t size = *(i32_t *)(header + 4);
    LOG_DEBUG("Reading KDB+ message of size %d", size);

    // Request buffer for the message
    poll_rx_buf_request(poll, selector, size);
    selector->rx.read_fn = raykx_read_msg;

    return option_some(NULL);
}

static option_t raykx_read_msg(poll_p poll, selector_p selector) {
    UNUSED(poll);
    obj_p res;

    LOG_DEBUG("Reading KDB+ message from connection %lld", selector->id);
    // TODO: Implement KDB+ message deserialization
    res = null(0);

    // Prepare for next message
    poll_rx_buf_release(poll, selector);
    selector->rx.read_fn = raykx_read_header;

    return option_some(res);
}

// ============================================================================
// Event Handlers
// ============================================================================

static nil_t raykx_on_error(poll_p poll, selector_p selector) {
    UNUSED(poll);
    UNUSED(selector);
    LOG_ERROR("Error occurred on KDB+ connection %lld", selector->id);
}

static nil_t raykx_on_close(poll_p poll, selector_p selector) {
    raykx_ctx_p ctx;

    LOG_INFO("KDB+ connection %lld closed", selector->id);

    // Clear any pending read operations
    selector->rx.read_fn = NULL;
    if (selector->rx.buf != NULL) {
        poll_rx_buf_release(poll, selector);
    }

    // Free context
    ctx = (raykx_ctx_p)selector->data;
    if (ctx != NULL) {
        drop_obj(ctx->name);
        heap_free(ctx);
    }
}

// ============================================================================
// Message Sending
// ============================================================================

obj_p raykx_send(obj_p fd, obj_p msg) {
    selector_p selector;
    raykx_ctx_p ctx;
    poll_p poll = runtime_get()->poll;

    LOG_DEBUG("Starting synchronous KDB+ send");

    return ipc_send(poll, fd->i64, msg, 1);
}
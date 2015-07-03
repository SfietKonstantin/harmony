/*
 * Copyright (C) 2014 Lucien XU <sfietkonstantin@free.fr>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * The names of its contributors may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include "enhancedcivetserver.h"
#include <assert.h>
#include <QtCore/QDebug>

namespace harmony { namespace private_impl {

EnhancedCivetServer::EnhancedCivetServer(const char **options, const mg_callbacks *callbacks)
    : CivetServer(options, callbacks)
{
}

EnhancedCivetServer::~EnhancedCivetServer()
{
    close();
}

std::string EnhancedCivetServer::getParameters(mg_connection *connection)
{
    const struct mg_request_info *ri = mg_get_request_info(connection);
    if (!ri->query_string) {
        return std::string();
    }
    return std::string(ri->query_string);
}

// Taking inspiration from getParam
std::string EnhancedCivetServer::getPostData(mg_connection *connection)
{
    const char *formParams = NULL;
    const struct mg_request_info *ri = mg_get_request_info(connection);
    assert(ri != NULL);
    EnhancedCivetServer *me = static_cast<EnhancedCivetServer *>(ri->user_data);
    assert(me != NULL);
    mg_lock_context(me->context);
    CivetConnection &conobj = me->connections[connection];
    mg_lock_connection(connection);
    mg_unlock_context(me->context);

    if (conobj.postData != NULL) {
        formParams = conobj.postData;
    } else {
        const char *con_len_str = mg_get_header(connection, "Content-Length");
        if (con_len_str) {
            unsigned long con_len = atoi(con_len_str);
            if (con_len > 0) {
                // Add one extra character: in case the post-data is a text, it
                // is required as 0-termination.
                // Do not increment con_len, since the 0 terminating is not part
                // of the content (text or binary).
                conobj.postData = (char *)malloc(con_len + 1);
                if (conobj.postData != NULL) {
                    // malloc may fail for huge requests
                    mg_read(connection, conobj.postData, con_len);
                    conobj.postData[con_len] = 0;
                    formParams = conobj.postData;
                    conobj.postDataLen = con_len;
                }
            }
        }
    }
    mg_unlock_connection(connection);
    if (!formParams) {
        return std::string();
    }
    return std::string(formParams);
}

void EnhancedCivetServer::addWebSocketHandler(const std::string &uri, CivetWebSocketHandler *handler)
{
    mg_set_websocket_handler(context, uri.c_str(), wsConnectHandler, wsReadyHandler,
                             wsDataHandler, wsCloseHandler, handler);
}

bool EnhancedCivetServer::wsWrite(mg_connection *connection, int opcode, const QByteArray &data)
{
    int value = mg_websocket_write(connection, opcode, data.data(), data.size());
    if (value == 0 && wsExists(connection)) {
        wsRemove(connection);
    }
    return value >= 0;
}

bool EnhancedCivetServer::wsExists(const mg_connection *connection) const
{
    std::lock_guard<std::mutex> lock (m_mutex);
    return m_webSockets.find(connection) != m_webSockets.end();
}

void EnhancedCivetServer::wsRemove(const mg_connection *connection)
{
    std::lock_guard<std::mutex> lock (m_mutex);
    m_webSockets.erase(connection);
}

int EnhancedCivetServer::wsConnectHandler(const mg_connection *connection, void *cwData)
{
    const struct mg_request_info *request_info = mg_get_request_info(connection);
    assert(request_info != NULL);
    EnhancedCivetServer *me = static_cast<EnhancedCivetServer *>(request_info->user_data);
    assert (!me->wsExists(connection));

    bool ok = static_cast<CivetWebSocketHandler *>(cwData)->handleConnect(me, connection);
    if (ok) {
        std::lock_guard<std::mutex> lock (me->m_mutex);
        me->m_webSockets.insert(connection);
    }

    return ok ? 0 : 1; // Accept when returing true
}

void EnhancedCivetServer::wsReadyHandler(mg_connection *connection, void *cwData)
{
    const struct mg_request_info *request_info = mg_get_request_info(connection);
    assert(request_info != NULL);
    EnhancedCivetServer *me = static_cast<EnhancedCivetServer *>(request_info->user_data);
    assert (me->wsExists(connection));
    static_cast<CivetWebSocketHandler *>(cwData)->handleReady(me, connection);
}

int EnhancedCivetServer::wsDataHandler(mg_connection *connection, int bits, char *data, size_t len, void *cwData)
{
    const struct mg_request_info *request_info = mg_get_request_info(connection);
    assert(request_info != NULL);
    EnhancedCivetServer *me = static_cast<EnhancedCivetServer *>(request_info->user_data);
    assert (me->wsExists(connection));
    int opcode = bits & 0xf;
    if (opcode != WEBSOCKET_OPCODE_TEXT && opcode != WEBSOCKET_OPCODE_BINARY) {
        return 0;
    }

    // Close handler handles removal of the websocket
    return static_cast<CivetWebSocketHandler *>(cwData)->handleData(me, connection, bits, data, len) ? 1 : 0;
}

void EnhancedCivetServer::wsCloseHandler(const mg_connection *connection, void *cwData)
{
    const struct mg_request_info *request_info = mg_get_request_info(connection);
    assert(request_info != NULL);
    EnhancedCivetServer *me = static_cast<EnhancedCivetServer *>(request_info->user_data);
    assert (me->wsExists(connection));
    static_cast<CivetWebSocketHandler *>(cwData)->handleClose(me, connection);
    me->wsRemove(connection);
}

}}


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

#ifndef ENHANCEDCIVETSERVER_H
#define ENHANCEDCIVETSERVER_H

#include <CivetServer.h>
#include <set>
#include <mutex>
#include <QtCore/QByteArray>

namespace harmony { namespace private_impl {

class EnhancedCivetServer;
class CivetWebSocketHandler
{
public:
    virtual ~CivetWebSocketHandler() {}
    CivetWebSocketHandler & operator=(const CivetWebSocketHandler &) = delete;
    CivetWebSocketHandler & operator=(CivetWebSocketHandler &&) = delete;
    virtual bool handleConnect(EnhancedCivetServer *server, const mg_connection *connection) = 0;
    virtual void handleReady(EnhancedCivetServer *server, mg_connection *connection) = 0;
    virtual bool handleData(EnhancedCivetServer *server, mg_connection *connection,
                            int bits, const char *data, size_t len) = 0;
    virtual void handleClose(EnhancedCivetServer *server, const mg_connection *connection) = 0;
};

class EnhancedCivetServer final : public CivetServer
{
public:
    EnhancedCivetServer(const char **options, const struct mg_callbacks *callbacks = 0);
    ~EnhancedCivetServer();
    static std::string getParameters(mg_connection *connection);
    static std::string getPostData(mg_connection *connection);
    void addWebSocketHandler(const std::string &uri, CivetWebSocketHandler *handler);
    // These methods do not perform any check on mg_connection
    bool wsWrite(mg_connection *connection, int opcode, const QByteArray &data);
private:
    bool wsExists(const mg_connection *connection) const;
    void wsRemove(const mg_connection *connection);
    static int wsConnectHandler(const mg_connection *connection, void *cwData);
    static void wsReadyHandler(mg_connection *connection, void *cwData);
    static int wsDataHandler(mg_connection *connection, int bits, char *data, size_t len, void *cwData);
    static void wsCloseHandler(const mg_connection *connection, void *cwData);
    std::set<const mg_connection *> m_webSockets;
    mutable std::mutex m_mutex;
};

}}

#endif // ENHANCEDCIVETSERVER_H

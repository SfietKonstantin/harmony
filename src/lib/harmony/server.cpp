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

#include "iserver.h"
#include <assert.h>
#include <sstream>
#include <CivetServer.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>
#include <QtCore/QDebug>
#include "iauthentificationservice.h"
#include "harmonyextension.h"
#include "iextensionmanager.h"

static const char *CERTIFICATE_DIR = "ssl";
static const char *CERTIFICATE = "harmony.pem";

namespace harmony
{

class EnhancedCivetServer: public CivetServer
{
public:
    EnhancedCivetServer(const char **options, const struct mg_callbacks *callbacks = 0);
    static std::string getParameters(mg_connection *connection);
    static std::string getPostData(mg_connection *connection);
};

EnhancedCivetServer::EnhancedCivetServer(const char **options, const mg_callbacks *callbacks)
    : CivetServer(options, callbacks)
{
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

class Server: public IServer
{
public:
    explicit Server(int port, IAuthentificationService &authentificationService,
                    const IExtensionManager &extensionManager);
    int port() const override;
    bool start() override;
    void stop() override;
private:
    class CertificateException : public std::runtime_error {
    public:
        CertificateException(const std::string &message) : std::runtime_error(message) {}
    };
    class PingHandler: public CivetHandler
    {
    public:
        bool handleGet(CivetServer *, struct mg_connection *connection) override;
    };
    class AuthentificationHandler: public CivetHandler
    {
    public:
        explicit AuthentificationHandler(Server &server);
        bool handlePost(CivetServer *, mg_connection *connection) override;
    private:
        Server &m_server;
    };
    class RequestHandler: public CivetHandler
    {
    public:
        explicit RequestHandler(Server &server, const Extension &extension, Endpoint endpoint);
        bool handleGet(CivetServer *, mg_connection *connection) override;
        bool handlePost(CivetServer *, mg_connection *connection) override;
        bool handleDelete(CivetServer *, mg_connection *connection) override;
        std::string endpoint() const;
    private:
        void handle(mg_connection *connection, bool hasData);
        Server &m_server;
        const Extension &m_extension;
        Endpoint m_endpoint;
    };
    static QByteArray getCertificateFilePath();
    static void writeAuthorizationRequired(mg_connection *connection);
    std::unique_ptr<CivetServer> m_server {nullptr};
    int m_port {0};
    IAuthentificationService &m_authentificationService;
    PingHandler m_pingHandler {};
    AuthentificationHandler m_authentificationHandler;
    std::vector<RequestHandler> m_handlers;
};

Server::Server(int port, IAuthentificationService &authentificationService,
               const IExtensionManager &extensionManager)
    : m_port{port}, m_authentificationService{authentificationService}
    , m_authentificationHandler{*this}
{
    for (const Extension *extension : extensionManager.extensions()) {
        for (const Endpoint &endpoint : extension->endpoints()) {
            m_handlers.push_back(RequestHandler(*this, *extension, endpoint));
        }
    }
}

int Server::port() const
{
    return m_port;
}

bool Server::start()
{
    bool ok = true;
    try {
        std::string port = std::to_string(m_port);
        port.append("s");

        const QByteArray &certificatePath = getCertificateFilePath();

#if HARMONY_DEBUG
        qDebug() << "Starting harmony server on port" << m_port;
        qDebug() << "Using certificate from" << certificatePath;
#endif

        const char * options[] = { "listening_ports", port.c_str(),
                                   "ssl_certificate", certificatePath.data(),
                                   nullptr };
        m_server.reset(new EnhancedCivetServer(options));
        m_server->addHandler("/ping", m_pingHandler);
        m_server->addHandler("/authenticate", m_authentificationHandler);
        for (RequestHandler &handler : m_handlers) {
            m_server->addHandler(handler.endpoint(), handler);
        }
    } catch (const CertificateException &e) {
#if HARMONY_DEBUG
        qWarning() << "Exception when creating certificate:" << e.what();
#else
        Q_UNUSED(e)
#endif
        ok = false;
    } catch (const CivetException &e) {
#if HARMONY_DEBUG
        qWarning() << "Exception when starting Server:" << e.what();
#else
        Q_UNUSED(e)
#endif
        ok = false;
    } catch (...) {
        ok = false;
    }

    return ok;
}

void Server::stop()
{
    m_server.reset();
}

QByteArray Server::getCertificateFilePath()
{
    QDir dir (QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    if (!dir.exists()) {
        if (!QDir::root().mkpath(dir.absolutePath())) {
            throw CertificateException("Failed to create application data directory");
        }
    }
    if (!dir.exists(CERTIFICATE_DIR)) {
        if (!dir.mkdir(CERTIFICATE_DIR)) {
            throw CertificateException("Failed to create the certificate directory");
        }
    }

    if (!dir.cd(CERTIFICATE_DIR)) {
        throw CertificateException("Failed to enter in the certificate directory");
    }

    if (!dir.exists(CERTIFICATE)) {
        QFile output (dir.absoluteFilePath(CERTIFICATE));
        if (!output.open(QIODevice::WriteOnly)) {
            throw CertificateException("Failed to open the certificate file");
        }

        QFile certificate (":/ssl/harmony.pem");
        if (!certificate.open(QIODevice::ReadOnly)) {
            throw CertificateException("Failed to open source certificate file");
        }
        output.write(certificate.readAll());
        output.close();
        certificate.close();
    }

    return dir.absoluteFilePath(CERTIFICATE).toLocal8Bit();
}

void Server::writeAuthorizationRequired(mg_connection *connection)
{
    std::stringstream ss;
    ss << "HTTP/1.1 401 Unauthorized\r\n"
       << "\r\n"
       << "Wrong authentification code";
    mg_printf(connection, ss.str().c_str());
}

IServer::Ptr IServer::create(int port, IAuthentificationService &authentificationService,
                             const IExtensionManager &extensionManager)
{
    return Ptr(new Server(port, authentificationService, extensionManager));
}

bool Server::PingHandler::handleGet(CivetServer *, mg_connection *connection)
{
    std::stringstream ss;
    ss << "HTTP/1.1 200 OK\r\n"
       << "\r\n"
       << "pong";
    mg_printf(connection, ss.str().c_str());
    return true;
}

Server::AuthentificationHandler::AuthentificationHandler(Server &server)
    : m_server{server}
{
}

bool Server::AuthentificationHandler::handlePost(CivetServer *, mg_connection *connection)
{
    std::string code;
    if (CivetServer::getParam(EnhancedCivetServer::getPostData(connection), "password", code)) {
        const JsonWebToken token = m_server.m_authentificationService.authenticate(code);
        if (!token.isNull()) {
            std::stringstream ss;
            ss << "HTTP/1.1 200 OK\r\n"
               << "\r\n"
               << m_server.m_authentificationService.hashJwt(token).data();
            mg_printf(connection, ss.str().c_str());
            return true;
        }
    }

    writeAuthorizationRequired(connection);
    return true;
}

Server::RequestHandler::RequestHandler(Server &server, const Extension &extension, Endpoint endpoint)
    : m_server(server), m_extension(extension), m_endpoint(std::move(endpoint))
{
}

bool Server::RequestHandler::handleGet(CivetServer *, mg_connection *connection)
{
    if (m_endpoint.type() != Endpoint::Type::Get) {
        return false;
    }
    handle(connection, false);
    return true;
}

bool Server::RequestHandler::handlePost(CivetServer *, mg_connection *connection)
{
    if (m_endpoint.type() != Endpoint::Type::Post) {
        return false;
    }
    handle(connection, true);
    return true;
}

bool Server::RequestHandler::handleDelete(CivetServer *, mg_connection *connection)
{
    if (m_endpoint.type() != Endpoint::Type::Delete) {
        return false;
    }
    handle(connection, false);
    return true;
}

std::string Server::RequestHandler::endpoint() const
{
    std::stringstream ss;
    ss << "/api/" << m_extension.id() << "/" << m_endpoint.name();
    return ss.str();
}

void Server::RequestHandler::handle(mg_connection *connection, bool hasData)
{
    const char *authorizationCharArray = CivetServer::getHeader(connection, "Authorization");
    std::string authorization = authorizationCharArray ? std::string(authorizationCharArray) : std::string();
    if (authorization.empty() || authorization.find("Bearer ") != 0) {
        writeAuthorizationRequired(connection);
        return;
    }

    authorization = authorization.substr(7);
    if (!m_server.m_authentificationService.isAuthorized(QByteArray::fromStdString(authorization))) {
        writeAuthorizationRequired(connection);
        return;
    }

    const std::string &params = EnhancedCivetServer::getParameters(connection);
    QUrlQuery query {QString::fromStdString(params)};

    QJsonDocument data;
    if (hasData) {
        const std::string &postData = EnhancedCivetServer::getPostData(connection);
        data = QJsonDocument::fromJson(QByteArray::fromStdString(postData));
    }

    Reply reply = m_extension.handleRequest(m_endpoint, query, data);
    std::stringstream ss;
    ss << "HTTP/1.1 ";
    switch (reply.status()) {
    case 200:
        ss << "200 OK";
        break;
    case 201:
        ss << "201 Created";
        break;
    case 202:
        ss << "202 Accepted";
        break;
    case 204:
        ss << "204 No Content";
        break;
    case 401:
        ss << "401 Unauthorized";
        break;
    case 403:
        ss << "403 Forbidden";
        break;
    case 404:
        ss << "404 Not Found";
        break;
    default:
        ss << "400 Bad Request";
        break;
    }

    ss << "\r\n\r\n";
    switch (reply.type()) {
    case Reply::Type::Json:
        ss << reply.value();
        break;
    default:
        break;
    }
    mg_printf(connection, ss.str().c_str());
}

}

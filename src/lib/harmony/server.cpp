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
#include <QtCore/QJsonArray>
#include <QtCore/QStandardPaths>
#include <QtCore/QLoggingCategory>
#include "private/enhancedcivetserver.h"
#include "iauthentificationservice.h"
#include "harmonyextension.h"
#include "iextensionmanager.h"

static const char *CERTIFICATE_DIR = "ssl";
static const char *CERTIFICATE = "harmony.pem";

namespace harmony {

using CivetWebSocketHandler = private_impl::CivetWebSocketHandler;
using EnhancedCivetServer = private_impl::EnhancedCivetServer;

class Server: public IServer
{
public:
    explicit Server(int port, IAuthentificationService &authentificationService,
                    IExtensionManager &extensionManager, const std::string &publicFolder);
    int port() const override;
    bool isRunning() const override;
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
        Endpoint m_endpoint {};
    };
    class ApiListHandler: public CivetHandler
    {
    public:
        explicit ApiListHandler(Server &server);
        bool handleGet(CivetServer *, mg_connection *connection) override;
    private:
        std::string m_cache {};
        Server &m_server;
    };
    class WebSocketHandler: public CivetWebSocketHandler
    {
    public:
        explicit WebSocketHandler(Server &server);
        bool handleConnect(EnhancedCivetServer *server, const mg_connection *connection);
        void handleReady(EnhancedCivetServer *server, mg_connection *connection);
        bool handleData(EnhancedCivetServer *server, mg_connection *connection, int bits,
                        const char *data, size_t len);
        void handleClose(EnhancedCivetServer *server, const mg_connection *connection);
    private:
        Server &m_server;
    };
    class WebSocketContainer: public IExtensionManager::ICallback
    {
    public:
        explicit WebSocketContainer(IExtensionManager &extensionManager, Server &server);
        ~WebSocketContainer();
        void addSocket(mg_connection *socket);
        void removeSocket(mg_connection *socket);
        void clear();
        void operator()(const QByteArray &data) const;
    private:
        IExtensionManager &m_extensionManager;
        Server &m_server;
        std::set<mg_connection *> m_sockets {};
        mutable std::mutex m_mutex {};
    };

    static QByteArray getCertificateFilePath();
    static void writeAuthorizationRequired(mg_connection *connection);
    bool checkAuthorization(mg_connection *connection);

    std::unique_ptr<EnhancedCivetServer> m_server {};

    const int m_port {0};
    const std::string m_publicFolder {};
    IAuthentificationService &m_authentificationService;
    const IExtensionManager &m_extensionManager;
    WebSocketContainer m_webSocketContainer;

    PingHandler m_pingHandler {};
    AuthentificationHandler m_authentificationHandler;
    std::vector<RequestHandler> m_handlers {};
    ApiListHandler m_apiListHandler;
    WebSocketHandler m_webSocketHandler;
};

Server::Server(int port, IAuthentificationService &authentificationService,
               IExtensionManager &extensionManager, const std::string &publicFolder)
    : m_port{port}, m_publicFolder{publicFolder}, m_authentificationService{authentificationService}
    , m_extensionManager{extensionManager}, m_webSocketContainer{extensionManager, *this}
    , m_authentificationHandler{*this}, m_apiListHandler{*this}, m_webSocketHandler{*this}
{
    for (const Extension *extension : m_extensionManager.extensions()) {
        for (const Endpoint &endpoint : extension->endpoints()) {
            m_handlers.push_back(RequestHandler(*this, *extension, endpoint));
        }
    }
}

int Server::port() const
{
    return m_port;
}

bool Server::isRunning() const
{
    return m_server != nullptr;
}

bool Server::start()
{
    bool ok = true;
    try {
        std::string port {std::to_string(m_port)};
        port.append("s");

        const QByteArray &certificatePath = getCertificateFilePath();

#ifdef HARMONY_DEBUG
        qCDebug(QLoggingCategory("server")) << "Starting harmony server on port" << m_port;
        qCDebug(QLoggingCategory("server")) << "Using certificate from" << certificatePath;
#endif

        const char *optionsNoPublic[] = {"listening_ports", port.c_str(),
                                         "ssl_certificate", certificatePath.data(),
                                         nullptr };
        const char *optionsPublic[] = {"listening_ports", port.c_str(),
                                       "ssl_certificate", certificatePath.data(),
                                       "document_root", m_publicFolder.c_str(),
                                       nullptr };
        if (m_publicFolder.empty()) {
            m_server.reset(new EnhancedCivetServer(optionsNoPublic));
        } else {
            m_server.reset(new EnhancedCivetServer(optionsPublic));
        }
        m_server->addHandler("/ping", m_pingHandler);
        m_server->addHandler("/authenticate", m_authentificationHandler);
        for (RequestHandler &handler : m_handlers) {
            m_server->addHandler(handler.endpoint(), handler);
        }
        m_server->addHandler("/api/list", m_apiListHandler);
        m_server->addWebSocketHandler("/api/ws", &m_webSocketHandler);
    } catch (const CertificateException &e) {
#ifdef HARMONY_DEBUG
        qWarning() << "Exception when creating certificate:" << e.what();
#else
        Q_UNUSED(e)
#endif
        ok = false;
    } catch (const CivetException &e) {
#ifdef HARMONY_DEBUG
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
    m_webSocketContainer.clear();
}

QByteArray Server::getCertificateFilePath()
{
    QDir dir {QStandardPaths::writableLocation(QStandardPaths::DataLocation)};
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
       << "Unauthorized";
    mg_printf(connection, ss.str().c_str());
}

bool Server::checkAuthorization(mg_connection *connection)
{
    const char *authorizationCharArray = CivetServer::getHeader(connection, "Authorization");
    std::string authorization = authorizationCharArray ? std::string(authorizationCharArray) : std::string();
    if (authorization.empty() || authorization.find("Bearer ") != 0) {
        writeAuthorizationRequired(connection);
        return false;
    }

    authorization = authorization.substr(7);
    if (!m_authentificationService.isAuthorized(QByteArray::fromStdString(authorization))) {
        writeAuthorizationRequired(connection);
        return false;
    }
    return true;
}

IServer::Ptr IServer::create(int port, IAuthentificationService &authentificationService,
                             IExtensionManager &extensionManager,
                             const std::string &publicFolder)
{
    return Ptr(new Server(port, authentificationService, extensionManager, publicFolder));
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
    std::string data = EnhancedCivetServer::getPostData(connection);
    QJsonDocument dataDocument = QJsonDocument::fromJson(QByteArray::fromStdString(data));
    if (dataDocument.isObject()) {
        const QJsonObject &object = dataDocument.object();
        const QString &code = object.value("password").toString();

        const JsonWebToken token = m_server.m_authentificationService.authenticate(code.toStdString());
        if (!token.isNull()) {
            std::stringstream ss;
            ss << "HTTP/1.1 200 OK\r\n"
               << "\r\n"
               << "{\"token\":\"" << m_server.m_authentificationService.hashJwt(token).data()
               << "\"}";
            mg_printf(connection, ss.str().c_str());
            return true;
        }
    }

    std::stringstream ss;
    ss << "HTTP/1.1 401 Unauthorized\r\n"
       << "\r\n"
       << "Wrong authentification code";
    mg_printf(connection, ss.str().c_str());
    return true;
}

Server::RequestHandler::RequestHandler(Server &server, const Extension &extension, Endpoint endpoint)
    : m_server{server}, m_extension{extension}, m_endpoint{std::move(endpoint)}
{
}

bool Server::RequestHandler::handleGet(CivetServer *, mg_connection *connection)
{
    assert(m_endpoint.type() == Endpoint::Type::Get);
    handle(connection, false);
    return true;
}

bool Server::RequestHandler::handlePost(CivetServer *, mg_connection *connection)
{
    assert(m_endpoint.type() == Endpoint::Type::Post);
    handle(connection, true);
    return true;
}

bool Server::RequestHandler::handleDelete(CivetServer *, mg_connection *connection)
{
    assert(m_endpoint.type() == Endpoint::Type::Delete);
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
    if (!m_server.checkAuthorization(connection)) {
        return;
    }

    const std::string &params = EnhancedCivetServer::getParameters(connection);
    QUrlQuery query {QString::fromStdString(params)};

    QJsonDocument data;
    if (hasData) {
        const std::string &postData = EnhancedCivetServer::getPostData(connection);
        data = QJsonDocument::fromJson(QByteArray::fromStdString(postData));
    }

    Reply reply {m_extension.handleRequest(m_endpoint, query, data)};
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

Server::ApiListHandler::ApiListHandler(Server &server)
    : m_server{server}
{
}

bool Server::ApiListHandler::handleGet(CivetServer *, mg_connection *connection)
{
    if (!m_server.checkAuthorization(connection)) {
        return true;
    }

    if (m_cache.empty()) {

        QJsonArray list;
        for (const Extension *extension : m_server.m_extensionManager.extensions()) {
            QJsonObject extensionObject {};
            extensionObject.insert("id", QString::fromStdString(extension->id()));
            extensionObject.insert("name", extension->name());
            extensionObject.insert("description", extension->description());
            QJsonArray endpoints {};
            for (const Endpoint &endpoint : extension->endpoints()) {
                QJsonObject endpointObject {};
                endpointObject.insert("name", QString::fromStdString(endpoint.name()));
                QString type {};
                switch (endpoint.type()) {
                case Endpoint::Type::Get:
                    type = "get";
                    break;
                case Endpoint::Type::Post:
                    type = "post";
                    break;
                case Endpoint::Type::Delete:
                    type = "delete";
                    break;
                default:
                    break;
                }
                endpointObject.insert("type", type);
                endpoints.append(endpointObject);
            }
            extensionObject.insert("endpoints", endpoints);
            list.append(extensionObject);
        }

        std::stringstream ss {};
        ss << "HTTP/1.1 200 OK\r\n"
           << "\r\n"
           << QJsonDocument(list).toJson(QJsonDocument::Compact).toStdString();
        m_cache = ss.str();
    }
    mg_printf(connection, m_cache.c_str());
    return true;
}

Server::WebSocketHandler::WebSocketHandler(Server &server)
    : m_server{server}
{
}

bool Server::WebSocketHandler::handleConnect(EnhancedCivetServer *server,
                                             const mg_connection *connection)
{
    Q_UNUSED(server);
    Q_UNUSED(connection);
    return true;
}

void Server::WebSocketHandler::handleReady(EnhancedCivetServer *server, mg_connection *connection)
{
    Q_UNUSED(server);
    Q_UNUSED(connection);
}

bool Server::WebSocketHandler::handleData(EnhancedCivetServer *server, mg_connection *connection,
                                          int bits, const char *data, size_t len)
{
    Q_UNUSED(server);
    Q_UNUSED(connection);
    Q_UNUSED(bits);
    QByteArray dataArray (data, len);
#ifdef HARMONY_DEBUG
    const struct mg_request_info *requestInfo = mg_get_request_info(connection);
    qCDebug(QLoggingCategory("ws")) << "Received from " << requestInfo->remote_addr << dataArray;
#endif
    bool ok = m_server.m_authentificationService.isAuthorized(data);
    if (ok) {
        m_server.m_webSocketContainer.addSocket(connection);
    }

    return ok;
}

void Server::WebSocketHandler::handleClose(EnhancedCivetServer *server,
                                           const mg_connection *connection)
{
    Q_UNUSED(server);
    m_server.m_webSocketContainer.removeSocket(const_cast<mg_connection *>(connection));
}

Server::WebSocketContainer::WebSocketContainer(IExtensionManager &extensionManager, Server &server)
    : m_extensionManager{extensionManager}, m_server{server}
{
    m_extensionManager.addCallback(*this);
}

Server::WebSocketContainer::~WebSocketContainer()
{
    m_extensionManager.removeCallback(*this);
}

void Server::WebSocketContainer::addSocket(mg_connection *socket)
{
    std::lock_guard<std::mutex> lock {m_mutex};
    m_sockets.insert(socket);
}

void Server::WebSocketContainer::removeSocket(mg_connection *socket)
{
    std::lock_guard<std::mutex> lock {m_mutex};
    m_sockets.erase(socket);
}

void Server::WebSocketContainer::clear()
{
    std::lock_guard<std::mutex> lock {m_mutex};
    m_sockets.clear();
}

void Server::WebSocketContainer::operator()(const QByteArray &data) const
{
    std::lock_guard<std::mutex> lock {m_mutex};
    for (mg_connection *socket : m_sockets) {
        m_server.m_server->wsWrite(socket, WEBSOCKET_OPCODE_TEXT, data);
    }
}

}

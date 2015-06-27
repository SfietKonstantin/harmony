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
#include <sstream>
#include <CivetServer.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStandardPaths>
#include <QtCore/QDebug>

static const char *CERTIFICATE_DIR = "ssl";
static const char *CERTIFICATE = "harmony.pem";

namespace harmony
{

class Server: public IServer
{
public:
    explicit Server(int port);
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
    static QByteArray getCertificateFilePath();
    std::unique_ptr<CivetServer> m_server {nullptr};
    int m_port {0};
    PingHandler m_pingHandler {};
};

Server::Server(int port)
    : m_port(port)
{
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
        CivetServer *server = new CivetServer(options);
        m_server.reset(server);
        server->addHandler("/ping", m_pingHandler);
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

IServer::Ptr IServer::create(int port)
{
    return Ptr(new Server(port));
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

}

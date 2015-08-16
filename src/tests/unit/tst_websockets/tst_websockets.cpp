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

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWebSockets/QWebSocket>
#include <private/enhancedcivetserver.h>
#include <jsonwebtoken.h>
#include <iserver.h>
#include <iauthentificationservice.h>
#include <iextensionmanager.h>
#include <thread>
#include <mutex>

Q_IMPORT_PLUGIN(HarmonyTestExtension)

using namespace harmony;
using namespace harmony::private_impl;

static const int PORT = 8080;

class Handler: public QObject, public CivetWebSocketHandler
{
    Q_OBJECT
public:
    enum State
    {
        Disconnected,
        Connected,
        Ready,
        DataAvailable
    };
    explicit Handler(QObject *parent = 0)
        : QObject(parent), m_state {Disconnected}, m_connection {nullptr}
    {
    }
    bool handleConnect(EnhancedCivetServer *server, const mg_connection *connection) override
    {
        Q_UNUSED(server);
        Q_UNUSED(connection);
        setState(Connected);
        return true;
    }
    void handleReady(EnhancedCivetServer *server, mg_connection *connection) override
    {
        Q_UNUSED(server);
        Q_UNUSED(connection);
        setState(Ready);
        setConnection(connection);
    }
    bool handleData(EnhancedCivetServer *server, mg_connection *connection,
                    int bits, const char *data, size_t len) override
    {
        Q_UNUSED(server);
        Q_UNUSED(connection);
        Q_UNUSED(bits);
        m_data = QByteArray(data, len);
        setState(DataAvailable);
        return (QByteArray(data) != "close");
    }
    void handleClose(EnhancedCivetServer *server, const mg_connection *connection) override
    {
        Q_UNUSED(server);
        Q_UNUSED(connection);
        setState(Disconnected);
        setConnection(nullptr);
    }
    State state() const
    {
        std::lock_guard<std::mutex> lock (m_mutex);
        return m_state;
    }
    mg_connection * connection() const
    {
        std::lock_guard<std::mutex> lock (m_mutex);
        return m_connection;
    }
    QByteArray data()
    {
        std::lock_guard<std::mutex> lock (m_mutex);
        QByteArray returned {m_data};
        m_data.clear();
        m_state = Ready;
        return returned;
    }
signals:
    void connectionChanged();
    void stateChanged(int state);
private:
    void setConnection(mg_connection *connection)
    {
        std::lock_guard<std::mutex> lock (m_mutex);
        if (m_connection != connection) {
            m_connection = connection;
            emit connectionChanged();
        }
    }

    void setState(State state)
    {
        std::lock_guard<std::mutex> lock (m_mutex);
        if (m_state != state) {
            m_state = state;
            emit stateChanged(state);
        }
    }
    State m_state {Disconnected};
    mg_connection *m_connection {nullptr};
    QByteArray m_data {};
    mutable std::mutex m_mutex {};
};

class TstWebSockets: public QObject
{
    Q_OBJECT
private:
    static void handleSslErrors(QNetworkReply &reply)
    {
        connect(&reply, &QNetworkReply::sslErrors, [&reply](const QList<QSslError> &sslErrors) {
            reply.ignoreSslErrors(sslErrors);
        });
    }
private Q_SLOTS:
    void initTestCase()
    {
        Q_INIT_RESOURCE(harmony);
    }
    void testWebSocketSimple()
    {
        // Server
        const char *options[] = {"listening_ports", "8080", nullptr };
        Handler handler;
        EnhancedCivetServer server (options);
        QSignalSpy handlerSpy (&handler, SIGNAL(stateChanged(int)));
        server.addWebSocketHandler("/test", &handler);

        // Client
        QWebSocket socket;
        socket.open(QUrl("ws://localhost:8080/test"));

        while (socket.state() != QAbstractSocket::ConnectedState) {
            QTest::qWait(100);
        }

        QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
        QCOMPARE(handler.state(), Handler::Ready);
        QCOMPARE(handlerSpy.count(), 2);
        QCOMPARE(handlerSpy.at(0).first().toInt(), static_cast<int>(Handler::Connected));
        QCOMPARE(handlerSpy.at(1).first().toInt(), static_cast<int>(Handler::Ready));
        handlerSpy.clear();

        socket.close();
        while (handler.state() != Handler::Disconnected) {
            QTest::qWait(100);
        }
        QCOMPARE(socket.state(), QAbstractSocket::UnconnectedState);
        QCOMPARE(handler.state(), Handler::Disconnected);
        QCOMPARE(handlerSpy.count(), 1);
        QCOMPARE(handlerSpy.at(0).first().toInt(), static_cast<int>(Handler::Disconnected));
    }

    void testWebSocketDialog()
    {
        // Server
        const char *options[] = {"listening_ports", "8080", nullptr };
        Handler handler;
        EnhancedCivetServer server (options);
        QSignalSpy handlerSpy (&handler, SIGNAL(stateChanged(int)));
        server.addWebSocketHandler("/test", &handler);

        // Client
        QWebSocket socket;
        socket.open(QUrl("ws://localhost:8080/test"));

        while (socket.state() != QAbstractSocket::ConnectedState) {
            QTest::qWait(100);
        }

        QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
        QCOMPARE(handler.state(), Handler::Ready);
        handlerSpy.clear();

        socket.sendTextMessage("Hello world");

        while (handler.state() != Handler::DataAvailable) {
            QTest::qWait(100);
        }
        QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
        QCOMPARE(handler.state(), Handler::DataAvailable);
        QCOMPARE(handlerSpy.count(), 1);
        QCOMPARE(handlerSpy.at(0).first().toInt(), static_cast<int>(Handler::DataAvailable));
        handlerSpy.clear();

        QCOMPARE(handler.data(), QByteArray("Hello world"));

        socket.close();
        while (handler.state() != Handler::Disconnected) {
            QTest::qWait(100);
        }
        QCOMPARE(socket.state(), QAbstractSocket::UnconnectedState);
        QCOMPARE(handler.state(), Handler::Disconnected);
        QCOMPARE(handlerSpy.count(), 1);
        QCOMPARE(handlerSpy.at(0).first().toInt(), static_cast<int>(Handler::Disconnected));
    }

    void testWebSocketDialog2()
    {
        // Server
        const char *options[] = {"listening_ports", "8080", nullptr };
        Handler handler;
        EnhancedCivetServer server (options);
        QSignalSpy handlerSpy (&handler, SIGNAL(stateChanged(int)));
        server.addWebSocketHandler("/test", &handler);

        // Client
        QWebSocket socket;
        socket.open(QUrl("ws://localhost:8080/test"));

        while (socket.state() != QAbstractSocket::ConnectedState) {
            QTest::qWait(100);
        }

        QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
        QCOMPARE(handler.state(), Handler::Ready);
        handlerSpy.clear();

        QString message;
        connect(&socket, &QWebSocket::textMessageReceived, [&message](const QString &received) {
            message = received;
        });

        server.wsWrite(handler.connection(), WEBSOCKET_OPCODE_TEXT, "Hello world");

        while (message.isEmpty()) {
            QTest::qWait(100);
        }
        QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
        QCOMPARE(handler.state(), Handler::Ready);
        QCOMPARE(handlerSpy.count(), 0);

        QCOMPARE(message, QString("Hello world"));

        socket.close();
        while (handler.state() != Handler::Disconnected) {
            QTest::qWait(100);
        }
        QCOMPARE(socket.state(), QAbstractSocket::UnconnectedState);
        QCOMPARE(handler.state(), Handler::Disconnected);
        QCOMPARE(handlerSpy.count(), 1);
        QCOMPARE(handlerSpy.at(0).first().toInt(), static_cast<int>(Handler::Disconnected));
    }

    void testWebSocketClose()
    {
        // Server
        const char *options[] = {"listening_ports", "8080", nullptr };
        Handler handler;
        EnhancedCivetServer server (options);
        QSignalSpy handlerSpy (&handler, SIGNAL(stateChanged(int)));
        server.addWebSocketHandler("/test", &handler);

        // Client
        QWebSocket socket;
        socket.open(QUrl("ws://localhost:8080/test"));

        while (socket.state() != QAbstractSocket::ConnectedState) {
            QTest::qWait(100);
        }

        QCOMPARE(socket.state(), QAbstractSocket::ConnectedState);
        QCOMPARE(handler.state(), Handler::Ready);
        handlerSpy.clear();

        socket.sendTextMessage("close");

        while (handler.state() != Handler::Disconnected) {
            QTest::qWait(100);
        }
        QCOMPARE(socket.state(), QAbstractSocket::UnconnectedState);
        QCOMPARE(handler.state(), Handler::Disconnected);
        QCOMPARE(handlerSpy.count(), 2);
        QCOMPARE(handlerSpy.at(0).first().toInt(), static_cast<int>(Handler::DataAvailable));
        QCOMPARE(handlerSpy.at(1).first().toInt(), static_cast<int>(Handler::Disconnected));
    }

    void testAuthentification()
    {
        QNetworkAccessManager network {};
        std::unique_ptr<QNetworkReply> reply {};

        IAuthentificationService::Ptr as = IAuthentificationService::create("test");
        IExtensionManager::Ptr em = IExtensionManager::create();
        IServer::Ptr server = IServer::create(*as, *em, PORT);
        QVERIFY(server->start());

        QNetworkRequest postRequest (QUrl("https://localhost:8080/authenticate"));
        postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QJsonObject object;
        object.insert("password", QString::fromStdString(as->password()));
        QByteArray query = QJsonDocument(object).toJson(QJsonDocument::Compact);
        reply.reset(network.post(postRequest, query));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::NoError);

        QJsonDocument result {QJsonDocument::fromJson(reply->readAll())};
        QByteArray jwt {result.object().value("token").toString().toLocal8Bit()};
        QByteArray token {"Bearer "};

        token.append(jwt);

        QWebSocket socket;
        socket.open(QUrl("wss://localhost:8080/api/ws"));
        socket.ignoreSslErrors();

        while (socket.state() != QAbstractSocket::ConnectedState) {
            QTest::qWait(100);
        }

        // Authenticate
        socket.sendBinaryMessage(jwt);

        QSignalSpy spy (&socket, SIGNAL(textMessageReceived(QString)));
        // Test websocket
        QNetworkRequest getRequest (QUrl("https://localhost:8080/api/test/test_ws"));
        getRequest.setRawHeader("Authorization", token);
        reply.reset(network.get(getRequest));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QCOMPARE(reply->readAll(), QByteArray("{}"));

        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).first().toString(), QString("Hello world"));
    }

    void testAuthentificationFailure()
    {
        IAuthentificationService::Ptr as = IAuthentificationService::create("test");
        IExtensionManager::Ptr em = IExtensionManager::create();
        IServer::Ptr server = IServer::create(*as, *em, PORT);
        QVERIFY(server->start());

        QWebSocket socket;
        socket.open(QUrl("wss://localhost:8080/api/ws"));
        socket.ignoreSslErrors();

        while (socket.state() != QAbstractSocket::ConnectedState) {
            QTest::qWait(100);
        }

        // Authenticate
        socket.sendBinaryMessage("test");

        while (socket.state() != QAbstractSocket::UnconnectedState) {
            QTest::qWait(100);
        }

        QCOMPARE(socket.error(), QAbstractSocket::RemoteHostClosedError);
    }
};


QTEST_MAIN(TstWebSockets)

#include "tst_websockets.moc"


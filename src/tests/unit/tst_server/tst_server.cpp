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

#include <algorithm>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <jsonwebtoken.h>
#include <iserver.h>
#include <iauthentificationservice.h>
#include <iextensionmanager.h>

Q_IMPORT_PLUGIN(HarmonyTestExtension)

using namespace harmony;

static const int PORT = 8080;

class TstServer: public QObject
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
    void testPing()
    {
        QNetworkAccessManager network {};
        std::unique_ptr<QNetworkReply> reply {};

        IAuthentificationService::Ptr as = IAuthentificationService::create("test");
        IExtensionManager::Ptr em = IExtensionManager::create();
        IServer::Ptr server = IServer::create(PORT, *as, *em);
        QCOMPARE(server->port(), PORT);
        QVERIFY(server->start());

        reply.reset(network.get(QNetworkRequest(QUrl("https://localhost:8080/ping"))));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QCOMPARE(reply->readAll(), QByteArray("pong"));

        server->stop();
        reply.reset(network.get(QNetworkRequest(QUrl("https://localhost:8080/ping"))));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }
        QCOMPARE(reply->error(), QNetworkReply::ConnectionRefusedError);
    }
    void testAuthentification()
    {
        QNetworkAccessManager network {};
        std::unique_ptr<QNetworkReply> reply {};

        IAuthentificationService::Ptr as = IAuthentificationService::create("test");
        IExtensionManager::Ptr em = IExtensionManager::create();
        IServer::Ptr server = IServer::create(PORT, *as, *em);
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
        QList<QByteArray> replySplitted = reply->readAll().split('.');
        QCOMPARE(replySplitted.count(), 3);
    }
    void testAuthentificationFailure()
    {
        QNetworkAccessManager network {};
        std::unique_ptr<QNetworkReply> reply {};

        IAuthentificationService::Ptr as = IAuthentificationService::create("test");
        IExtensionManager::Ptr em = IExtensionManager::create();
        IServer::Ptr server = IServer::create(PORT, *as, *em);
        QVERIFY(server->start());

        QNetworkRequest postRequest (QUrl("https://localhost:8080/authenticate"));
        postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QJsonObject object;
        object.insert("password", "test");
        QByteArray query = QJsonDocument(object).toJson(QJsonDocument::Compact);

        reply.reset(network.post(postRequest, query));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::AuthenticationRequiredError);
    }
    void testMultiRequest()
    {
        QSKIP("Takes time");
        QNetworkAccessManager network {};
        std::unique_ptr<QNetworkReply> reply1 {};
        std::unique_ptr<QNetworkReply> reply2 {};
        std::unique_ptr<QNetworkReply> reply3 {};

        bool changed {false};
        IAuthentificationService::PasswordChangedCallback_t callback = [&changed](const std::string &) {
            changed = true;
        };

        IAuthentificationService::Ptr as = IAuthentificationService::create("test", callback);
        IExtensionManager::Ptr em = IExtensionManager::create();
        IServer::Ptr server = IServer::create(PORT, *as, *em);
        QVERIFY(server->start());

        QNetworkRequest postRequest (QUrl("https://localhost:8080/authenticate"));
        postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QJsonObject object;
        object.insert("password", "test");
        QByteArray query = QJsonDocument(object).toJson(QJsonDocument::Compact);

        reply1.reset(network.post(postRequest, query));
        handleSslErrors(*reply1);
        reply2.reset(network.post(postRequest, query));
        handleSslErrors(*reply2);
        reply3.reset(network.post(postRequest, query));
        handleSslErrors(*reply3);

        while (!reply1->isFinished() || !reply2->isFinished() || !reply3->isFinished()) {
            QTest::qWait(100);
        }
        QVERIFY(changed);
    }
    void testMultiRequest2()
    {
        QSKIP("Takes time");
        QNetworkAccessManager network {};

        IAuthentificationService::Ptr as = IAuthentificationService::create("test");
        IExtensionManager::Ptr em = IExtensionManager::create();
        IServer::Ptr server = IServer::create(PORT, *as, *em);
        QVERIFY(server->start());

        QNetworkRequest postRequest (QUrl("https://localhost:8080/authenticate"));
        postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QJsonObject object;
        object.insert("password", QString::fromStdString(as->password()));
        QByteArray query = QJsonDocument(object).toJson(QJsonDocument::Compact);

        std::vector<std::unique_ptr<QNetworkReply>> replies;

        for (int i = 0; i < 10; ++i) {
            std::unique_ptr<QNetworkReply> reply {network.post(postRequest, query)};
            handleSslErrors(*reply);
            replies.push_back(std::move(reply));
        }

        while (!std::accumulate(std::begin(replies), std::end(replies), true,
                                [](bool finished, const std::unique_ptr<QNetworkReply> &reply) {
            return finished && reply->isFinished();
        })) {
            QTest::qWait(100);
        }

        for (const std::unique_ptr<QNetworkReply> &reply : replies) {
            QVERIFY(reply->isFinished());
        }

        int count = std::accumulate(std::begin(replies), std::end(replies), 0,
                                    [](int count, const std::unique_ptr<QNetworkReply> &reply) {
            return count + (reply->error() == QNetworkReply::NoError ? 1 : 0);
        });
        QCOMPARE(count, 1);
    }
    void testRequests()
    {
        QNetworkAccessManager network {};
        std::unique_ptr<QNetworkReply> reply {};

        IAuthentificationService::Ptr as = IAuthentificationService::create("test");
        IExtensionManager::Ptr em = IExtensionManager::create();
        IServer::Ptr server = IServer::create(PORT, *as, *em);
        QVERIFY(server->start());

        // Authorization
        QNetworkRequest authorizationRequest (QUrl("https://localhost:8080/authenticate"));
        authorizationRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QJsonObject object;
        object.insert("password", QString::fromStdString(as->password()));
        QByteArray query = QJsonDocument(object).toJson(QJsonDocument::Compact);
        reply.reset(network.post(authorizationRequest, query));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }
        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QByteArray token {"Bearer "};

        QJsonDocument result {QJsonDocument::fromJson(reply->readAll())};
        token.append(result.object().value("token").toString());

        // Get
        QNetworkRequest getRequest (QUrl("https://localhost:8080/api/test/test_get?string=test&int=3"));
        getRequest.setRawHeader("Authorization", token);
        reply.reset(network.get(getRequest));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QCOMPARE(reply->readAll(), QByteArray("{\"body\":{},\"name\":\"test_get\",\"params\":{\"int\":\"3\",\"string\":\"test\"},\"type\":\"get\"}"));

        // Post
        QJsonObject postData;
        postData.insert("string", "test2");
        postData.insert("int", 12345);
        postData.insert("bool", true);
        postData.insert("array", QJsonArray({"a", "b", "c"}));
        QJsonDocument document {postData};

        QNetworkRequest postRequest (QUrl("https://localhost:8080/api/test/test_post?string=test&int=3"));
        postRequest.setRawHeader("Authorization", token);
        postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        reply.reset(network.post(postRequest, document.toJson(QJsonDocument::Compact)));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QCOMPARE(reply->readAll(), QByteArray("{\"body\":{\"array\":[\"a\",\"b\",\"c\"],\"bool\":true,\"int\":12345,\"string\":\"test2\"},\"name\":\"test_post\",\"params\":{\"int\":\"3\",\"string\":\"test\"},\"type\":\"post\"}"));

        // Delete
        QNetworkRequest deleteRequest (QUrl("https://localhost:8080/api/test/test_delete?string=test&int=3"));
        deleteRequest.setRawHeader("Authorization", token);
        reply.reset(network.deleteResource(deleteRequest));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QCOMPARE(reply->readAll(), QByteArray("{\"body\":{},\"name\":\"test_delete\",\"params\":{\"int\":\"3\",\"string\":\"test\"},\"type\":\"delete\"}"));

        // API
        QNetworkRequest apiRequest (QUrl("https://localhost:8080/api/list"));
        apiRequest.setRawHeader("Authorization", token);
        reply.reset(network.get(apiRequest));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::NoError);
        document = QJsonDocument::fromJson(reply->readAll());
        QVERIFY(document.isArray());
        QCOMPARE(document.array().count(), 1);
        const QJsonObject &extension = document.array().first().toObject();
        QCOMPARE(extension.value("id").toString(), QString("test"));
    }
    void testUnauthorizedRequests()
    {
        QNetworkAccessManager network {};
        std::unique_ptr<QNetworkReply> reply {};

        IAuthentificationService::Ptr as = IAuthentificationService::create("test");
        IExtensionManager::Ptr em = IExtensionManager::create();
        IServer::Ptr server = IServer::create(PORT, *as, *em);
        QVERIFY(server->start());

        // Get
        reply.reset(network.get(QNetworkRequest(QUrl("https://localhost:8080/api/test/test_get"))));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::AuthenticationRequiredError);

        // Post
        QNetworkRequest postRequest (QUrl("https://localhost:8080/api/test/test_post"));
        postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        reply.reset(network.post(postRequest, "{}"));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::AuthenticationRequiredError);

        // Delete
        reply.reset(network.deleteResource(QNetworkRequest(QUrl("https://localhost:8080/api/test/test_delete"))));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::AuthenticationRequiredError);

        // API
        reply.reset(network.get(QNetworkRequest(QUrl("https://localhost:8080/api/list"))));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::AuthenticationRequiredError);
    }
    void testUnauthorizedRequests2()
    {
        QNetworkAccessManager network {};
        std::unique_ptr<QNetworkReply> reply {};

        IAuthentificationService::Ptr as = IAuthentificationService::create("test");
        IExtensionManager::Ptr em = IExtensionManager::create();
        IServer::Ptr server = IServer::create(PORT, *as, *em);
        QVERIFY(server->start());

        // Get
        QNetworkRequest getRequest (QUrl("https://localhost:8080/api/test/test_get"));
        getRequest.setRawHeader("Authorization", "Bearer test");
        reply.reset(network.get(getRequest));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::AuthenticationRequiredError);

        // Post
        QNetworkRequest postRequest (QUrl("https://localhost:8080/api/test/test_post"));
        postRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        postRequest.setRawHeader("Authorization", "Bearer test");
        reply.reset(network.post(postRequest, "{}"));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::AuthenticationRequiredError);

        // Delete
        QNetworkRequest deleteRequest (QUrl("https://localhost:8080/api/test/test_delete"));
        deleteRequest.setRawHeader("Authorization", "Bearer test");
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::AuthenticationRequiredError);

        // API
        reply.reset(network.get(QNetworkRequest(QUrl("https://localhost:8080/api/list"))));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::AuthenticationRequiredError);
    }
};


QTEST_MAIN(TstServer)

#include "tst_server.moc"


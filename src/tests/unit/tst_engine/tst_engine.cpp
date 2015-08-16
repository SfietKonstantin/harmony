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
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtDBus/QDBusInterface>
#include <iengine.h>
#include <iextensionmanager.h>
#include <dbus/idbusinterface.h>

using namespace harmony;

static const int PORT = 8080;

class TstEngine: public QObject
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

        // Remove path to certificate
        QDir dir {QStandardPaths::writableLocation(QStandardPaths::DataLocation)};
        if (dir.exists()) {
            dir.removeRecursively();
        }
    }
    void testEngine()
    {
        QNetworkAccessManager network {};
        std::unique_ptr<QNetworkReply> reply {};

        IEngine::Ptr engine {IEngine::create("test", PORT)};
        QVERIFY(engine->start());
        QVERIFY(!engine->start());
        QVERIFY(engine->isRunning());

        reply.reset(network.get(QNetworkRequest(QUrl("https://localhost:8080/ping"))));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QCOMPARE(reply->readAll(), QByteArray("pong"));

        QVERIFY(engine->stop());
        QVERIFY(!engine->stop());
        QVERIFY(!engine->isRunning());
        reply.reset(network.get(QNetworkRequest(QUrl("https://localhost:8080/ping"))));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }
        QCOMPARE(reply->error(), QNetworkReply::ConnectionRefusedError);
    }
    void testEngineDBus()
    {
        QNetworkAccessManager network {};
        std::unique_ptr<QNetworkReply> reply {};

        IEngine::Ptr engine {IEngine::create("test", PORT)};
        IDBusInterface::Ptr dbus {IDBusInterface::create(std::move(engine))};
        QDBusInterface interface {"harbour.harmony", "/", "harbour.harmony"};

        QDBusMessage result {interface.call("Start")};
        QCOMPARE(result.type(), QDBusMessage::ReplyMessage);
        QCOMPARE(result.arguments().count(), 1);
        QCOMPARE(result.arguments().at(0), QVariant(true));

        result = interface.call("Start");
        QCOMPARE(result.type(), QDBusMessage::ReplyMessage);
        QCOMPARE(result.arguments().count(), 1);
        QCOMPARE(result.arguments().at(0), QVariant(false));

        result = interface.call("IsRunning");
        QCOMPARE(result.type(), QDBusMessage::ReplyMessage);
        QCOMPARE(result.arguments().count(), 1);
        QCOMPARE(result.arguments().at(0), QVariant(true));


        reply.reset(network.get(QNetworkRequest(QUrl("https://localhost:8080/ping"))));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }

        QCOMPARE(reply->error(), QNetworkReply::NoError);
        QCOMPARE(reply->readAll(), QByteArray("pong"));

        result = interface.call("Stop");
        QCOMPARE(result.type(), QDBusMessage::ReplyMessage);
        QCOMPARE(result.arguments().count(), 1);
        QCOMPARE(result.arguments().at(0), QVariant(true));

        result = interface.call("Stop");
        QCOMPARE(result.type(), QDBusMessage::ReplyMessage);
        QCOMPARE(result.arguments().count(), 1);
        QCOMPARE(result.arguments().at(0), QVariant(false));

        result = interface.call("IsRunning");
        QCOMPARE(result.type(), QDBusMessage::ReplyMessage);
        QCOMPARE(result.arguments().count(), 1);
        QCOMPARE(result.arguments().at(0), QVariant(false));

        reply.reset(network.get(QNetworkRequest(QUrl("https://localhost:8080/ping"))));
        handleSslErrors(*reply);
        while (!reply->isFinished()) {
            QTest::qWait(100);
        }
        QCOMPARE(reply->error(), QNetworkReply::ConnectionRefusedError);
    }
    void testDoubleDBus()
    {
        IDBusInterface::Ptr emptyDBus {IDBusInterface::create(std::move(IEngine::Ptr()))};
        QVERIFY(!emptyDBus);

        IEngine::Ptr engine {IEngine::create("test", PORT)};
        IDBusInterface::Ptr dbus {IDBusInterface::create(std::move(engine))};
        QVERIFY(dbus != nullptr);

        IEngine::Ptr engine2 {IEngine::create("test", PORT)};
        IDBusInterface::Ptr dbus2 {IDBusInterface::create(std::move(engine2))};
        QVERIFY(!dbus2);
    }
};


QTEST_MAIN(TstEngine)

#include "tst_engine.moc"


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
#include <QtCore/QDateTime>
#include <iauthentificationservice.h>

using namespace harmony;

static const int PORT = 8080;

class TstAuthentificationService: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSimple()
    {
        IAuthentificationService::Ptr service = IAuthentificationService::create("test");
        QCOMPARE(static_cast<int>(service->password().size()), 8);
        const JsonWebToken &token = service->authenticate(service->password());
        QVERIFY(!token.isNull());
        QCOMPARE(token.payload().value("iat"), QJsonValue(QDateTime::currentMSecsSinceEpoch() / 1000));
    }
    void testPasswordChanged()
    {
        bool changed {false};
        IAuthentificationService::PasswordChangedCallback_t callback = [&changed](const std::string &) {
            changed = true;
        };
        IAuthentificationService::Ptr service = IAuthentificationService::create("test", callback);
        QVERIFY(service->authenticate("test").isNull());
        QVERIFY(!changed);
        service->authenticate("test");
        QVERIFY(!changed);
        service->authenticate("test");
        QVERIFY(changed);
    }
    void testPasswordChanged2()
    {
        bool changed {false};
        IAuthentificationService::PasswordChangedCallback_t callback = [&changed](const std::string &) {
            changed = true;
        };
        IAuthentificationService::Ptr service = IAuthentificationService::create("test", callback);
        QVERIFY(!service->authenticate(service->password()).isNull());
        QVERIFY(changed);
    }
    void testIsAuthorized()
    {
        IAuthentificationService::Ptr service = IAuthentificationService::create("test");
        const JsonWebToken &token = service->authenticate(service->password());
        QByteArray hashedJwt = service->hashJwt(token);
        QVERIFY(service->isAuthorized(hashedJwt));
        hashedJwt.append("1");
        QVERIFY(!service->isAuthorized(hashedJwt));

        QJsonObject payload;
        payload.insert("exp", 10);
        JsonWebToken expiredToken {payload};
        QVERIFY(!service->isAuthorized(service->hashJwt(expiredToken)));
    }
};


QTEST_MAIN(TstAuthentificationService)

#include "tst_authentificationservice.moc"


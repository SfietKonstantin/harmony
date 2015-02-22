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
#include <identificationservice.h>
#include <testserviceprovider.h>
#include <dbusintrospect.h>
#include <QtCore/QDebug>
#include "testproxy.h"

class TstIdentificationService : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDBusRegister();
    void testClientRegister();
    void testPassword();
};

void TstIdentificationService::testDBusRegister()
{
    TestServiceProvider::create();
    QVERIFY(!DBusIntrospect("org.sfietkonstantin.Harmony", "/identificationservice").isValid());
    {
        // Calling create the first time should register to DBus
        // and return a valid pointer
        IdentificationService::Ptr first = IdentificationService::create();
        QVERIFY(!first.isNull());

        QVERIFY(DBusIntrospect("org.sfietkonstantin.Harmony", "/identificationservice").isValid());

        // Calling create a second time should fail, as it is already
        // registered to DBus
        IdentificationService::Ptr  second = IdentificationService::create();
        QVERIFY(second.isNull());
    }
    QVERIFY(!DBusIntrospect("org.sfietkonstantin.Harmony", "/identificationservice").isValid());
}

void TstIdentificationService::testClientRegister()
{
    TestServiceProvider::create();
    IdentificationService::Ptr service = IdentificationService::create();
    TestProxy proxy ("org.sfietkonstantin.Harmony", "/identificationservice",
                     QDBusConnection::sessionBus());
    QSignalSpy spy (service.data(), SIGNAL(passwordChanged()));

    QString testClient ("testclient");

    // Register client
    QString password = service->password();
    QVERIFY(proxy.RegisterClient(testClient, password));

    // Password has changed after registering client
    QCOMPARE(spy.count(), 1);
    QVERIFY(service->password() != password);

    // Client is registered
    QCOMPARE(proxy.RegisteredClients().value().size(), 1);
    QCOMPARE(proxy.RegisteredClients().value().first(), testClient);

    // Unregister
    QVERIFY(proxy.UnregisterClient(testClient));
    QVERIFY(proxy.RegisteredClients().value().isEmpty());
}

void TstIdentificationService::testPassword()
{
    IdentificationService::Ptr service = IdentificationService::create();
    TestProxy proxy ("org.sfietkonstantin.Harmony", "/identificationservice",
                     QDBusConnection::sessionBus());
    QSignalSpy spy (service.data(), SIGNAL(passwordChanged()));

    QString testClient ("testclient");
    QString password = service->password();

    // Fail to register client
    QVERIFY(!proxy.RegisterClient(testClient, QString()));
    QVERIFY(proxy.RegisteredClients().value().isEmpty());
    QCOMPARE(spy.count(), 0);
    QCOMPARE(service->password(), password);

    // Second time
    QVERIFY(!proxy.RegisterClient(testClient, QString()));
    QVERIFY(proxy.RegisteredClients().value().isEmpty());
    QCOMPARE(spy.count(), 0);
    QCOMPARE(service->password(), password);

    // Third time
    QVERIFY(!proxy.RegisterClient(testClient, QString()));
    QVERIFY(proxy.RegisteredClients().value().isEmpty());

    // Password should have changed
    QCOMPARE(spy.count(), 1);
    QVERIFY(service->password() != password);
}


QTEST_MAIN(TstIdentificationService)

#include "tst_identificationservice.moc"


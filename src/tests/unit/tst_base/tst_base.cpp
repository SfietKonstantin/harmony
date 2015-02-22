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
#include <certificatemanager.h>
#include <nodeconfigurationservice.h>
#include <testserviceprovider.h>
#include <dbusintrospect.h>
#include <QtCore/QDebug>
#include "testproxy.h"

class TstBase: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testNodeConfigurationService();
    void testServiceProvider();
};

void TstBase::testNodeConfigurationService()
{
    TestServiceProvider::create();
    CertificateManager::Ptr certificateManager = HarmonyCertificateManager::create();
    QVERIFY(!DBusIntrospect("org.sfietkonstantin.Harmony", "/nodeconfigurationservice").isValid());
    {
        // Calling create the first time should register to DBus
        // and return a valid pointer
        NodeConfigurationService::Ptr first = NodeConfigurationService::create(certificateManager);
        QVERIFY(!first.isNull());

        TestProxy proxy ("org.sfietkonstantin.Harmony", "/nodeconfigurationservice",
                         QDBusConnection::sessionBus());
        QVERIFY(DBusIntrospect("org.sfietkonstantin.Harmony", "/nodeconfigurationservice").isValid());
        QString certificatePath = proxy.certificatePath();
        QCOMPARE(certificatePath, certificateManager->certificatePath());

        // Calling create a second time should fail, as it is already
        // registered to DBus
        NodeConfigurationService::Ptr  second = NodeConfigurationService::create(certificateManager);
        QVERIFY(second.isNull());
    }
    QVERIFY(!DBusIntrospect("org.sfietkonstantin.Harmony", "/nodeconfigurationservice").isValid());
}

void TstBase::testServiceProvider()
{
    CertificateManager::Ptr certificateManager = HarmonyCertificateManager::create();
    PluginManager::Ptr pluginManager = PluginManager::create();

    QVERIFY(!DBusIntrospect("org.sfietkonstantin.Harmony", "/").isValid());
    {
        // Calling create the first time should register to DBus
        // and return a valid pointer
        ServiceProvider::Ptr first = HarmonyServiceProvider::create(certificateManager,
                                                                    pluginManager);
        QVERIFY(!first.isNull());

        DBusIntrospect introspect("org.sfietkonstantin.Harmony", "/");
        QVERIFY(introspect.isValid());

        // Calling create a second time should fail, as it is already
        // registered to DBus
        ServiceProvider::Ptr  second = HarmonyServiceProvider::create(certificateManager,
                                                                      pluginManager);
        QVERIFY(second.isNull());

        QVERIFY(introspect.children().contains("identificationservice"));
        QVERIFY(introspect.children().contains("nodeconfigurationservice"));
        QVERIFY(introspect.children().contains("pluginservice"));
        QVERIFY(!first->identificationService().isNull());
        QVERIFY(!first->nodeConfigurationService().isNull());
        QVERIFY(!first->pluginService().isNull());
    }
    QVERIFY(!DBusIntrospect("org.sfietkonstantin.Harmony", "/").isValid());
}


QTEST_MAIN(TstBase)

#include "tst_base.moc"


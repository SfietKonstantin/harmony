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
#include <harmonyextension.h>
#include <testserviceprovider.h>
#include "testadaptor.h"
#include "testproxy.h"

Q_IMPORT_PLUGIN(HarmonyTestPlugin)

class DBusTestObject: public QObject
{
    Q_OBJECT
public:
    explicit DBusTestObject(QObject *parent = 0);
private Q_SLOTS:
    HarmonyEndpoint TestEndpoint(const HarmonyEndpoint &endpoint);
    HarmonyRequestResult TestRequestResult(const HarmonyRequestResult &requestResult);
};

DBusTestObject::DBusTestObject(QObject *parent)
    : QObject(parent)
{
    new TestAdaptor(this);
}

HarmonyEndpoint DBusTestObject::TestEndpoint(const HarmonyEndpoint &endpoint)
{
    return endpoint;
}

HarmonyRequestResult DBusTestObject::TestRequestResult(const HarmonyRequestResult &requestResult)
{
    return requestResult;
}

class TstHarmonyExtension : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testHarmonyEndpoint();
    void testHarmonyEndpointDBus();
    void testHarmonyRequestResult();
    void testHarmonyRequestResultDBus();
    void testPluginManager();
};

void TstHarmonyExtension::testHarmonyEndpoint()
{
    HarmonyEndpoint endpoint1;
    QVERIFY(endpoint1.isNull());
    QCOMPARE(endpoint1.type(), HarmonyEndpoint::Invalid);
    QVERIFY(endpoint1.name().isEmpty());

    endpoint1.setType(HarmonyEndpoint::Get);
    QVERIFY(endpoint1.isNull());
    QCOMPARE(endpoint1.type(), HarmonyEndpoint::Get);
    QVERIFY(endpoint1.name().isEmpty());

    endpoint1.setName("test");
    QVERIFY(!endpoint1.isNull());
    QCOMPARE(endpoint1.type(), HarmonyEndpoint::Get);
    QCOMPARE(endpoint1.name(), QString("test"));

    HarmonyEndpoint endpoint2 (endpoint1);
    QCOMPARE(endpoint2.isNull(), endpoint1.isNull());
    QCOMPARE(endpoint2.type(), endpoint1.type());
    QCOMPARE(endpoint2.name(), endpoint1.name());
    QVERIFY(endpoint2 == endpoint1);

    HarmonyEndpoint endpoint3;
    endpoint3 = endpoint1;
    QCOMPARE(endpoint3.isNull(), endpoint1.isNull());
    QCOMPARE(endpoint3.type(), endpoint1.type());
    QCOMPARE(endpoint3.name(), endpoint1.name());

    HarmonyEndpoint endpoint4 (HarmonyEndpoint::Post, "test2");
    QVERIFY(!endpoint4.isNull());
    QCOMPARE(endpoint4.type(), HarmonyEndpoint::Post);
    QCOMPARE(endpoint4.name(), QString("test2"));

    HarmonyEndpoint endpoint5 (endpoint1);
    endpoint5.setName("test2");
    QVERIFY(!endpoint5.isNull());
    QCOMPARE(endpoint5.type(), HarmonyEndpoint::Get);
    QCOMPARE(endpoint5.name(), QString("test2"));
    QVERIFY(!(endpoint5 == endpoint1));

}

void TstHarmonyExtension::testHarmonyEndpointDBus()
{
    TestServiceProvider::create();
    DBusTestObject dbusTestObject;
    QVERIFY(QDBusConnection::sessionBus().registerObject("/test", &dbusTestObject));

    HarmonyEndpoint endpoint (HarmonyEndpoint::Post, "test");
    TestProxy proxy ("org.sfietkonstantin.Harmony", "/test", QDBusConnection::sessionBus());

    HarmonyEndpoint result = proxy.TestEndpoint(endpoint);
    QVERIFY(result == endpoint);

    endpoint = HarmonyEndpoint(HarmonyEndpoint::Get, "test2");
    result = proxy.TestEndpoint(endpoint);
    QVERIFY(result == endpoint);

    endpoint = HarmonyEndpoint(HarmonyEndpoint::Delete, "test3");
    result = proxy.TestEndpoint(endpoint);
    QVERIFY(result == endpoint);

    QDBusConnection::sessionBus().unregisterObject("/test");
}

void TstHarmonyExtension::testHarmonyRequestResult()
{
    HarmonyRequestResult result1;
    QVERIFY(result1.isNull());
    QCOMPARE(result1.type(), HarmonyRequestResult::Invalid);
    QCOMPARE(result1.status(), 200);
    QVERIFY(result1.value().isEmpty());

    result1.setType(HarmonyRequestResult::Json);
    QVERIFY(result1.isNull());
    QCOMPARE(result1.type(), HarmonyRequestResult::Json);
    QCOMPARE(result1.status(), 200);
    QVERIFY(result1.value().isEmpty());

    result1.setStatus(404);
    QVERIFY(result1.isNull());
    QCOMPARE(result1.type(), HarmonyRequestResult::Json);
    QCOMPARE(result1.status(), 404);
    QVERIFY(result1.value().isEmpty());

    result1.setValue("[1]");
    QVERIFY(!result1.isNull());
    QCOMPARE(result1.type(), HarmonyRequestResult::Json);
    QCOMPARE(result1.status(), 404);
    QCOMPARE(result1.value(), QString("[1]"));

    HarmonyRequestResult result2 (result1);
    QCOMPARE(result2.isNull(), result1.isNull());
    QCOMPARE(result2.type(), result1.type());
    QCOMPARE(result2.status(), result1.status());
    QCOMPARE(result2.value(), result1.value());

    HarmonyRequestResult result3;
    result3 = result1;
    QCOMPARE(result3.isNull(), result1.isNull());
    QCOMPARE(result3.type(), result1.type());
    QCOMPARE(result3.status(), result1.status());
    QCOMPARE(result3.value(), result1.value());

    QJsonObject testJsonObjct;
    testJsonObjct.insert("test", 1);
    QJsonDocument testDocument (testJsonObjct);
    HarmonyRequestResult endpoint4 (testDocument);
    QVERIFY(!endpoint4.isNull());
    QCOMPARE(endpoint4.type(), HarmonyRequestResult::Json);
    QCOMPARE(endpoint4.status(), 200);
    QCOMPARE(endpoint4.value(), QString("{\"test\":1}"));

    HarmonyRequestResult endpoint5 ("/usr/bin/harmony");
    QVERIFY(!endpoint5.isNull());
    QCOMPARE(endpoint5.type(), HarmonyRequestResult::File);
    QCOMPARE(endpoint5.status(), 200);
    QCOMPARE(endpoint5.value(), QString("/usr/bin/harmony"));

    HarmonyRequestResult endpoint6 (404, testDocument);
    QVERIFY(!endpoint6.isNull());
    QCOMPARE(endpoint6.type(), HarmonyRequestResult::Json);
    QCOMPARE(endpoint6.status(), 404);
    QCOMPARE(endpoint6.value(), QString("{\"test\":1}"));

    HarmonyRequestResult endpoint7 (endpoint5);
    endpoint7.setValue("test2");
    QVERIFY(!endpoint7.isNull());
    QCOMPARE(endpoint7.type(), HarmonyRequestResult::File);
    QCOMPARE(endpoint7.status(), 200);
    QCOMPARE(endpoint7.value(), QString("test2"));
}

void TstHarmonyExtension::testHarmonyRequestResultDBus()
{
    TestServiceProvider::create();
    DBusTestObject dbusTestObject;
    QVERIFY(QDBusConnection::sessionBus().registerObject("/test", &dbusTestObject));

    HarmonyRequestResult endpoint ("/usr/bin/harmony");
    TestProxy proxy ("org.sfietkonstantin.Harmony", "/test", QDBusConnection::sessionBus());

    HarmonyRequestResult result = proxy.TestRequestResult(endpoint);
    QVERIFY(result == endpoint);

    QJsonObject testJsonObjct;
    testJsonObjct.insert("test", 1);
    QJsonDocument testDocument (testJsonObjct);
    endpoint = HarmonyRequestResult(404, testDocument);
    result = proxy.TestRequestResult(endpoint);
    QVERIFY(result == endpoint);

    QDBusConnection::sessionBus().unregisterObject("/test");
}

void TstHarmonyExtension::testPluginManager()
{
    PluginManager::Ptr pluginManager = PluginManager::create();
    QList<HarmonyExtension *> plugins = pluginManager->plugins();
    QCOMPARE(plugins.count(), 1);

    const HarmonyExtension *testPlugin = plugins.first();
    QCOMPARE(testPlugin->id(), QString("test"));
    QCOMPARE(testPlugin->name(), QString("Test"));
    QCOMPARE(testPlugin->description(), QString("The Harmony test plugin."));

    const QList<HarmonyEndpoint> &endpoints = testPlugin->endpoints();
    QCOMPARE(endpoints.count(), 3);
}


QTEST_MAIN(TstHarmonyExtension)

#include "tst_harmonyextension.moc"


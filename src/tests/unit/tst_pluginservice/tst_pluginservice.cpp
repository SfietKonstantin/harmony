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

#include <QtCore/QtPlugin>
#include <QtCore/QPluginLoader>
#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>
#include <pluginservice.h>
#include <testserviceprovider.h>
#include <dbusintrospect.h>
#include "testadaptor.h"
#include "testproxy.h"

Q_IMPORT_PLUGIN(HarmonyTestPlugin)


class DBusTestObject: public QObject
{
    Q_OBJECT
public:
    explicit DBusTestObject(QObject *parent = 0);
private Q_SLOTS:
    HarmonyPlugin TestPlugin(const HarmonyPlugin &plugin);
};

DBusTestObject::DBusTestObject(QObject *parent)
    : QObject(parent)
{
    new TestAdaptor(this);
}

HarmonyPlugin DBusTestObject::TestPlugin(const HarmonyPlugin &plugin)
{
    return plugin;
}

class TstPluginService : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDBusRegister();
    void testHarmonyPlugin();
    void testHarmonyPluginDBus();
    void testPluginService();
};

void TstPluginService::testDBusRegister()
{
    TestServiceProvider::create();
    PluginManager::Ptr pluginManager = PluginManager::create();
    QVERIFY(!DBusIntrospect("org.sfietkonstantin.Harmony", "/pluginservice").isValid());
    {
        // Calling create the first time should register to DBus
        // and return a valid pointer
        PluginService::Ptr first = PluginService::create(pluginManager);
        QVERIFY(!first.isNull());

        QVERIFY(DBusIntrospect("org.sfietkonstantin.Harmony", "/pluginservice").isValid());

        // Calling create a second time should fail, as it is already
        // registered to DBus
        PluginService::Ptr  second = PluginService::create(pluginManager);
        QVERIFY(second.isNull());
    }
    QVERIFY(!DBusIntrospect("org.sfietkonstantin.Harmony", "/pluginservice").isValid());
}

void TstPluginService::testHarmonyPlugin()
{
    HarmonyPlugin plugin1;
    QVERIFY(plugin1.isNull());
    QVERIFY(plugin1.id().isEmpty());
    QVERIFY(plugin1.name().isEmpty());
    QVERIFY(plugin1.description().isEmpty());

    plugin1.setId("test_id");
    QVERIFY(plugin1.isNull());
    QCOMPARE(plugin1.id(), QString("test_id"));
    QVERIFY(plugin1.name().isEmpty());
    QVERIFY(plugin1.description().isEmpty());

    plugin1.setName("test_name");
    QVERIFY(plugin1.isNull());
    QCOMPARE(plugin1.id(), QString("test_id"));
    QCOMPARE(plugin1.name(), QString("test_name"));
    QVERIFY(plugin1.description().isEmpty());

    plugin1.setDescription("test_description");
    QVERIFY(!plugin1.isNull());
    QCOMPARE(plugin1.id(), QString("test_id"));
    QCOMPARE(plugin1.name(), QString("test_name"));
    QCOMPARE(plugin1.description(), QString("test_description"));

    HarmonyPlugin plugin2 (plugin1);
    QCOMPARE(plugin2.isNull(), plugin1.isNull());
    QCOMPARE(plugin2.id(), plugin1.id());
    QCOMPARE(plugin2.name(), plugin1.name());
    QCOMPARE(plugin2.description(), plugin1.description());
    QVERIFY(plugin2 == plugin1);

    HarmonyPlugin plugin3;
    plugin3 = plugin1;
    QCOMPARE(plugin3.isNull(), plugin1.isNull());
    QCOMPARE(plugin3.id(), plugin1.id());
    QCOMPARE(plugin3.name(), plugin1.name());
    QCOMPARE(plugin3.description(), plugin1.description());

    HarmonyPlugin plugin4 ("test_id2", "test_name2", "test_description2");
    QVERIFY(!plugin4.isNull());
    QCOMPARE(plugin4.id(), QString("test_id2"));
    QCOMPARE(plugin4.name(), QString("test_name2"));
    QCOMPARE(plugin4.description(), QString("test_description2"));

    HarmonyPlugin plugin5 (plugin1);
    plugin5.setName("test_name2");
    QVERIFY(!plugin5.isNull());
    QCOMPARE(plugin5.id(), QString("test_id"));
    QCOMPARE(plugin5.name(), QString("test_name2"));
    QCOMPARE(plugin5.description(), QString("test_description"));
    QVERIFY(!(plugin5 == plugin1));
}

void TstPluginService::testHarmonyPluginDBus()
{
    TestServiceProvider::create();
    DBusTestObject dbusTestObject;
    QVERIFY(QDBusConnection::sessionBus().registerObject("/test", &dbusTestObject));

    HarmonyPlugin endpoint ("test_id", "test_name", "test_description");
    TestProxy proxy ("org.sfietkonstantin.Harmony", "/test", QDBusConnection::sessionBus());

    HarmonyPlugin result = proxy.TestPlugin(endpoint);
    QVERIFY(result == endpoint);

    QDBusConnection::sessionBus().unregisterObject("/test");
}

void TstPluginService::testPluginService()
{
    PluginManager::Ptr pluginManager = PluginManager::create();
    PluginService::Ptr pluginService = PluginService::create(pluginManager);

    QList<HarmonyPlugin> plugins = pluginService->plugins();
    QCOMPARE(plugins.count(), 1);
    const HarmonyPlugin &plugin = plugins.first();
    HarmonyPlugin expectedPlugin ("test", "Harmony test plugin", "The Harmony test plugin.");
    QCOMPARE(plugin, expectedPlugin);
}

QTEST_MAIN(TstPluginService)

#include "tst_pluginservice.moc"

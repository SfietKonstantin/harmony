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

Q_IMPORT_PLUGIN(HarmonyTestPlugin)

class TstPluginService : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPluginManager();
    void testDBusRegister();
};

void TstPluginService::testPluginManager()
{
    PluginManager::Ptr pluginManager = PluginManager::create();
    QList<HarmonyExtension *> plugins = pluginManager->plugins();
    QCOMPARE(plugins.count(), 1);

    const HarmonyExtension *testPlugin = plugins.first();
    QCOMPARE(testPlugin->id(), QString("test"));
    QCOMPARE(testPlugin->name(), QString("Harmony test plugin"));
    QCOMPARE(testPlugin->description(), QString("The Harmony test plugin."));

    const QList<HarmonyEndpoint> &endpoints = testPlugin->endpoints();
    QCOMPARE(endpoints.count(), 3);
}

void TstPluginService::testDBusRegister()
{
    PluginManager::Ptr pluginManager = PluginManager::create();

    // Calling create the first time should register to DBus
    // and return a valid pointer
    PluginService::Ptr first = PluginService::create(pluginManager);
    QVERIFY(!first.isNull());

    // Calling create a second time should fail, as it is already
    // registered to DBus
    PluginService::Ptr  second = PluginService::create(pluginManager);
    QVERIFY(second.isNull());
}

QTEST_MAIN(TstPluginService)

#include "tst_pluginservice.moc"


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
#include <iextensionmanager.h>

Q_IMPORT_PLUGIN(HarmonyTestPlugin)

using namespace harmony;

class TstHarmonyExtension : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testEndpoint();
    void testReply();
    void testExtensionManager();
};

void TstHarmonyExtension::testEndpoint()
{
    Endpoint endpoint1;
    QVERIFY(endpoint1.isNull());
    QCOMPARE(endpoint1.type(), Endpoint::Type::Invalid);
    QVERIFY(endpoint1.name().empty());

    Endpoint endpoint2 (Endpoint::Type::Get, "test");
    QVERIFY(!endpoint2.isNull());
    QCOMPARE(endpoint2.type(), Endpoint::Type::Get);
    QCOMPARE(endpoint2.name(), std::string("test"));

    Endpoint endpoint3 (Endpoint::Type::Post, "test2");

    QVERIFY(endpoint2 == endpoint2);
    QVERIFY(!(endpoint2 == endpoint3));
}

void TstHarmonyExtension::testReply()
{
    Reply reply1 {};
    QVERIFY(reply1.isNull());
    QCOMPARE(reply1.status(), 200);
    QCOMPARE(reply1.type(), Reply::Type::Invalid);
    QVERIFY(reply1.value().empty());

    QJsonDocument document = QJsonDocument::fromJson("[1]");
    Reply reply2 {document};
    QVERIFY(!reply2.isNull());
    QCOMPARE(reply2.status(), 200);
    QCOMPARE(reply2.type(), Reply::Type::Json);
    QCOMPARE(reply2.value(), std::string("[1]"));
    QCOMPARE(reply2.valueJson(), document);

    Reply reply3 {404, document};
    QCOMPARE(reply3.status(), 404);
    QCOMPARE(reply3.type(), Reply::Type::Json);
    QCOMPARE(reply3.value(), std::string("[1]"));
    QCOMPARE(reply3.valueJson(), document);

    QVERIFY(reply2 == reply2);
    QVERIFY(!(reply2 == reply3));
}

void TstHarmonyExtension::testExtensionManager()
{
    IExtensionManager::Ptr extensionManager = IExtensionManager::create();
    std::vector<Extension *> extensions = extensionManager->extensions();
    QCOMPARE(static_cast<int>(extensions.size()), 1);

    const Extension *testExtension = *extensions.begin();
    QCOMPARE(testExtension->id(), std::string("test"));
    QCOMPARE(testExtension->name(), QString("Test"));
    QCOMPARE(testExtension->description(), QString("The Harmony test plugin."));

    const std::vector<Endpoint> &endpoints = testExtension->endpoints();
    QCOMPARE(static_cast<int>(endpoints.size()), 3);
}


QTEST_MAIN(TstHarmonyExtension)

#include "tst_harmonyextension.moc"


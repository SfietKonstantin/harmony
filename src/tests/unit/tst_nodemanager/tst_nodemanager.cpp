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
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtDBus/QDBusConnection>
#include <QtTest/QSignalSpy>
#include <nodemanager.h>

Q_DECLARE_METATYPE(NodeManager::Status)

class TstNodeManager : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testDBusRegister();
    void testStartStopNode();
    void testStartErrorNode();
    void testStartTimeoutNode();
};

void TstNodeManager::initTestCase()
{
    QStandardPaths::enableTestMode(true);
    QDir dir (QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    if (dir.exists()) {
        QVERIFY(dir.removeRecursively());
    }
    QVERIFY(QDir::root().mkpath(dir.absolutePath()));

    QDir resDir (":/");
    foreach (const QString &fileName, resDir.entryList(QDir::Files)) {
        QFile file (resDir.absoluteFilePath(fileName));
        QVERIFY(file.copy(dir.absoluteFilePath(fileName)));
    }

    QProcess process;
    process.setWorkingDirectory(dir.absolutePath());
    process.start("npm install");
    QVERIFY(process.waitForFinished(-1));
}

void TstNodeManager::testDBusRegister()
{
    // Calling create the first time should register to DBus
    // and return a valid pointer
    NodeManager::Ptr first = NodeManager::create();
    QVERIFY(!first.isNull());

    // Calling create a second time should fail, as it is already
    // registered to DBus
    NodeManager::Ptr  second = NodeManager::create();
    QVERIFY(second.isNull());
}

class StatusWatcher: public QObject
{
    Q_OBJECT
public:
    explicit StatusWatcher(NodeManager *nodeManager)
        : QObject(nodeManager)
    {
        connect(nodeManager, &NodeManager::statusChanged, [this, nodeManager]() {
            NodeManager::Status status = nodeManager->status();
            emit statusChanged(status);
        });
    }
signals:
    void statusChanged(NodeManager::Status status);
};

void TstNodeManager::testStartStopNode()
{
    QStandardPaths::enableTestMode(true);
    QDir dir (QStandardPaths::writableLocation(QStandardPaths::DataLocation));

    QVERIFY(QDBusConnection::sessionBus().registerService("org.sfietkonstantin.Harmony"));
    NodeManager::Ptr instance = NodeManager::create();

    QSignalSpy spy (new StatusWatcher(instance.data()), SIGNAL(statusChanged(NodeManager::Status)));

    QCOMPARE(instance->status(), NodeManager::Stopped);
    QVERIFY(instance->startNode(dir.absoluteFilePath("success.js")));
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.first().value<NodeManager::Status>(), NodeManager::Starting);

    while (!spy.count()) {
        QTest::qWait(100);
    }

    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.first().value<NodeManager::Status>(), NodeManager::Ready);

    instance->stopNode();
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.first().value<NodeManager::Status>(), NodeManager::Stopping);

    while (!spy.count()) {
        QTest::qWait(100);
    }
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.first().value<NodeManager::Status>(), NodeManager::Stopped);

    QDBusConnection::sessionBus().unregisterService("org.sfietkonstantin.Harmony");
}

void TstNodeManager::testStartErrorNode()
{
    QStandardPaths::enableTestMode(true);
    QDir dir (QStandardPaths::writableLocation(QStandardPaths::DataLocation));

    QVERIFY(QDBusConnection::sessionBus().registerService("org.sfietkonstantin.Harmony"));
    NodeManager::Ptr instance = NodeManager::create();

    QSignalSpy spy (new StatusWatcher(instance.data()), SIGNAL(statusChanged(NodeManager::Status)));

    QCOMPARE(instance->status(), NodeManager::Stopped);
    QVERIFY(instance->startNode(dir.absoluteFilePath("error.js")));
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.first().value<NodeManager::Status>(), NodeManager::Starting);

    while (!spy.count()) {
        QTest::qWait(100);
    }

    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.first().value<NodeManager::Status>(), NodeManager::Stopped);

    QDBusConnection::sessionBus().unregisterService("org.sfietkonstantin.Harmony");
}

void TstNodeManager::testStartTimeoutNode()
{
    QStandardPaths::enableTestMode(true);
    QDir dir (QStandardPaths::writableLocation(QStandardPaths::DataLocation));

    QVERIFY(QDBusConnection::sessionBus().registerService("org.sfietkonstantin.Harmony"));
    NodeManager::Ptr instance = NodeManager::create();

    QSignalSpy spy (new StatusWatcher(instance.data()), SIGNAL(statusChanged(NodeManager::Status)));

    QCOMPARE(instance->status(), NodeManager::Stopped);
    QVERIFY(instance->startNode(dir.absoluteFilePath("timeout.js")));
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.first().value<NodeManager::Status>(), NodeManager::Starting);

    while (!spy.count()) {
        QTest::qWait(100);
    }

    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.first().value<NodeManager::Status>(), NodeManager::Stopped);

    QDBusConnection::sessionBus().unregisterService("org.sfietkonstantin.Harmony");
}

QTEST_MAIN(TstNodeManager)

#include "tst_nodemanager.moc"


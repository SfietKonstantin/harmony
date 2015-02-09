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
#include <QtCore/QDebug>

class TestCertificateManager: public AbstractCertificateManager
{
public:
    typedef QSharedPointer<TestCertificateManager> Ptr;
    static Ptr create(QObject *parent = 0);
private:
    explicit TestCertificateManager(QObject *parent = 0);
    QString m_certificatePath;
};

TestCertificateManager::TestCertificateManager(QObject *parent)
    : AbstractCertificateManager(parent)
{
    QString certificatePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir dir (certificatePath);
    if (dir.exists()) {
        QVERIFY(dir.removeRecursively());
    }

    qDebug() << "Using certificate path" << certificatePath;
    setCertificatePath(certificatePath);
}

TestCertificateManager::Ptr TestCertificateManager::create(QObject *parent)
{
    return Ptr(new TestCertificateManager(parent));
}

class TstCertificateManager : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void test();
};

void TstCertificateManager::initTestCase()
{
    Q_INIT_RESOURCE(harmony);
    QCoreApplication::instance()->setOrganizationName("harmony");
    QCoreApplication::instance()->setApplicationName("tst_certificatemanager");
    QStandardPaths::setTestModeEnabled(true);
}

void TstCertificateManager::test()
{
    TestCertificateManager::Ptr certificateManager = TestCertificateManager::create();
    QVERIFY(!certificateManager->hasCertificates());
    QVERIFY(certificateManager->createCertificates());
    QVERIFY(certificateManager->hasCertificates());
    QVERIFY(certificateManager->removeCertificates());
    QVERIFY(!certificateManager->hasCertificates());
}

QTEST_MAIN(TstCertificateManager)

#include "tst_certificatemanager.moc"


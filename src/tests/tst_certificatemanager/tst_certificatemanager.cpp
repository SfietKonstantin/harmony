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
#include <utils.h>

class TestCertificateManager: public CertificateManager
{
public:
    static QString testWritableLocation();
    static Ptr create();
private:
    explicit TestCertificateManager();
    QString m_certificatePath;
};

TestCertificateManager::TestCertificateManager()
    : CertificateManager()
{
    QString certificatePath = testWritableLocation();
    qDebug() << "Using certificate path" << certificatePath;
    setCertificatePath(certificatePath);
}

QString TestCertificateManager::testWritableLocation()
{
    return Utils::testPath();
}

CertificateManager::Ptr TestCertificateManager::create()
{
    return Ptr(new TestCertificateManager());
}

class TstCertificateManager : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void test();
    void testCertificateManager();
};

void TstCertificateManager::initTestCase()
{
    Q_INIT_RESOURCE(harmony);
    QCoreApplication::instance()->setOrganizationName("harmony");
    QCoreApplication::instance()->setApplicationName("tst_certificatemanager");
}

void TstCertificateManager::test()
{
    TestCertificateManager::Ptr certificateManager = TestCertificateManager::create();
    QCOMPARE(certificateManager->certificatePath(), TestCertificateManager::testWritableLocation());
    QVERIFY(!certificateManager->hasCertificates());
    QVERIFY(certificateManager->createCertificates());
    QVERIFY(certificateManager->hasCertificates());
    QVERIFY(certificateManager->removeCertificates());
    QVERIFY(!certificateManager->hasCertificates());
}

void TstCertificateManager::testCertificateManager()
{
    QVERIFY(HarmonyCertificateManager::create().data());
}

QTEST_MAIN(TstCertificateManager)

#include "tst_certificatemanager.moc"


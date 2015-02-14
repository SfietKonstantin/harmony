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

#include "certificatemanager.h"
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QTemporaryFile>

static const char *CERTIFICATE_DIR = "ssl";
static const char *SHELL_EXEC = "/bin/sh";
static const char *HARMONY_CA_CRT = "harmony-ca.crt";
static const char *HARMONY_KEY_PUB = "harmony-key.pub";
static const char *HARMONY_CRT = "harmony.crt";
static const char *HARMONY_KEY = "harmony.key";

class CertificateManagerPrivate
{
public:
    explicit CertificateManagerPrivate(CertificateManager *q);
    QString certificatePath;
protected:
    CertificateManager * const q_ptr;
private:
    Q_DECLARE_PUBLIC(CertificateManager)
};

CertificateManagerPrivate::CertificateManagerPrivate(CertificateManager *q)
    : q_ptr(q)
{
}

CertificateManager::CertificateManager()
    : QObject(), d_ptr(new CertificateManagerPrivate(this))
{
}

CertificateManager::~CertificateManager()
{
#ifdef HARMONY_DEBUG
    qDebug() << "Destroying CertificateManager";
#endif
}

QString CertificateManager::certificatePath() const
{
    Q_D(const CertificateManager);
    return d->certificatePath;
}

bool CertificateManager::hasCertificates() const
{
    Q_D(const CertificateManager);
    QDir dir (d->certificatePath);
    if (!dir.exists()) {
        return false;
    }

    if (!dir.exists(HARMONY_CA_CRT) || !dir.exists(HARMONY_KEY_PUB)
        || !dir.exists(HARMONY_CRT) || !dir.exists(HARMONY_KEY)) {
        return false;
    }

    return true;
}

bool CertificateManager::createCertificates() const
{
    Q_D(const CertificateManager);
    QTemporaryFile file;
    if (!file.open()) {
        return false;
    }

    QFile gencert (":/gencert.sh");
    if (!gencert.open(QIODevice::ReadOnly)) {
        return false;
    }

    file.write(gencert.readAll());
    file.close();
#ifdef HARMONY_DEBUG
    qDebug() << "Using" << file.fileName() << "to generate permissions";
#endif

    file.setPermissions(QFile::ReadUser | QFile::ExeUser);
    QStringList args;
    args.append(file.fileName());
    args.append(d->certificatePath);
    QProcess process;
    process.setProgram(SHELL_EXEC);
    process.setArguments(args);
    process.start();
    process.waitForFinished(-1);
#ifdef HARMONY_DEBUG
    qDebug() << "gencert exit with code:" << process.exitCode();
    qDebug() << "Standard output:" << process.readAllStandardOutput();
    qDebug() << "Standard error:" << process.readAllStandardError();
#endif
    return (process.exitCode() == 0);
}

bool CertificateManager::removeCertificates() const
{
    Q_D(const CertificateManager);
    QDir dir (d->certificatePath);
    return dir.removeRecursively();
}

void CertificateManager::setCertificatePath(const QString &certificatePath)
{
    Q_D(CertificateManager);
    d->certificatePath = certificatePath;
}

HarmonyCertificateManager::HarmonyCertificateManager()
    : CertificateManager()
{
    QDir dir (QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    setCertificatePath(dir.absoluteFilePath(CERTIFICATE_DIR));
}

CertificateManager::Ptr HarmonyCertificateManager::create()
{
    return Ptr(new HarmonyCertificateManager());
}


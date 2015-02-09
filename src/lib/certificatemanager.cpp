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
static const char *SHELL_EXEC = "/usr/bin/sh";
static const char *HARMONY_CA_CRT = "harmony-ca.crt";
static const char *HARMONY_KEY_PUB = "harmony-key.pub";
static const char *HARMONY_CRT = "harmony.crt";
static const char *HARMONY_KEY = "harmony.key";

class AbstractCertificateManagerPrivate
{
public:
    explicit AbstractCertificateManagerPrivate(AbstractCertificateManager *q);
    QString certificatePath;
protected:
    AbstractCertificateManager * const q_ptr;
private:
    Q_DECLARE_PUBLIC(AbstractCertificateManager)
};

AbstractCertificateManagerPrivate::AbstractCertificateManagerPrivate(AbstractCertificateManager *q)
    : q_ptr(q)
{
}

AbstractCertificateManager::AbstractCertificateManager(QObject *parent)
    : QObject(parent), d_ptr(new AbstractCertificateManagerPrivate(this))
{
}

AbstractCertificateManager::~AbstractCertificateManager()
{
#ifdef HARMONY_DEBUG
    qDebug() << "Destroying AbstractCertificateManager";
#endif
}

QString AbstractCertificateManager::certificatePath() const
{
    Q_D(const AbstractCertificateManager);
    return d->certificatePath;
}

bool AbstractCertificateManager::hasCertificates() const
{
    Q_D(const AbstractCertificateManager);
    QDir dir (d->certificatePath);
    if (!dir.exists()) {
        return false;
    }

    if (!dir.exists(HARMONY_CA_CRT)) {
        return false;
    }

    if (!dir.exists(HARMONY_KEY_PUB)) {
        return false;
    }

    if (!dir.exists(HARMONY_CRT)) {
        return false;
    }

    if (!dir.exists(HARMONY_KEY)) {
        return false;
    }

    return true;
}

bool AbstractCertificateManager::createCertificates() const
{
    Q_D(const AbstractCertificateManager);
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
    return (process.exitCode() == 0);
}

bool AbstractCertificateManager::removeCertificates() const
{
    Q_D(const AbstractCertificateManager);
    QDir dir (d->certificatePath);
    return dir.removeRecursively();
}

void AbstractCertificateManager::setCertificatePath(const QString &certificatePath)
{
    Q_D(AbstractCertificateManager);
    d->certificatePath = certificatePath;
}

class CertificateManagerPrivate: public AbstractCertificateManagerPrivate
{
public:
    explicit CertificateManagerPrivate(CertificateManager *q);
private:
    Q_DECLARE_PUBLIC(CertificateManager)
};

CertificateManager::CertificateManager(QObject *parent)
    : AbstractCertificateManager(parent)
{
    QDir dir (QStandardPaths::writableLocation(QStandardPaths::DataLocation));
    setCertificatePath(dir.absoluteFilePath(CERTIFICATE_DIR));
}

CertificateManager::Ptr CertificateManager::create(QObject *parent)
{
    return Ptr(new CertificateManager(parent));
}


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

#ifndef CERTIFICATEMANAGER_H
#define CERTIFICATEMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>

class AbstractCertificateManagerPrivate;
class AbstractCertificateManager: public QObject
{
    Q_OBJECT
public:
    virtual ~AbstractCertificateManager();
    virtual QString certificatePath() const = 0;
    bool hasCertificates() const;
    bool createCertificates() const;
    bool removeCertificates() const;
protected:
    explicit AbstractCertificateManager(QObject *parent = 0);
    QScopedPointer<AbstractCertificateManagerPrivate> d_ptr;
private:
    Q_DECLARE_PRIVATE(AbstractCertificateManager)
};

class CertificateManagerPrivate;
class CertificateManager : public AbstractCertificateManager
{
    Q_OBJECT
public:
    typedef QSharedPointer<CertificateManager> Ptr;
    static Ptr create(QObject *parent = 0);
    QString certificatePath() const override;
protected:
    explicit CertificateManager(QObject *parent = 0);
private:
    Q_DECLARE_PRIVATE(CertificateManager)
};

#endif // CERTIFICATEMANAGER_H

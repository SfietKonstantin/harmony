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

#include "identificationservice.h"
#include "identificationserviceadaptor.h"
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtDBus/QDBusConnection>

static const char *PATH = "/identificationservice";
static const int PASSWORD_LENGTH = 8;
static const int PASSWORD_ATTEMPTS_MAX = 3;

class IdentificationServicePrivate
{
public:
    explicit IdentificationServicePrivate(IdentificationService *q);
    void decrementPasswordAttempts();
    void generatePassword();
    bool registered;
    QString password;
    int passwordAttempts;
    QSet<QString> registeredClients;
protected:
    IdentificationService * const q_ptr;
private:
    Q_DECLARE_PUBLIC(IdentificationService)
};

IdentificationServicePrivate::IdentificationServicePrivate(IdentificationService *q)
    : registered(false), passwordAttempts(0), q_ptr(q)
{
    qsrand(QDateTime::currentMSecsSinceEpoch());
}

void IdentificationServicePrivate::decrementPasswordAttempts()
{
    --passwordAttempts;
    if (passwordAttempts <= 0) {
        generatePassword();
    }
}

void IdentificationServicePrivate::generatePassword()
{
    Q_Q(IdentificationService);
    QString newPassword;
    for (int i = 0; i < PASSWORD_LENGTH; ++i) {
        newPassword.append(QString::number(qrand() % 10));
    }

    if (password != newPassword) {
        password = newPassword;
        emit q->passwordChanged();
        emit q->PasswordChanged();
    }

    passwordAttempts = PASSWORD_ATTEMPTS_MAX;

#ifdef HARMONY_DEBUG
    qDebug() << "Current password:" << password;
#endif
}

IdentificationService::IdentificationService()
    : QObject(), d_ptr(new IdentificationServicePrivate(this))
{
}

IdentificationService::~IdentificationService()
{
    Q_D(IdentificationService);
#ifdef HARMONY_DEBUG
    qDebug() << "Destroying IdentificationService";
#endif
    if (d->registered) {
        QDBusConnection::sessionBus().unregisterObject(PATH);
#ifdef HARMONY_DEBUG
        qDebug() << "Unregistered DBus object" << PATH;
#endif
    }
}

IdentificationService::Ptr IdentificationService::create()
{
    Ptr instance = Ptr(new IdentificationService());
    new IdentificationServiceAdaptor(instance.data());

    if (!QDBusConnection::sessionBus().registerObject(PATH, instance.data())) {
        qWarning() << "Failed to register DBus object" << PATH;
        return Ptr();
    }

    instance->d_func()->registered = true;
    instance->d_func()->generatePassword();
    return instance;
}

QString IdentificationService::password() const
{
    Q_D(const IdentificationService);
    return d->password;
}

QStringList IdentificationService::registeredClients() const
{
    return RegisteredClients();
}

bool IdentificationService::registerClient(const QString &token, const QString &password)
{
    return RegisterClient(token, password);
}

bool IdentificationService::unregisterClient(const QString &token)
{
    return UnregisterClient(token);
}

QStringList IdentificationService::RegisteredClients() const
{
    Q_D(const IdentificationService);
    return d->registeredClients.toList();
}

bool IdentificationService::RegisterClient(const QString &token, const QString &password)
{
    Q_D(IdentificationService);
    if (d->password != password) {
        d->decrementPasswordAttempts();
        return false;
    }

    if (d->registeredClients.contains(token)) {
        return false;
    }

    d->registeredClients.insert(token);
    d->generatePassword();
    return true;
}

bool IdentificationService::UnregisterClient(const QString &token)
{
    Q_D(IdentificationService);
    if (!d->registeredClients.contains(token)) {
        return false;
    }
    d->registeredClients.remove(token);
    return true;
}

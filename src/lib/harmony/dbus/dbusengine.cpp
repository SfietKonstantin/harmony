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

#include "idbusengine.h"
#include "private/dbusengineimpl.h"
#include "private/adaptor.h"
#include <QtCore/QLoggingCategory>
#include <QtDBus/QDBusConnection>

namespace harmony
{

static const char *SERVICE = "harbour.harmony";
static const char *PATH = "/";

DBusEngineImpl::DBusEngineImpl(const QByteArray &key, IAuthentificationService::PasswordChangedCallback_t &&passwordChangedCallback, int port, const std::string &publicFolder)
    : QObject(), m_passwordChangedCallback(passwordChangedCallback)
{
    m_engine = IEngine::create(key, [this](const std::string &password) {
        emit PasswordChanged(QString::fromStdString(password));
        m_passwordChangedCallback(password);
    }, port, publicFolder);
}

DBusEngineImpl::~DBusEngineImpl()
{
    QDBusConnection connection {QDBusConnection::sessionBus()};
    connection.unregisterObject(PATH);
    connection.unregisterService(SERVICE);
}


DBusEngineImpl::Ptr DBusEngineImpl::create(const QByteArray &key, IAuthentificationService::PasswordChangedCallback_t &&passwordChangedCallback,
                                           int port, const std::string &publicFolder)
{
    QDBusConnection connection {QDBusConnection::sessionBus()};
    if (!connection.registerService(SERVICE)) {
        qCDebug(QLoggingCategory("dbus")) << "Failed to register DBus service";
        return Ptr();
    }

    DBusEngineImpl::Ptr instance {new DBusEngineImpl(key, std::move(passwordChangedCallback), port, publicFolder)};

    if (!connection.registerObject(PATH, instance.get())) {
        qCDebug(QLoggingCategory("dbus")) << "Failed to register DBus object";
        return Ptr();
    }

    new HarmonyAdaptor(instance.get());
    return instance;
}

bool DBusEngineImpl::isRunning() const
{
    return m_engine->isRunning();
}

bool DBusEngineImpl::start()
{
    return m_engine->start();
}

bool DBusEngineImpl::stop()
{
    return m_engine->stop();
}

std::string DBusEngineImpl::password() const
{
    return m_engine->password();
}

bool DBusEngineImpl::IsRunning() const
{
    return isRunning();
}

bool DBusEngineImpl::Start()
{
    return start();
}

bool DBusEngineImpl::Stop()
{
    return stop();
}

QString DBusEngineImpl::Password() const
{
    return QString::fromStdString(password());
}

IDBusEngine::Ptr IDBusEngine::create(const QByteArray &key,
                                     IAuthentificationService::PasswordChangedCallback_t &&passwordChangedCallback,
                                     int port, const std::string &publicFolder)
{
    return DBusEngineImpl::create(key, std::move(passwordChangedCallback), port, publicFolder);
}

}

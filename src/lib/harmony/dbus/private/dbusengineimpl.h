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

#ifndef DBUSINTERFACEIMPL_H
#define DBUSINTERFACEIMPL_H

#include <QtCore/QObject>
#include "idbusengine.h"

namespace harmony
{

class DBusEngineImpl: public QObject, public IDBusEngine
{
    Q_OBJECT
public:
    using Ptr = std::unique_ptr<DBusEngineImpl>;
    ~DBusEngineImpl();
    static Ptr create(const QByteArray &key, IAuthentificationService::PasswordChangedCallback_t &&passwordChangedCallback,
                      int port, const std::string &publicFolder);
    bool isRunning() const override;
    bool start() override;
    bool stop() override;
    std::string password() const;
private slots:
    bool IsRunning() const;
    bool Start();
    bool Stop();
    QString Password() const;
signals:
    void PasswordChanged(const QString &password);
private:
    explicit DBusEngineImpl(const QByteArray &key, IAuthentificationService::PasswordChangedCallback_t &&passwordChangedCallback,
                            int port, const std::string &publicFolder);
    IEngine::Ptr m_engine;
    const IAuthentificationService::PasswordChangedCallback_t m_passwordChangedCallback {};
};

}

#endif // DBUSINTERFACEIMPL_H

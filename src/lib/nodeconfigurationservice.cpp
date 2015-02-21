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

#include "nodeconfigurationservice.h"
#include "nodeconfigurationserviceadaptor.h"
#include <QtCore/QDebug>

static const char *PATH = "/nodeconfigurationservice";

class NodeConfigurationServicePrivate
{
public:
    explicit NodeConfigurationServicePrivate(NodeConfigurationService *q);
    bool registered;
    QString certificatePath;
protected:
    NodeConfigurationService * const q_ptr;
private:
    Q_DECLARE_PUBLIC(NodeConfigurationService)
};

NodeConfigurationServicePrivate::NodeConfigurationServicePrivate(NodeConfigurationService *q)
    : registered(false), q_ptr(q)
{
}

NodeConfigurationService::NodeConfigurationService()
    : QObject(), d_ptr(new NodeConfigurationServicePrivate(this))
{
}

NodeConfigurationService::~NodeConfigurationService()
{
    Q_D(NodeConfigurationService);
#ifdef HARMONY_DEBUG
    qDebug() << "Destroying NodeConfigurationService";
#endif
    if (d->registered) {
        QDBusConnection::sessionBus().unregisterObject(PATH);
#ifdef HARMONY_DEBUG
        qDebug() << "Unregistered DBus object" << PATH;
#endif
    }
}

QString NodeConfigurationService::certificatePath() const
{
    Q_D(const NodeConfigurationService);
    return d->certificatePath;
}

NodeConfigurationService::Ptr NodeConfigurationService::create(HarmonyCertificateManager::Ptr certificateManager)
{
    Ptr instance = Ptr(new NodeConfigurationService());
    instance->d_func()->certificatePath = certificateManager->certificatePath();
    new NodeConfigurationServiceAdaptor(instance.data());

    if (!QDBusConnection::sessionBus().registerObject(PATH, instance.data())) {
        qWarning() << "Failed to register DBus object" << PATH;
        return Ptr();
    }

    instance->d_func()->registered = true;
    return instance;
}

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

#include "serviceprovider.h"
#include <QtCore/QDebug>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusConnection>
#include "harmonyextension.h"
#include "pluginservice.h"

static const char *SERVICE = "org.sfietkonstantin.Harmony";
static const char *PATH = "/";

class ServiceProviderPrivate
{
public:
    explicit ServiceProviderPrivate(ServiceProvider *q);
    NodeConfigurationService::Ptr nodeConfigurationService;
    IdentificationService::Ptr identificationService;
    PluginService::Ptr pluginService;
protected:
    ServiceProvider * const q_ptr;
private:
    Q_DECLARE_PUBLIC(ServiceProvider)
};

ServiceProviderPrivate::ServiceProviderPrivate(ServiceProvider *q)
    : q_ptr(q)
{
}

ServiceProvider::ServiceProvider(NodeConfigurationService::Ptr nodeConfigurationService,
                                 IdentificationService::Ptr identificationService,
                                 PluginService::Ptr pluginService)
    : QObject(), d_ptr(new ServiceProviderPrivate(this))
{
    Q_D(ServiceProvider);
    d->nodeConfigurationService = nodeConfigurationService;
    d->identificationService = identificationService;
    d->pluginService = pluginService;
}

ServiceProvider::~ServiceProvider()
{
#ifdef HARMONY_DEBUG
    qDebug() << "Destroying ServiceProvider";
#endif
}

IdentificationService::Ptr ServiceProvider::identificationService() const
{
    Q_D(const ServiceProvider);
    return d->identificationService;
}

NodeConfigurationService::Ptr ServiceProvider::nodeConfigurationService() const
{
    Q_D(const ServiceProvider);
    return d->nodeConfigurationService;
}

PluginService::Ptr ServiceProvider::pluginService() const
{
    Q_D(const ServiceProvider);
    return d->pluginService;
}

bool ServiceProvider::registerToDBus(ServiceProvider::Ptr instance)
{
    qDBusRegisterMetaType<HarmonyEndpoint>();
    qDBusRegisterMetaType<HarmonyRequestResult>();
    qDBusRegisterMetaType<HarmonyPlugin>();

    if (!QDBusConnection::sessionBus().registerService(SERVICE)) {
        qWarning() << "Failed to register DBus service" << SERVICE;
        return false;
    }

    if (!QDBusConnection::sessionBus().registerObject(PATH, instance.data())) {
        qWarning() << "Failed to register DBus object" << PATH;
        return false;
    }

    return true;
}

HarmonyServiceProvider::HarmonyServiceProvider(HarmonyCertificateManager::Ptr certificateManager,
                                               PluginManager::Ptr pluginManager)
    : ServiceProvider(NodeConfigurationService::create(certificateManager),
                      IdentificationService::create(), PluginService::create(pluginManager))
{
}

ServiceProvider::Ptr HarmonyServiceProvider::create(HarmonyCertificateManager::Ptr certificateManager,
                                                    PluginManager::Ptr pluginManager)
{
    Ptr instance = Ptr(new HarmonyServiceProvider(certificateManager, pluginManager));
    if (!ServiceProvider::registerToDBus(instance)) {
        return Ptr();
    }

    return instance;
}

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

#include "pluginservice.h"
#include "pluginserviceadaptor.h"
#include "pluginadaptor.h"
#include <QtCore/QDebug>

static const char *PATH = "/pluginservice";
static const char *PLUGIN_PATH = "/plugin/%1";

class HarmonyPluginPrivate: public QSharedData
{
public:
    explicit HarmonyPluginPrivate();
    HarmonyPluginPrivate(const HarmonyPluginPrivate &other);
    ~HarmonyPluginPrivate();
    QString id;
    QString name;
    QString description;
    QList<HarmonyEndpoint> endpoints;
};

HarmonyPluginPrivate::HarmonyPluginPrivate()
    : QSharedData()
{
}

HarmonyPluginPrivate::HarmonyPluginPrivate(const HarmonyPluginPrivate &other)
    : QSharedData(other), id(other.id), name(other.name), description(other.description)
    , endpoints(other.endpoints)
{
}

HarmonyPluginPrivate::~HarmonyPluginPrivate()
{
}

HarmonyPlugin::HarmonyPlugin()
    : d_ptr(new HarmonyPluginPrivate())
{
}

HarmonyPlugin::HarmonyPlugin(const QString &id, const QString &name, const QString &description,
                             const QList<HarmonyEndpoint> &endpoints)
    : d_ptr(new HarmonyPluginPrivate())
{
    d_ptr->id = id;
    d_ptr->name = name;
    d_ptr->description = description;
    d_ptr->endpoints = endpoints;
}

HarmonyPlugin::HarmonyPlugin(const HarmonyPlugin &other)
    : d_ptr(other.d_ptr)
{
}

HarmonyPlugin & HarmonyPlugin::operator=(const HarmonyPlugin &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

HarmonyPlugin::~HarmonyPlugin()
{
}

bool HarmonyPlugin::operator==(const HarmonyPlugin &other) const
{
    return (d_ptr->id == other.d_ptr->id)
            && (d_ptr->name == other.d_ptr->name)
            && (d_ptr->description == other.d_ptr->description)
            && (d_ptr->endpoints == other.d_ptr->endpoints);
}

bool HarmonyPlugin::isNull() const
{
    return (d_ptr->id.isEmpty() || d_ptr->name.isEmpty() || d_ptr->description.isEmpty());
}

QString HarmonyPlugin::id() const
{
    return d_ptr->id;
}

void HarmonyPlugin::setId(const QString &id)
{
    d_ptr->id = id;
}

QString HarmonyPlugin::name() const
{
    return d_ptr->name;
}

void HarmonyPlugin::setName(const QString &name)
{
    d_ptr->name = name;
}

QString HarmonyPlugin::description() const
{
    return d_ptr->description;
}

void HarmonyPlugin::setDescription(const QString &description)
{
    d_ptr->description = description;
}

QList<HarmonyEndpoint> HarmonyPlugin::endpoints() const
{
    return d_ptr->endpoints;
}

void HarmonyPlugin::setEndpoints(const QList<HarmonyEndpoint> &endpoints)
{
    d_ptr->endpoints = endpoints;
}

QDBusArgument &operator<<(QDBusArgument &argument, const HarmonyPlugin &harmonyPlugin)
{
    argument.beginStructure();
    argument << harmonyPlugin.id();
    argument << harmonyPlugin.name();
    argument << harmonyPlugin.description();
    argument.beginArray(qMetaTypeId<HarmonyEndpoint>());
    for (const HarmonyEndpoint &endpoint : harmonyPlugin.endpoints()) {
        argument << endpoint;
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, HarmonyPlugin &harmonyPlugin)
{
    argument.beginStructure();
    QString id;
    argument >> id;
    harmonyPlugin.setId(id);
    QString name;
    argument >> name;
    harmonyPlugin.setName(name);
    QString description;
    argument >> description;
    harmonyPlugin.setDescription(description);
    argument.beginArray();
    QList<HarmonyEndpoint> endpoints;
    while (!argument.atEnd()) {
        HarmonyEndpoint endpoint;
        argument >> endpoint;
        endpoints.append(endpoint);
    }
    harmonyPlugin.setEndpoints(endpoints);
    argument.endArray();
    argument.endStructure();
    return argument;
}

class PluginServicePrivate
{
public:
    explicit PluginServicePrivate(PluginService *q);
    bool registered;
    QList<HarmonyPlugin> plugins;
    QList<PluginAdaptor *> adaptors;
protected:
    PluginService * const q_ptr;
private:
    Q_DECLARE_PUBLIC(PluginService)
};

PluginServicePrivate::PluginServicePrivate(PluginService *q)
    : registered(false), q_ptr(q)
{
}

PluginService::PluginService()
    : QObject(), d_ptr(new PluginServicePrivate(this))
{
}

PluginService::~PluginService()
{
    Q_D(PluginService);
#ifdef HARMONY_DEBUG
    qDebug() << "Destroying PluginService";
#endif
    if (d->registered) {
        QDBusConnection::sessionBus().unregisterObject(PATH);
#ifdef HARMONY_DEBUG
        qDebug() << "Unregistered DBus object" << PATH;
#endif
    }

    // Unregister plugins
    for (const HarmonyPlugin &plugin : d->plugins) {
        QString pluginObject = QString(PLUGIN_PATH).arg(plugin.id());
        QDBusConnection::sessionBus().unregisterObject(pluginObject);
#ifdef HARMONY_DEBUG
        qDebug() << "Unregistered DBus object" << pluginObject;
#endif
    }
    qDeleteAll(d->adaptors);
}

QList<HarmonyPlugin> PluginService::plugins() const
{
    Q_D(const PluginService);
    return d->plugins;
}

PluginService::Ptr PluginService::create(PluginManager::Ptr pluginManager)
{
    Ptr instance = Ptr(new PluginService());
    for (HarmonyExtension *plugin : pluginManager->plugins()) {
        HarmonyPlugin pluginDescription (plugin->id(), plugin->name(), plugin->description(),
                                         plugin->endpoints());
        instance->d_func()->plugins.append(pluginDescription);
    }
    new PluginServiceAdaptor(instance.data());

    if (!QDBusConnection::sessionBus().registerObject(PATH, instance.data())) {
        qWarning() << "Failed to register DBus object" << PATH;
        return Ptr();
    }

    // Register plugins to DBus
    for (HarmonyExtension *plugin : pluginManager->plugins()) {
        PluginAdaptor *adaptor = new PluginAdaptor(plugin);
        instance->d_func()->adaptors.append(adaptor);
        QString pluginPath = QString(PLUGIN_PATH).arg(plugin->id());
        if (!QDBusConnection::sessionBus().registerObject(pluginPath, plugin)) {
            qWarning() << "Failed to register DBus object" << pluginPath;
        }
    }

    instance->d_func()->registered = true;
    return instance;
}

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

#ifndef PLUGINSERVICE_H
#define PLUGINSERVICE_H

#include <QtCore/QObject>
#include <QtCore/QSharedPointer>
#include "pluginmanager.h"

class HarmonyPluginPrivate;
class HarmonyPlugin
{
public:
    explicit HarmonyPlugin();
    explicit HarmonyPlugin(const QString &id, const QString &name, const QString &description,
                           const QList<HarmonyEndpoint> &endpoints = QList<HarmonyEndpoint>());
    HarmonyPlugin(const HarmonyPlugin &other);
    HarmonyPlugin & operator=(const HarmonyPlugin &other);
    virtual ~HarmonyPlugin();
    bool operator==(const HarmonyPlugin &other) const;
    bool isNull() const;
    QString id() const;
    void setId(const QString &id);
    QString name() const;
    void setName(const QString &name);
    QString description() const;
    void setDescription(const QString &description);
    QList<HarmonyEndpoint> endpoints() const;
    void setEndpoints(const QList<HarmonyEndpoint> &endpoints);
private:
    QSharedDataPointer<HarmonyPluginPrivate> d_ptr;
};

Q_DECLARE_METATYPE(HarmonyPlugin)
QDBusArgument &operator<<(QDBusArgument &argument, const HarmonyPlugin &harmonyPlugin);
const QDBusArgument &operator>>(const QDBusArgument &argument, HarmonyPlugin &harmonyPlugin);

class PluginServicePrivate;
class PluginService : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<HarmonyPlugin> plugins READ plugins CONSTANT)
    Q_PROPERTY(QList<HarmonyPlugin> Plugins READ plugins CONSTANT)
public:
    typedef QSharedPointer<PluginService> Ptr;
    virtual ~PluginService();
    QList<HarmonyPlugin> plugins() const;
    static Ptr create(PluginManager::Ptr pluginManager);
protected:
    QScopedPointer<PluginServicePrivate> d_ptr;
private:
    explicit PluginService();
    Q_DECLARE_PRIVATE(PluginService)
};

#endif // PLUGINSERVICE_H

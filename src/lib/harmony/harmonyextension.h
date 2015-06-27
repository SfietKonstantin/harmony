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

#ifndef HARMONYEXTENSION_H
#define HARMONYEXTENSION_H

#include <QObject>
#include <QtCore/QtPlugin>
#include <QtCore/QJsonDocument>
#include <QtDBus/QDBusArgument>

class HarmonyEndpointPrivate;
class HarmonyEndpoint
{
public:
    enum Type
    {
        Invalid,
        Get,
        Post,
        Delete
    };
    explicit HarmonyEndpoint();
    explicit HarmonyEndpoint(Type type, const QString &name);
    HarmonyEndpoint(const HarmonyEndpoint &other);
    HarmonyEndpoint & operator=(const HarmonyEndpoint &other);
    virtual ~HarmonyEndpoint();
    bool operator==(const HarmonyEndpoint &other) const;
    bool isNull() const;
    Type type() const;
    void setType(Type type);
    QString name() const;
    void setName(const QString &name);
private:
    QSharedDataPointer<HarmonyEndpointPrivate> d_ptr;
};

Q_DECLARE_METATYPE(HarmonyEndpoint)
QDBusArgument &operator<<(QDBusArgument &argument, const HarmonyEndpoint &harmonyEndpoint);
const QDBusArgument &operator>>(const QDBusArgument &argument, HarmonyEndpoint &harmonyEndpoint);

class HarmonyRequestResultPrivate;
class HarmonyRequestResult
{
public:
    enum Type
    {
        Invalid,
        Json,
        File
    };
    explicit HarmonyRequestResult();
    explicit HarmonyRequestResult(const QJsonDocument &json);
    explicit HarmonyRequestResult(const QString &file);
    explicit HarmonyRequestResult(int status, const QJsonDocument &json);
    HarmonyRequestResult(const HarmonyRequestResult &other);
    HarmonyRequestResult & operator=(const HarmonyRequestResult &other);
    virtual ~HarmonyRequestResult();
    bool operator==(const HarmonyRequestResult &other) const;
    bool isNull() const;
    int status() const;
    void setStatus(int status);
    Type type() const;
    void setType(Type type);
    QString value() const;
    void setValue(const QString &value);
    QJsonDocument valueJson() const;
private:
    QSharedDataPointer<HarmonyRequestResultPrivate> d_ptr;
};

Q_DECLARE_METATYPE(HarmonyRequestResult)
QDBusArgument &operator<<(QDBusArgument &argument, const HarmonyRequestResult &harmonyRequestResult);
const QDBusArgument &operator>>(const QDBusArgument &argument, HarmonyRequestResult &harmonyEndpoint);

class IHarmonyExtension
{
public:
    virtual ~IHarmonyExtension() {}
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QList<HarmonyEndpoint> endpoints() const = 0;
protected:
    virtual HarmonyRequestResult request(const HarmonyEndpoint &endpoint,
                                         const QJsonDocument &params,
                                         const QJsonDocument &body) = 0;
};

Q_DECLARE_INTERFACE(IHarmonyExtension, "org.SfietKonstantin.harmony.IHarmonyExtension/1.0")

class HarmonyExtension : public QObject, public IHarmonyExtension
{
    Q_OBJECT
    Q_INTERFACES(IHarmonyExtension)
public:
    explicit HarmonyExtension(QObject *parent = 0);
    virtual ~HarmonyExtension();
private:
    Q_INVOKABLE HarmonyRequestResult Request(const HarmonyEndpoint &endpoint, const QString &params,
                                             const QString &body);
};

#endif // HARMONYEXTENSION_H

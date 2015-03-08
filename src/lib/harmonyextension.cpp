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

#include "harmonyextension.h"
#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>

class HarmonyEndpointPrivate: public QSharedData
{
public:
    explicit HarmonyEndpointPrivate();
    HarmonyEndpointPrivate(const HarmonyEndpointPrivate &other);
    ~HarmonyEndpointPrivate();
    HarmonyEndpoint::Type type;
    QString name;
};

HarmonyEndpointPrivate::HarmonyEndpointPrivate()
    : QSharedData(), type(HarmonyEndpoint::Invalid)
{
}

HarmonyEndpointPrivate::HarmonyEndpointPrivate(const HarmonyEndpointPrivate &other)
    : QSharedData(other), type(other.type), name(other.name)
{
}

HarmonyEndpointPrivate::~HarmonyEndpointPrivate()
{
}

HarmonyEndpoint::HarmonyEndpoint()
    : d_ptr(new HarmonyEndpointPrivate())
{
}

HarmonyEndpoint::HarmonyEndpoint(Type type, const QString &name)
    : d_ptr(new HarmonyEndpointPrivate())
{
    d_ptr->type = type;
    d_ptr->name = name;
}

HarmonyEndpoint::HarmonyEndpoint(const HarmonyEndpoint &other)
    : d_ptr(other.d_ptr)
{
}

HarmonyEndpoint & HarmonyEndpoint::operator=(const HarmonyEndpoint &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

HarmonyEndpoint::~HarmonyEndpoint()
{
}

bool HarmonyEndpoint::operator==(const HarmonyEndpoint &other) const
{
    return (d_ptr->type == other.d_ptr->type)
            && (d_ptr->name == other.d_ptr->name);
}

bool HarmonyEndpoint::isNull() const
{
    return (d_ptr->type == Invalid || d_ptr->name.isEmpty());
}

HarmonyEndpoint::Type HarmonyEndpoint::type() const
{
    return d_ptr->type;
}

void HarmonyEndpoint::setType(Type type)
{
    d_ptr->type = type;
}

QString HarmonyEndpoint::name() const
{
    return d_ptr->name;
}

void HarmonyEndpoint::setName(const QString &name)
{
    d_ptr->name = name;
}

static QString harmonyEndpointTypeToString(HarmonyEndpoint::Type type)
{
    switch (type) {
    case HarmonyEndpoint::Get:
        return "get";
        break;
    case HarmonyEndpoint::Post:
        return "post";
        break;
    case HarmonyEndpoint::Delete:
        return "delete";
        break;
    default:
        return QString();
    }
}

static HarmonyEndpoint::Type harmonyEndpointTypeFromString(const QString &type)
{
    if (type == "get") {
        return HarmonyEndpoint::Get;
    } else if (type == "post") {
        return HarmonyEndpoint::Post;
    } else if (type == "delete") {
        return HarmonyEndpoint::Delete;
    } else {
        return HarmonyEndpoint::Invalid;
    }
}

QDBusArgument &operator<<(QDBusArgument &argument, const HarmonyEndpoint &harmonyEndpoint)
{
    argument.beginStructure();
    argument << harmonyEndpointTypeToString(harmonyEndpoint.type());
    argument << harmonyEndpoint.name();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, HarmonyEndpoint &harmonyEndpoint)
{
    argument.beginStructure();
    QString type;
    argument >> type;
    harmonyEndpoint.setType(harmonyEndpointTypeFromString(type));
    QString name;
    argument >> name;
    harmonyEndpoint.setName(name);
    argument.endStructure();
    return argument;
}

class HarmonyRequestResultPrivate: public QSharedData
{
public:
    explicit HarmonyRequestResultPrivate();
    HarmonyRequestResultPrivate(const HarmonyRequestResultPrivate &other);
    ~HarmonyRequestResultPrivate();
    int status;
    HarmonyRequestResult::Type type;
    QString value;
};

HarmonyRequestResultPrivate::HarmonyRequestResultPrivate()
    : QSharedData(), status(200), type(HarmonyRequestResult::Invalid)
{
}

HarmonyRequestResultPrivate::HarmonyRequestResultPrivate(const HarmonyRequestResultPrivate &other)
    : QSharedData(other), status(other.status), type(other.type), value(other.value)
{
}

HarmonyRequestResultPrivate::~HarmonyRequestResultPrivate()
{
}

HarmonyRequestResult::HarmonyRequestResult()
    : d_ptr(new HarmonyRequestResultPrivate())
{
}

HarmonyRequestResult::HarmonyRequestResult(const QJsonDocument &json)
    : d_ptr(new HarmonyRequestResultPrivate())
{
    d_ptr->type = Json;
    d_ptr->value = QString(json.toJson(QJsonDocument::Compact));
}

HarmonyRequestResult::HarmonyRequestResult(const QString &file)
    : d_ptr(new HarmonyRequestResultPrivate())
{
    d_ptr->type = File;
    d_ptr->value = file;
}

HarmonyRequestResult::HarmonyRequestResult(int status, const QJsonDocument &json)
    : d_ptr(new HarmonyRequestResultPrivate())
{
    d_ptr->status = status;
    d_ptr->type = Json;
    d_ptr->value = QString(json.toJson(QJsonDocument::Compact));
}

HarmonyRequestResult::HarmonyRequestResult(const HarmonyRequestResult &other)
    : d_ptr(other.d_ptr)
{
}

HarmonyRequestResult & HarmonyRequestResult::operator=(const HarmonyRequestResult &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

HarmonyRequestResult::~HarmonyRequestResult()
{
}

bool HarmonyRequestResult::operator==(const HarmonyRequestResult &other) const
{
    return (d_ptr->status == other.d_ptr->status
            && d_ptr->type == other.d_ptr->type)
            && (d_ptr->value == other.d_ptr->value);
}

bool HarmonyRequestResult::isNull() const
{
    return (d_ptr->type == Invalid || d_ptr->value.isEmpty());
}

int HarmonyRequestResult::status() const
{
    return d_ptr->status;
}

void HarmonyRequestResult::setStatus(int status)
{
    d_ptr->status = status;
}

HarmonyRequestResult::Type HarmonyRequestResult::type() const
{
    return d_ptr->type;
}

void HarmonyRequestResult::setType(Type type)
{
    d_ptr->type = type;
}

QString HarmonyRequestResult::value() const
{
    return d_ptr->value;
}

QJsonDocument HarmonyRequestResult::valueJson() const
{
    return QJsonDocument::fromJson(d_ptr->value.toLocal8Bit());
}

void HarmonyRequestResult::setValue(const QString &value)
{
    d_ptr->value = value;
}

static QString harmonyRequestResultTypeToString(HarmonyRequestResult::Type type)
{
    switch (type) {
    case HarmonyRequestResult::Json:
        return "json";
        break;
    case HarmonyRequestResult::File:
        return "file";
        break;
    default:
        return QString();
    }
}

static HarmonyRequestResult::Type harmonyRequestResultTypeFromString(const QString &type)
{
    if (type == "json") {
        return HarmonyRequestResult::Json;
    } else if (type == "file") {
        return HarmonyRequestResult::File;
    } else {
        return HarmonyRequestResult::Invalid;
    }
}

QDBusArgument &operator<<(QDBusArgument &argument, const HarmonyRequestResult &harmonyRequestResult)
{
    argument.beginStructure();
    argument << harmonyRequestResult.status();
    argument << harmonyRequestResultTypeToString(harmonyRequestResult.type());
    argument << harmonyRequestResult.value();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, HarmonyRequestResult &harmonyRequestResult)
{
    argument.beginStructure();
    int status = 0;
    argument >> status;
    harmonyRequestResult.setStatus(status);
    QString type;
    argument >> type;
    harmonyRequestResult.setType(harmonyRequestResultTypeFromString(type));
    QString value;
    argument >> value;
    harmonyRequestResult.setValue(value);
    argument.endStructure();
    return argument;
}

HarmonyExtension::HarmonyExtension(QObject *parent)
    : QObject(parent)
{
}

HarmonyExtension::~HarmonyExtension()
{
}

HarmonyRequestResult HarmonyExtension::Request(const HarmonyEndpoint &endpoint, const QString &params,
                                               const QString &body)
{
    QJsonParseError error;
    QJsonDocument paramsDocument;
    if (!params.isEmpty()) {
        paramsDocument = QJsonDocument::fromJson(params.toLocal8Bit(), &error);
        if (error.error != QJsonParseError::NoError) {
    #ifdef HARMONY_DEBUG
            qWarning() << "When requesting" << endpoint.type() << endpoint.name()
                       << "failed to parse parameters:" << error.errorString();
    #endif
            return HarmonyRequestResult();
        }
    }

    QJsonDocument bodyDocument;
    if (!body.isEmpty()) {
        bodyDocument = QJsonDocument::fromJson(body.toLocal8Bit(), &error);
        if (error.error != QJsonParseError::NoError) {
    #ifdef HARMONY_DEBUG
            qWarning() << "When requesting" << endpoint.type() << endpoint.name()
                       << "failed to parse body:" << error.errorString();
    #endif
            return HarmonyRequestResult();
        }
    }

    return request(endpoint, paramsDocument, bodyDocument);
}

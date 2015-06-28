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

namespace harmony
{

Endpoint::Endpoint()
{
}

Endpoint::Endpoint(Type type, const std::string &name)
    : m_type{type}, m_name{name}
{
}

bool Endpoint::operator==(const Endpoint &other) const
{
    return m_type == other.m_type && m_name == other.m_name;
}

bool Endpoint::isNull() const
{
    return m_type == Type::Invalid || m_name.empty();
}

Endpoint::Type Endpoint::type() const
{
    return m_type;
}

std::string Endpoint::name() const
{
    return m_name;
}

Reply::Reply()
{
}

Reply::Reply(const QJsonDocument &json)
    : m_type{Type::Json}, m_value{json.toJson(QJsonDocument::Compact).toStdString()}
{
}

Reply::Reply(int status, const QJsonDocument &json)
    : m_status{status}, m_type{Type::Json}, m_value{json.toJson(QJsonDocument::Compact).toStdString()}
{
}

bool Reply::operator==(const Reply &other) const
{
    return m_status == other.m_status && m_type == other.m_type && m_value == other.m_value;
}

bool Reply::isNull() const
{
    return m_type == Type::Invalid || m_value.empty();
}

int Reply::status() const
{
    return m_status;
}

Reply::Type Reply::type() const
{
    return m_type;
}

std::string Reply::value() const
{
    return m_value;
}

QJsonDocument Reply::valueJson() const
{
    return QJsonDocument::fromJson(QByteArray::fromStdString(m_value));
}

Extension::Extension(QObject *parent)
    : QObject(parent)
{
}

}

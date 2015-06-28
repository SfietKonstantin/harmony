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
#include <QtCore/QUrlQuery>
#include <QtCore/QJsonDocument>

namespace harmony
{

class Endpoint
{
public:
    enum class Type
    {
        Invalid,
        Get,
        Post,
        Delete
    };
    explicit Endpoint();
    explicit Endpoint(Type type, const std::string &name);
    bool operator==(const Endpoint &other) const;
    bool isNull() const;
    Type type() const;
    std::string name() const;
private:
    Type m_type {Type::Invalid};
    std::string m_name {};
};

class Reply
{
public:
    enum class Type
    {
        Invalid,
        Json
    };
    explicit Reply();
    explicit Reply(const QJsonDocument &json);
    explicit Reply(int status, const QJsonDocument &json);
    bool operator==(const Reply &other) const;
    bool isNull() const;
    int status() const;
    Type type() const;
    std::string value() const;
    QJsonDocument valueJson() const;
private:
    int m_status {200};
    Type m_type {Type::Invalid};
    std::string m_value {};
};

class IExtension
{
public:
    virtual ~IExtension() {}
    virtual std::string id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual std::vector<Endpoint> endpoints() const = 0;
    virtual Reply handleRequest(const Endpoint &endpoint, const QUrlQuery &params,
                                const QJsonDocument &body) const = 0;
};

}

Q_DECLARE_INTERFACE(harmony::IExtension, "org.SfietKonstantin.harmony.IExtension/1.0")

namespace harmony
{

class Extension : public QObject, public IExtension
{
    Q_OBJECT
    Q_INTERFACES(harmony::IExtension)
public:
    explicit Extension(QObject *parent = 0);
};

}


#endif // HARMONYEXTENSION_H

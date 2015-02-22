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

#include "dbusintrospect.h"
#include <QtCore/QDebug>
#include <QtCore/QStack>
#include <QtCore/QXmlStreamReader>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

static const char *ELEMENT_NODE = "node";
static const char *ELEMENT_INTERFACE = "interface";
static const char *ELEMENT_METHOD = "method";
static const char *ELEMENT_SIGNAL = "signal";
static const char *ELEMENT_ARG = "arg";

static const char *ATTRIBUTE_NAME = "name";

class DBusIntrospectPrivate: public QSharedData
{
public:
    enum Element
    {
        Invalid,
        Node,
        Interface,
        Method,
        Signal,
        Arg,
    };
    explicit DBusIntrospectPrivate();
    DBusIntrospectPrivate(const DBusIntrospectPrivate &other);
    ~DBusIntrospectPrivate();
    static Element elementFromString(const QStringRef &element);
    QDBusError error;
    QStringList children;
};

DBusIntrospectPrivate::DBusIntrospectPrivate()
    : QSharedData()
{
}

DBusIntrospectPrivate::DBusIntrospectPrivate(const DBusIntrospectPrivate &other)
    : QSharedData(other), error(other.error)
{
}

DBusIntrospectPrivate::~DBusIntrospectPrivate()
{
}

DBusIntrospectPrivate::Element DBusIntrospectPrivate::elementFromString(const QStringRef &element)
{
    if (element == ELEMENT_NODE) {
        return Node;
    } else if (element == ELEMENT_INTERFACE) {
        return Interface;
    } else if (element == ELEMENT_METHOD) {
        return Method;
    } else if (element == ELEMENT_SIGNAL) {
        return Signal;
    } else if (element == ELEMENT_ARG) {
        return Arg;
    } else {
        return Invalid;
    }
}

DBusIntrospect::DBusIntrospect()
    : d_ptr(new DBusIntrospectPrivate())
{
}

DBusIntrospect::DBusIntrospect(const QString &service, const QString &path, const QDBusConnection &connection)
    : d_ptr(new DBusIntrospectPrivate())
{
    QDBusInterface testInterface (service, path, "org.freedesktop.DBus.Introspectable", connection);
    QDBusReply<QString> introspect = testInterface.call("Introspect");
    d_ptr->error = introspect.error();

    if (introspect.isValid()) {
        QXmlStreamReader reader (introspect.value());
        QStack<DBusIntrospectPrivate::Element> elements;
        while (!reader.atEnd()) {
            reader.readNext();
            if (reader.isStartElement()) {
                elements.push(DBusIntrospectPrivate::elementFromString(reader.name()));

                if (elements.size() > 1) {
                    switch (elements.top()) {
                    case DBusIntrospectPrivate::Node:
                    {
                        // Get children nodes
                        QXmlStreamAttributes attributes = reader.attributes();
                        QString name = attributes.value(ATTRIBUTE_NAME).toString();
                        d_ptr->children.append(name);
                        break;
                    }
                    default:
                        break;
                    }

                }
            } else if (reader.isEndElement()) {
                elements.pop();
            }
        }
    }
}

DBusIntrospect::DBusIntrospect(const DBusIntrospect &other)
    : d_ptr(other.d_ptr)
{
}

DBusIntrospect & DBusIntrospect::operator=(const DBusIntrospect &other)
{
    d_ptr = other.d_ptr;
    return *this;
}

DBusIntrospect::~DBusIntrospect()
{
}

bool DBusIntrospect::isValid() const
{
    return !d_ptr->error.isValid();
}

const QDBusError & DBusIntrospect::error() const
{
    return d_ptr->error;
}

QStringList DBusIntrospect::children() const
{
    return d_ptr->children;
}

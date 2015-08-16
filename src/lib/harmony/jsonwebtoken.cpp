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

#include "jsonwebtoken.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QList>
#include <QtCore/QMessageAuthenticationCode>

namespace harmony
{

static_assert(std::is_copy_constructible<JsonWebToken>::value, "JsonWebToken must be copy constructible");
static_assert(std::is_copy_assignable<JsonWebToken>::value, "JsonWebToken must be copy assignable");
static_assert(std::is_move_constructible<JsonWebToken>::value, "JsonWebToken must be move constructible");
static_assert(std::is_move_assignable<JsonWebToken>::value, "JsonWebToken must be move assignable");

static QByteArray makeSignature(const QByteArray &jwt, const QByteArray &key)
{
    return QMessageAuthenticationCode::hash(jwt, key, QCryptographicHash::Sha256).toHex();
}

JsonWebToken::JsonWebToken()
{
}

JsonWebToken::JsonWebToken(const QJsonObject &payload)
    : m_payload{payload}
{
}

bool JsonWebToken::operator==(const JsonWebToken &other) const
{
    return m_payload == other.m_payload;
}

bool JsonWebToken::isNull() const
{
    return m_payload.isEmpty();
}

QJsonObject JsonWebToken::payload() const
{
    return m_payload;
}

QByteArray JsonWebToken::toJwt(const QByteArray &key) const
{
    QByteArray jwt {};

    QJsonObject header {};
    header.insert("typ", "JWT");
    header.insert("alg", "HS256");
    QJsonDocument headerDocument {header};
    const QByteArray &headerArray = headerDocument.toJson(QJsonDocument::Compact).toBase64();

    QJsonDocument payloadDocument {m_payload};
    const QByteArray &payloadArray = payloadDocument.toJson(QJsonDocument::Compact).toBase64();

    jwt.append(headerArray);
    jwt.append(".");
    jwt.append(payloadArray);

    const QByteArray &signature = makeSignature(jwt, key);

    jwt.append(".");
    jwt.append(signature);
    return jwt;
}

JsonWebToken JsonWebToken::fromJwt(const QByteArray &jwt, const QByteArray &key)
{
    QList<QByteArray> splitted = jwt.split('.');
    if (splitted.count() != 3) {
        return JsonWebToken();
    }
    QByteArray message {};
    message.append(splitted.at(0));
    message.append(".");
    message.append(splitted.at(1));

    if (makeSignature(message, key) != splitted.at(2)) {
        return JsonWebToken();
    }

    QJsonDocument document = QJsonDocument::fromJson(QByteArray::fromBase64(splitted.at(1)));
    if (!document.isObject()) {
        return JsonWebToken();
    }

    return JsonWebToken(document.object());
}

}

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

#ifndef JSONWEBTOKEN_H
#define JSONWEBTOKEN_H

#include <QtCore/QJsonObject>

namespace harmony
{

/**
 * @brief A simple implementation of Auth0's jwt
 *
 * JSON Web Token (JWT) is a compact URL-safe means of
 * representing claims to be transferred between two parties.
 * The claims in a JWT are encoded as a JSON object that is
 * digitally signed using JSON Web Signature (JWS).
 *
 * For more information, visit http://jwt.io.
 *
 * This class only supports HMAC-SHA256 based JSON web tokens.
 */
class JsonWebToken final
{
public:
    explicit JsonWebToken();
    explicit JsonWebToken(const QJsonObject &payload);
    bool operator==(const JsonWebToken &other) const;
    bool isNull() const;
    QJsonObject payload() const;
    QByteArray toJwt(const QByteArray &key) const;
    static JsonWebToken fromJwt(const QByteArray &jwt, const QByteArray &key);
private:
    QJsonObject m_payload {};
};

}

#endif // JSONWEBTOKEN_H

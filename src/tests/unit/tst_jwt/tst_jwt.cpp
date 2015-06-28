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

#include <QtTest/QtTest>
#include <QtCore/QDebug>
#include <jsonwebtoken.h>

using namespace harmony;

static const int PORT = 8080;

class TstJwt: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSimple()
    {
        QJsonObject payload;
        payload.insert("sub", "1234567890");
        payload.insert("name", "John Doe");
        payload.insert("admin", true);

        JsonWebToken token {payload};
        const QByteArray &jwt = token.toJwt("secret");
        QCOMPARE(jwt, QByteArray("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhZG1pbiI6dHJ1ZSwibmFtZSI6IkpvaG4gRG9lIiwic3ViIjoiMTIzNDU2Nzg5MCJ9.78d2bf7e29ac096dd0fa678e5f273f767647b9b9760f8799908727ea59678829"));

        JsonWebToken verificationToken = JsonWebToken::fromJwt(jwt, "secret");
        QVERIFY(!verificationToken.isNull());
        QCOMPARE(token, verificationToken);
    }
    void testBad()
    {
        // Wrong token shape
        JsonWebToken verificationToken1 = JsonWebToken::fromJwt("aaaaaaaaaaaaaaaaaa", "secret");
        QVERIFY(verificationToken1.isNull());

        QJsonObject payload;
        payload.insert("sub", "1234567890");
        payload.insert("name", "John Doe");
        payload.insert("admin", true);

        // Wrong signature
        JsonWebToken token {payload};
        QByteArray jwt2 = token.toJwt("secret");
        jwt2.append("a");

        JsonWebToken verificationToken2 = JsonWebToken::fromJwt(jwt2, "secret");
        QVERIFY(verificationToken2.isNull());

        // Wrong payload
        QByteArray jwt3 = "aaaaaaaaaaaaaaaaaa.aaaaaaaaaaaaaaaaaa";
        const QByteArray &signature3 = QMessageAuthenticationCode::hash(jwt3, "secret",
                                                                        QCryptographicHash::Sha256).toHex();
        jwt3.append(".");
        jwt3.append(signature3);

        JsonWebToken verificationToken3 = JsonWebToken::fromJwt(jwt3, "secret");
        QVERIFY(verificationToken3.isNull());
    }
};


QTEST_MAIN(TstJwt)

#include "tst_jwt.moc"


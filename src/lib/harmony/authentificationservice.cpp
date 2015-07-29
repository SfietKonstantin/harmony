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

#include "iauthentificationservice.h"
#include <iomanip>
#include <sstream>
#include <chrono>
#include <QtCore/QLoggingCategory>
#include <QtCore/QUuid>
#include <atomic>
#include <mutex>

static const int PASSWORD_LENGTH = 8;
static constexpr int PASSWORD_MAX = std::pow(10, PASSWORD_LENGTH) - 1;
static const int PASSWORD_ATTEMPTS_MAX = 3;
static const int VALIDITY_DURATION = 86400;
namespace chrono = std::chrono;

namespace harmony
{

class AuthentificationService: public IAuthentificationService
{
public:
    explicit AuthentificationService(const QByteArray &key, PasswordChangedCallback_t passwordChangedCallback);
    std::string password() const override;
    JsonWebToken authenticate(const std::string &password) override;
    QByteArray hashJwt(const JsonWebToken &token) override;
    bool isAuthorized(const QByteArray &jwt) override;
private:
    bool comparePassword(const std::string &password) const;
    void setPassword(const std::string &password, bool init);
    void decrementPasswordAttempts();
    void generatePassword(bool init = false);
    std::atomic_int m_passwordAttempts {0};
    std::string m_password {};
    const QByteArray m_key {};
    const PasswordChangedCallback_t m_passwordChangedCallback {};
    mutable std::mutex m_mutex {};
};

AuthentificationService::AuthentificationService(const QByteArray &key,
                                                 PasswordChangedCallback_t passwordChangedCallback)
    : m_key{key}, m_passwordChangedCallback(std::move(passwordChangedCallback))
{
    generatePassword(true);
}

std::string AuthentificationService::password() const
{
    std::lock_guard<std::mutex> lock (m_mutex);
    return m_password;
}

JsonWebToken AuthentificationService::authenticate(const std::string &password)
{
    if (!comparePassword(password)) {
        decrementPasswordAttempts();
        return JsonWebToken();
    }

    generatePassword();

    chrono::time_point<std::chrono::system_clock> now = chrono::system_clock::now();
    int iat = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();

    QString jti = QUuid::createUuid().toString().remove("{").remove("}"); // TODO: save the JTI
    QJsonObject payload {};
    payload.insert("iat", iat);
    payload.insert("exp", iat + VALIDITY_DURATION);
    payload.insert("jti", jti);
    return JsonWebToken(payload);
}

QByteArray AuthentificationService::hashJwt(const JsonWebToken &token)
{
    return token.toJwt(m_key);
}

bool AuthentificationService::isAuthorized(const QByteArray &jwt)
{
    JsonWebToken token = JsonWebToken::fromJwt(jwt, m_key);
    if (token.isNull()) {
        return false;
    }

    chrono::time_point<std::chrono::system_clock> now = chrono::system_clock::now();
    int currentTime = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();

    int exp = token.payload().value("exp").toInt();
    if (currentTime >= exp) {
        return false;
    }

    return true;
}

bool AuthentificationService::comparePassword(const std::string &password) const
{
    std::lock_guard<std::mutex> lock (m_mutex);
    return m_password == password;
}

void AuthentificationService::setPassword(const std::string &password, bool init)
{
    std::lock_guard<std::mutex> lock (m_mutex);
    if (m_password != password) {
        m_password = password;
        if (m_passwordChangedCallback && !init) {
            m_passwordChangedCallback(password);
        }
    }
}

void AuthentificationService::decrementPasswordAttempts()
{
    --m_passwordAttempts;
    if (m_passwordAttempts <= 0) {
        generatePassword();
    }
}

void AuthentificationService::generatePassword(bool init)
{
    class Random {
    public:
        explicit Random()
        {
            std::random_device randomDevice;
            m_rng.seed(randomDevice());
        }
        int operator()() {
            std::uniform_int_distribution<int> range(0, PASSWORD_MAX);
            return range(m_rng);
        }
    private:
        std::mt19937 m_rng {};
    };

    static Random random;
    std::stringstream ss;
    ss << std::setw(PASSWORD_LENGTH) << std::setfill('0') << random();
    std::string password = ss.str();

    setPassword(password, init);
    m_passwordAttempts = PASSWORD_ATTEMPTS_MAX;

#ifdef HARMONY_DEBUG
    qCDebug(QLoggingCategory("auth-service")) << "Current password:" << QString::fromStdString(m_password);
#endif
}

IAuthentificationService::Ptr IAuthentificationService::create(const QByteArray &key,
                                                               PasswordChangedCallback_t &&passwordChangedCallback)
{
    return Ptr(new AuthentificationService(key, std::move(passwordChangedCallback)));
}

}

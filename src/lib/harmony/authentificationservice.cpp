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
#include <QtCore/QDebug>
#include <QtCore/QUuid>

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
    explicit AuthentificationService(PasswordChangedCallback_t passwordChangedCallback);
    std::string password() const override;
    JsonWebToken authenticate(const std::string &password) override;
private:
    void decrementPasswordAttempts();
    void generatePassword(bool init = false);
    int m_passwordAttempts {0};
    std::string m_password {};
    PasswordChangedCallback_t m_passwordChangedCallback {};
};

AuthentificationService::AuthentificationService(PasswordChangedCallback_t passwordChangedCallback)
    : m_passwordChangedCallback(std::move(passwordChangedCallback))
{
    generatePassword(true);
}

std::string AuthentificationService::password() const
{
    return m_password;
}

JsonWebToken AuthentificationService::authenticate(const std::string &password)
{
    if (password != m_password) {
        decrementPasswordAttempts();
        return JsonWebToken();
    }

    chrono::time_point<std::chrono::system_clock> now = chrono::system_clock::now();
    int iat = chrono::duration_cast<chrono::seconds>(now.time_since_epoch()).count();

    QString jti = QUuid::createUuid().toString(); // TODO: save the JTI
    QJsonObject payload {};
    payload.insert("iat", iat);
    payload.insert("exp", iat + VALIDITY_DURATION);
    payload.insert("jti", jti);
    return JsonWebToken(payload);
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
    if (m_password != password) {
        m_password = ss.str();
        if (m_passwordChangedCallback && !init) {
            m_passwordChangedCallback(password);
        }
    }
    m_passwordAttempts = PASSWORD_ATTEMPTS_MAX;

#ifdef HARMONY_DEBUG
    qDebug() << "Current password:" << QString::fromStdString(m_password);
#endif
}

IAuthentificationService::Ptr IAuthentificationService::create(PasswordChangedCallback_t passwordChangedCallback)
{
    return Ptr(new AuthentificationService(passwordChangedCallback));
}

}

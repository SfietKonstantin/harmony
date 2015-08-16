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

#include "iengine.h"
#include "iauthentificationservice.h"
#include "iextensionmanager.h"
#include "iserver.h"

namespace harmony
{

class Engine: public IEngine
{
public:
    explicit Engine(const QByteArray &key,
                    IAuthentificationService::PasswordChangedCallback_t &&passwordChangedCallback,
                    int port, const std::string &publicFolder);
    bool isRunning() const override;
    bool start() override;
    bool stop() override;
    std::string password() const override;
private:
    IAuthentificationService::Ptr m_authentificationService {};
    IExtensionManager::Ptr m_extensionManager {};
    IServer::Ptr m_server {};
};

Engine::Engine(const QByteArray &key,
               IAuthentificationService::PasswordChangedCallback_t &&passwordChangedCallback,
               int port, const std::string &publicFolder)
    : m_authentificationService{IAuthentificationService::create(key, std::move(passwordChangedCallback))}
    , m_extensionManager{IExtensionManager::create()}
    , m_server{IServer::create(*m_authentificationService, *m_extensionManager, port, publicFolder)}
{
}

bool Engine::isRunning() const
{
    return m_server->isRunning();
}

bool Engine::start()
{
    return m_server->start();
}

bool Engine::stop()
{
    if (!m_server->isRunning()) {
        return false;
    }
    m_server->stop();
    return true;
}

std::string Engine::password() const
{
    return m_authentificationService->password();
}

IEngine::Ptr IEngine::create(const QByteArray &key,
                             IAuthentificationService::PasswordChangedCallback_t &&passwordChangedCallback,
                             int port, const std::string &publicFolder)
{
    return Ptr(new Engine(key, std::move(passwordChangedCallback), port, publicFolder));
}

}

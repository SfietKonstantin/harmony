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

#include <harmonyextension.h>
#include <QtCore/QJsonObject>

using namespace harmony;

class HarmonyTestExtension: public Extension
{
    Q_OBJECT
#ifndef SONAR_RUN
    Q_PLUGIN_METADATA(IID "org.SfietKonstantin.harmony.IHarmonyExtension")
#endif
public:
    std::string id() const override
    {
        return "test";
    }

    QString name() const override
    {
        return "Test";
    }

    QString description() const override
    {
        return "The Harmony test plugin.";
    }

    std::vector<Endpoint> endpoints() const override
    {
        std::vector<Endpoint> endpoints;
        endpoints.push_back(Endpoint(Endpoint::Type::Get, "test_get"));
        endpoints.push_back(Endpoint(Endpoint::Type::Post, "test_post"));
        endpoints.push_back(Endpoint(Endpoint::Type::Delete, "test_delete"));
        endpoints.push_back(Endpoint(Endpoint::Type::Get, "test_ws"));
        return endpoints;
    }

    Reply handleWsRequest() const
    {
        emit broadcast("Hello world");
        return Reply(QJsonDocument(QJsonObject()));
    }

    Reply handleRequest(const Endpoint &endpoint, const QUrlQuery &params,
                        const QJsonDocument &body) const override
    {
        if (endpoint.name() == "test_ws" && endpoint.type() == Endpoint::Type::Get) {
            return handleWsRequest();
        }

        QJsonObject returned {};
        QString type {};
        switch (endpoint.type()) {
        case Endpoint::Type::Get:
            type = "get";
            break;
        case Endpoint::Type::Post:
            type = "post";
            break;
        case Endpoint::Type::Delete:
            type = "delete";
            break;
        default:
            break;
        }

        int status = 200;
        QJsonObject paramsObject;
        for (const QPair<QString, QString> &query : params.queryItems(QUrl::FullyDecoded)) {
            paramsObject.insert(query.first, query.second);
            if (query.first == "status") {
                status = query.second.toInt();
            }
        }

        returned.insert("type", type);
        returned.insert("name", QString::fromStdString(endpoint.name()));
        returned.insert("params", paramsObject);
        returned.insert("body", body.object());

        return Reply(status, QJsonDocument(returned));
    }
};

#include "plugin.moc"

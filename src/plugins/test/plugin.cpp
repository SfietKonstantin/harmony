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

class HarmonyTestPlugin: public HarmonyExtension
{
    Q_OBJECT
#ifndef SONAR_RUN
    Q_PLUGIN_METADATA(IID "org.SfietKonstantin.harmony.IHarmonyExtension")
#endif
public:
    QString id() const Q_DECL_OVERRIDE
    {
        return "test";
    }

    QString name() const Q_DECL_OVERRIDE
    {
        return "Harmony test plugin";
    }

    QString description() const Q_DECL_OVERRIDE
    {
        return "The Harmony test plugin.";
    }

    QList<HarmonyEndpoint> endpoints() const Q_DECL_OVERRIDE
    {
        QList<HarmonyEndpoint> endpoints;
        HarmonyEndpoint testGet (HarmonyEndpoint::Get, "test_get");
        HarmonyEndpoint testPost (HarmonyEndpoint::Post, "test_post");
        HarmonyEndpoint testDelete (HarmonyEndpoint::Delete, "test_delete");
        endpoints.append(testGet);
        endpoints.append(testPost);
        endpoints.append(testDelete);
        return endpoints;
    }

    HarmonyRequestResult request(const QString &method, const QJsonDocument &request)
    {
        QJsonObject returned;
        returned.insert("method", method);
        returned.insert("request", request.object());

        return HarmonyRequestResult(QJsonDocument(returned));
    }
};

#include "plugin.moc"

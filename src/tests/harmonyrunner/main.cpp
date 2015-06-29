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

#include <QtCore/QCoreApplication>
#include <QtCore/QtPlugin>
#include <QtCore/QLoggingCategory>
#include <QtCore/QDir>
#include <QtCore/QUuid>
#include <iauthentificationservice.h>
#include <iextensionmanager.h>
#include <iserver.h>
#include <iostream>
#include <signal.h>

using namespace harmony;

Q_IMPORT_PLUGIN(HarmonyTestPlugin)

void help()
{
    std::cout << "Usage:" << std::endl
              << "  harmonyrunner path/to/public" << std::endl
              << std::endl
              << "Run the Harmony daemon in commandline." << std::endl;
}

void daemonize()
{
    signal(SIGTERM, [](int signal){
        switch (signal) {
        case SIGTERM:
            QCoreApplication::exit();
            break;
        }
    });
}

int main(int argc, char **argv)
{
    QCoreApplication app (argc, argv);
    Q_INIT_RESOURCE(harmony);
    daemonize();
    const QStringList arguments = app.arguments();
    if (arguments.count() != 2) {
        help();
        return 1;
    }

    const QString dir = arguments.at(1);
    if (!QDir(dir).exists()) {
        help();
        return 1;
    }

    IAuthentificationService::Ptr authentificationService = IAuthentificationService::create(QUuid::createUuid().toByteArray());
    IExtensionManager::Ptr extensionManager = IExtensionManager::create();
    IServer::Ptr server = IServer::create(8080, *authentificationService, *extensionManager,
                                          dir.toStdString());

    std::vector<Extension *> extensions = extensionManager->extensions();
    qCWarning(QLoggingCategory("harmony-runner")) << "Loaded extensions:" << extensions.size();
    for (Extension *extension : extensions) {
        qCWarning(QLoggingCategory("harmony-runner")) << QString::fromStdString(extension->id()) << ":" << extension->name();
    }
    server->start();
    return app.exec();
}

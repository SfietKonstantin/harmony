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
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <serviceprovider.h>
#include <certificatemanager.h>
#include <nodemanager.h>
#include <iostream>
#include <signal.h>

void help()
{
    std::cout << "Usage:" << std::endl
              << "  harmonyrunner path/to/script" << std::endl
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

    const QString file = arguments.at(1);
    if (!QFile(file).exists()) {
        help();
        return 1;
    }

    HarmonyCertificateManager::Ptr certificateManager = HarmonyCertificateManager::create();
    if (!certificateManager->hasCertificates()) {
        certificateManager->createCertificates();
    }
    HarmonyServiceProvider::Ptr serviceProvider = HarmonyServiceProvider::create(certificateManager);
    NodeManager::Ptr nodeManager = NodeManager::create();
    NodeManager *nodeManagerData = nodeManager.data();
    QObject::connect(nodeManagerData, &NodeManager::statusChanged, [nodeManagerData]{
        qDebug() << "NodeManager status changed:" << nodeManagerData->status();
        if (nodeManagerData->status() == NodeManager::Stopped) {
            QCoreApplication::instance()->exit();
        }
    });
    nodeManager->startNode(file);

    return app.exec();
}

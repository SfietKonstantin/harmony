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

#ifndef DESKTOP
#include <sailfishapp.h>
#endif
#include <QtGui/QGuiApplication>
#include <QtQuick/QQuickView>
#include <QtCore/QPointer>
#include <QtQml/qqml.h>
#include <serviceprovider.h>

int main(int argc, char *argv[])
{

    // App
#ifndef DESKTOP
    QPointer<QGuiApplication> app = SailfishApp::application(argc, argv);
#else
    QPointer<QGuiApplication> app = QPointer<QGuiApplication>(new QGuiApplication(argc, argv));
    app->setOrganizationName("harbour-harmony");
    app->setApplicationName("harbour-harmony");
#endif

    ServiceProvider::create(app);

    // View
#ifndef DESKTOP
    QPointer<QQuickView> view = SailfishApp::createView();
    view->setSource(SailfishApp::pathTo("qml/main.qml"));
#else
    QPointer<QQuickView> view = new QQuickView();
    view->setSource(QUrl("qrc:/qml/main.qml"));
#endif
    view->show();
    return app->exec();
}

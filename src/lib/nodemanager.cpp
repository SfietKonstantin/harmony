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

#include "nodemanager.h"
#include <QtCore/QDebug>
#include <QtCore/QPointer>
#include <QtCore/QProcess>
#include <QtDBus/QDBusConnection>
#include "nodemanageradaptor.h"

static const char *PATH = "/nodemanager";
static const char *NODE_EXEC = "/usr/bin/node";
static const int TIMEOUT_INTERVAL = 10000; // 10 seconds

class NodeManagerPrivate
{
public:
    explicit NodeManagerPrivate(NodeManager *q);
    void setStatus(NodeManager::Status newStatus);
    bool registered;
    QPointer<QProcess> node;
    NodeManager::Status status;
    QTimer *timer;
private:
    NodeManager * const q_ptr;
    Q_DECLARE_PUBLIC(NodeManager)
};

NodeManagerPrivate::NodeManagerPrivate(NodeManager *q)
    : registered(false), status(NodeManager::Stopped), timer(0), q_ptr(q)
{
}

void NodeManagerPrivate::setStatus(NodeManager::Status newStatus)
{
    Q_Q(NodeManager);
    if (status != newStatus) {
        status = newStatus;
        emit q->statusChanged();
    }
}

NodeManager::NodeManager(QObject *parent)
    : QObject(parent), d_ptr(new NodeManagerPrivate(this))
{
    Q_D(NodeManager);
    d->timer = new QTimer(this);
    d->timer->setInterval(TIMEOUT_INTERVAL);
    d->timer->setSingleShot(true);
    connect(d->timer, &QTimer::timeout, [=](){
       d->node->terminate();
    });
}

NodeManager::~NodeManager()
{
    Q_D(NodeManager);
    if (d->registered) {
        QDBusConnection::sessionBus().unregisterObject(PATH);
#ifdef HARMONY_DEBUG
        qDebug() << "Unregistered DBus object" << PATH;
#endif
    }

    if (!d->node.isNull()) {
        d->node->terminate();
        if (!d->node->waitForFinished(10000)) {
            d->node->kill();
            d->node->waitForFinished(-1);
        }
    }
}

NodeManager::Ptr NodeManager::create(QObject *parent)
{
    Ptr instance = Ptr(new NodeManager(parent));
    new NodeManagerAdaptor(instance.data());

    if (!QDBusConnection::sessionBus().registerObject(PATH, instance.data())) {
        qWarning() << "Failed to register DBus object" << PATH;
        return Ptr();
    }

    instance->d_func()->registered = true;
    return instance;
}

NodeManager::Status NodeManager::status() const
{
    Q_D(const NodeManager);
    return d->status;
}

bool NodeManager::startNode(const QString &script)
{
    Q_D(NodeManager);
    if (!d->registered) {
        return false;
    }

    if (!d->node.isNull()) {
        return false;
    }

    d->setStatus(Starting);
    d->node = QPointer<QProcess>(new QProcess(this));

    d->node->setProgram(NODE_EXEC);
    QStringList args;
    args.append(script);
    d->node->setArguments(args);

    void (QProcess:: *finishedSignal)(int) = &QProcess::finished;
    connect(d->node.data(), finishedSignal, [=](int) {
        d->setStatus(Stopped);
        d->node->deleteLater();
    });
    void (QProcess:: *errorSignal)(QProcess::ProcessError) = &QProcess::error;
    connect(d->node.data(), errorSignal, [=](QProcess::ProcessError){
#ifdef HARMONY_DEBUG
        qDebug() << "Node failed:" << d->node->errorString();
#endif
        d->setStatus(NodeManager::Stopped);
    });

#ifdef HARMONY_DEBUG
    connect(d->node.data(), &QProcess::readyReadStandardOutput, [=](){
        qDebug() << d->node->readAllStandardOutput();
    });
#endif
    connect(d->node.data(), &QProcess::readyReadStandardError, [=](){
        qWarning() << d->node->readAllStandardError();
    });

    d->timer->start();
    d->node->start();

    return true;
}

void NodeManager::stopNode()
{
    Q_D(NodeManager);
    if (!d->node) {
        return;
    }
    d->node->terminate();
    d->setStatus(Stopping);
}

void NodeManager::RegisterNode()
{
    Q_D(NodeManager);
    if (!d->node) {
        return;
    }

    d->setStatus(Ready);
}

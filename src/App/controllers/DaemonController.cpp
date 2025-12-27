#include "DaemonController.h"
#include "LauncherController.h"
#include "../models/LauncherModel.h"
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

DaemonController::DaemonController(LauncherController *launcher, QObject *parent)
    : QObject(parent), m_launcher(launcher), m_server(new QLocalServer(this))
{
    connect(m_server, &QLocalServer::newConnection, this, &DaemonController::handleNewConnection);
}

DaemonController::~DaemonController()
{
    stop();
}

QString DaemonController::socketPath()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/awelauncher";
    QDir().mkpath(cacheDir);
    return cacheDir + "/ipc.sock";
}

bool DaemonController::start()
{
    QString path = socketPath();
    
    // Cleanup old socket if it exists
    QLocalServer::removeServer(path);
    
    if (!m_server->listen(path)) {
        qWarning() << "IPC Server failed to listen on" << path << ":" << m_server->errorString();
        return false;
    }
    
    qDebug() << "IPC Server listening on" << path;
    return true;
}

void DaemonController::stop()
{
    if (m_server->isListening()) {
        m_server->close();
        QLocalServer::removeServer(socketPath());
    }
}

void DaemonController::handleNewConnection()
{
    QLocalSocket *socket = m_server->nextPendingConnection();
    if (!socket) return;
    
    connect(socket, &QLocalSocket::readyRead, this, &DaemonController::handleReadyRead);
    connect(socket, &QLocalSocket::disconnected, this, &DaemonController::handleDisconnected);
}

void DaemonController::handleReadyRead()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull() || !doc.isObject()) {
        sendResponse(socket, "error", "Invalid JSON or not an object");
        return;
    }
    
    processMessage(socket, doc.object());
}

void DaemonController::handleDisconnected()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket*>(sender());
    if (socket) {
        socket->deleteLater();
    }
}

#include "../utils/Profiler.h"

void DaemonController::processMessage(QLocalSocket *socket, const QJsonObject &msg)
{
    QElapsedTimer timer;
    timer.start();
    
    QString action = msg.value("action").toString();
    QJsonObject payload = msg.value("payload").toObject();
    
    if (action == "show") {
        QString setName = payload.value("set").toString();
        QString mode = payload.value("mode").toString();
        QString initialQuery = payload.value("query").toString();

        m_launcher->loadSet(setName, mode);
        m_launcher->setVisible(true);
        if (!initialQuery.isEmpty()) {
            m_launcher->filter(initialQuery);
        }
        sendResponse(socket, "ok");
    } else if (action == "hide") {
        m_launcher->setVisible(false);
        sendResponse(socket, "ok");
    } else if (action == "toggle") {
        m_launcher->toggle();
        sendResponse(socket, "ok");
    } else if (action == "reload") {
        // Implementation for reload needed: just re-exec with same settings?
        // For now, just reload the set.
        m_launcher->loadSet(""); // default
        sendResponse(socket, "ok", "Reloading...");
    } else if (action == "query") {
        QString text = payload.value("text").toString();
        int limit = payload.value("limit").toInt(10);
        
        m_launcher->filter(text);
        
        QJsonObject data;
        QJsonArray items;
        
        auto model = m_launcher->model();
        if (model) {
            auto displayed = model->getDisplayedItems();
            int count = 0;
            for (const auto& item : displayed) {
                if (count >= limit) break;
                QJsonObject jItem;
                jItem["id"] = item.id;
                jItem["primary"] = item.primary;
                jItem["secondary"] = item.secondary;
                jItem["exec"] = item.exec;
                items.append(jItem);
                count++;
            }
        }
        
        data["count"] = items.size();
        data["items"] = items;
        sendResponse(socket, "ok", "", data);
    } else if (action == "status") {
        QJsonObject info;
        info["visible"] = m_launcher->isVisible();
        sendResponse(socket, "ok", "", info);
    } else {
        sendResponse(socket, "error", "Unknown action: " + action);
    }
    
    APP_PROFILE_POINT(timer, "Request processed: " + action);
}

void DaemonController::sendResponse(QLocalSocket *socket, const QString &status, const QString &message, const QJsonObject &data)
{
    QJsonObject response;
    response["status"] = status;
    if (!message.isEmpty()) response["message"] = message;
    if (!data.isEmpty()) response["data"] = data;
    
    socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact));
    socket->flush();
}

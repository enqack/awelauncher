#pragma once

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonObject>

class LauncherController;

class DaemonController : public QObject
{
    Q_OBJECT
public:
    explicit DaemonController(LauncherController *launcher, QObject *parent = nullptr);
    ~DaemonController();

    bool start();
    void stop();

    static QString socketPath();

private slots:
    void handleNewConnection();
    void handleReadyRead();
    void handleDisconnected();

private:
    void processMessage(QLocalSocket *socket, const QJsonObject &msg);
    void sendResponse(QLocalSocket *socket, const QString &status, const QString &message = "", const QJsonObject &data = QJsonObject());

    LauncherController *m_launcher;
    QLocalServer *m_server;
};

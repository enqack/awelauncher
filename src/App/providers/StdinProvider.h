#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <QVector>
#include "../models/LauncherModel.h"

class StdinProvider : public QObject
{
    Q_OBJECT
public:
    explicit StdinProvider(QObject *parent = nullptr);
    
    // Non-blocking read setup
    void start();
    
    QVector<LauncherItem> getItems() const;

signals:
    void itemsChanged();

private slots:
    void onActivated(int socket);

private:
    QSocketNotifier* m_notifier = nullptr;
    QVector<LauncherItem> m_items;
};

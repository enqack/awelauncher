#pragma once

#include <QObject>

class LauncherController : public QObject
{
    Q_OBJECT
public:
    explicit LauncherController(QObject *parent = nullptr);

    Q_INVOKABLE void filter(const QString &text);
    Q_INVOKABLE void activate(int index);

    void setModel(class LauncherModel* model);

signals:
    void windowVisibleChanged(bool visible);

private:
   class LauncherModel* m_model = nullptr;
};

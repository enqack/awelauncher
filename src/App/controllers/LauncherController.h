#pragma once

#include <QObject>

class LauncherController : public QObject
{
    Q_OBJECT
public:
    explicit LauncherController(QObject *parent = nullptr);

    Q_INVOKABLE void filter(const QString &text);
    Q_INVOKABLE void activate(int index);
    
    // Window mode actions
    Q_INVOKABLE void closeWindow(int index);
    Q_INVOKABLE void toggleFullscreen(int index);
    Q_INVOKABLE void toggleMaximize(int index);
    Q_INVOKABLE void toggleMinimize(int index);

    void setModel(class LauncherModel* model);
    void setWindowProvider(class WindowProvider* provider);
    
    // Output management
    Q_INVOKABLE QStringList getOutputs();
    Q_INVOKABLE void moveWindowToOutput(int index, const QString& outputName);

signals:
    void windowVisibleChanged(bool visible);

private:
   class LauncherModel* m_model = nullptr;
   class WindowProvider* m_windowProvider = nullptr;
};

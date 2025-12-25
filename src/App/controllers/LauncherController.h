#pragma once

#include <QObject>

class LauncherController : public QObject
{
    Q_OBJECT
public:
    enum SelectionMode {
        Normal,
        MonitorSelect
    };
    Q_ENUM(SelectionMode)

    Q_PROPERTY(SelectionMode selectionMode READ selectionMode NOTIFY selectionModeChanged)
    Q_PROPERTY(QString promptOverride READ promptOverride NOTIFY promptOverrideChanged)

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

    void setDmenuMode(bool enabled);

    SelectionMode selectionMode() const { return m_selectionMode; }
    QString promptOverride() const { return m_promptOverride; }

signals:
    void windowVisibleChanged(bool visible);
    void selectionModeChanged();
    void promptOverrideChanged();
    void clearSearch();

private:
   class LauncherModel* m_model = nullptr;
   class WindowProvider* m_windowProvider = nullptr;
   bool m_dmenuMode = false;
   SelectionMode m_selectionMode = Normal;
   QString m_promptOverride = "";
   QString m_pendingHandle = "";
};

#pragma once

#include <QObject>

/**
 * @class LauncherController
 * @brief Manages the main logic and interaction between UI and providers.
 * 
 * This class is exposed to QML as the @c Controller object. It handles
 * filtering results, activating items, and management of windows/monitors.
 */
class LauncherController : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief The SelectionMode enum defines the current interaction state.
     */
    enum SelectionMode {
        Normal,        /**< Standard searching and activation */
        MonitorSelect  /**< Selecting a monitor for window movement */
    };
    Q_ENUM(SelectionMode)

    /** @brief The current selection mode. Changes UI behavior. */
    Q_PROPERTY(SelectionMode selectionMode READ selectionMode NOTIFY selectionModeChanged)
    /** @brief Optional text to display in the prompt when selection mode changes. */
    Q_PROPERTY(QString promptOverride READ promptOverride NOTIFY promptOverrideChanged)

    explicit LauncherController(QObject *parent = nullptr);

    /** @brief Filters the current model based on search text. */
    Q_INVOKABLE void filter(const QString &text);
    /** @brief Activates the item at the specified index. */
    Q_INVOKABLE void activate(int index);
    
    /** @brief Closes the window at the specified index (window mode only). */
    Q_INVOKABLE void closeWindow(int index);
    /** @brief Toggles fullscreen for the window at index. */
    Q_INVOKABLE void toggleFullscreen(int index);
    /** @brief Toggles maximization for the window at index. */
    Q_INVOKABLE void toggleMaximize(int index);
    /** @brief Toggles minimization for the window at index. */
    Q_INVOKABLE void toggleMinimize(int index);
    /** @brief Initiates the monitor selection flow for the window at index. */
    Q_INVOKABLE void beginMoveToMonitor(int index);

    void setModel(class LauncherModel* model);
    void setWindowProvider(class WindowProvider* provider);
    
    /** @brief Returns names of all detected monitors. */
    Q_INVOKABLE QStringList getOutputs();
    /** @brief Moves a window to the specified monitor. */
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

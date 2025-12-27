#pragma once

#include <QObject>
#include <functional>

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
    /** @brief Visibility state of the launcher. */
    Q_PROPERTY(bool isVisible READ isVisible WRITE setVisible NOTIFY windowVisibleChanged)
    /** @brief Current prompt text. */
    Q_PROPERTY(QString prompt READ prompt NOTIFY promptChanged)
    /** @brief Current icon key. */
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    /** @brief Current mode (drun, run, etc). */
    Q_PROPERTY(QString mode READ mode NOTIFY modeChanged)
    /** @brief Whether to use explicit screen selection (true) or compositor placement (false). */
    Q_PROPERTY(bool explicitScreen READ explicitScreen WRITE setExplicitScreen NOTIFY explicitScreenChanged)

    explicit LauncherController(QObject *parent = nullptr);

    enum ActionFlag {
        None = 0,
        ForceTerminal = 1,
        HoldTerminal = 2
    };
    Q_ENUM(ActionFlag)

    // Q_PROPERTY(SelectionMode selectionMode READ selectionMode NOTIFY selectionModeChanged)
    // Removed Q_PROPERTY comment? No, just keep flow clean.

    /** @brief Filters the current model based on search text. */
    Q_INVOKABLE void filter(const QString &text);
    /** @brief Activates the item at the specified index. */
    Q_INVOKABLE void activate(int index, int flags = 0);
    
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
    
    /** @brief Hides the launcher. Quits if not in daemon mode. */
    Q_INVOKABLE void hide();
    /** @brief Toggles the launcher visibility. */
    Q_INVOKABLE void toggle();
    /** @brief Explicitly quits the application. */
    Q_INVOKABLE void quit();
    
    /** @brief Loads a specific provider set. */
    Q_INVOKABLE void loadSet(const QString &setName, const QString &mode = "");

    void setModel(class LauncherModel* model);
    class LauncherModel* model() const { return m_model; }
    void setWindowProvider(class WindowProvider* provider);
    
    /** @brief Returns names of all detected monitors. */
    Q_INVOKABLE QStringList getOutputs();
    /** @brief Moves a window to the specified monitor. */
    Q_INVOKABLE void moveWindowToOutput(int index, const QString& outputName);

    void setDmenuMode(bool enabled);
    void setDaemonMode(bool enabled);

    SelectionMode selectionMode() const { return m_selectionMode; }
    QString promptOverride() const { return m_promptOverride; }
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible);
    
    /** @brief Sets a callback to be invoked when the UI is first requested. */
    void setUiInitializer(std::function<void()> callback) { m_uiInitializer = callback; }
    void setMainWindow(class QWindow* window) { m_mainWindow = window; }
    
    /** @brief Forcefully asks the OS for focus. */
    Q_INVOKABLE void requestFocus();
    
    QString prompt() const { return m_selectionMode == Normal ? m_prompt : m_promptOverride; }
    QString icon() const { return m_icon; }
    QString mode() const { return m_mode; }
    bool explicitScreen() const { return m_explicitScreen; }
    void setExplicitScreen(bool explicitScreen) {
        if (m_explicitScreen != explicitScreen) {
            m_explicitScreen = explicitScreen;
            emit explicitScreenChanged();
        }
    }

signals:
    void windowVisibleChanged(bool visible);
    void explicitScreenChanged();
    void promptChanged();
    void iconChanged();
    void modeChanged();
    void selectionModeChanged();
    void promptOverrideChanged();
    void clearSearch();

private:
   class LauncherModel* m_model = nullptr;
   class WindowProvider* m_windowProvider = nullptr;
   bool m_dmenuMode = false;
   bool m_daemonMode = false;
   bool m_visible = false;
   bool m_explicitScreen = false;
   SelectionMode m_selectionMode = Normal;
   QString m_prompt = "Search...";
   QString m_icon = "search";
   QString m_mode = "drun";
   QString m_promptOverride = "";
   QString m_pendingHandle = "";
    std::function<void()> m_uiInitializer = nullptr;
    class QWindow* m_mainWindow = nullptr;
};

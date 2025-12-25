#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QSet>
#include "../models/LauncherModel.h"

// Forward declarations for Wayland types
struct wl_display;
struct wl_registry;
struct wl_array;
struct wl_seat;
struct wl_output;
struct zwlr_foreign_toplevel_manager_v1;
struct zwlr_foreign_toplevel_handle_v1;

/**
 * @class WindowProvider
 * @brief Interacts with the Wayland compositor to manage foreign toplevel windows.
 * 
 * This class uses the @c wlr-foreign-toplevel-management-unstable-v1 protocol to
 * enumerate windows, track their titles and application IDs, and perform actions
 * such as activation, closing, or moving between monitors.
 */
class WindowProvider : public QObject
{
    Q_OBJECT
public:
    explicit WindowProvider(QObject *parent = nullptr);
    ~WindowProvider();
    
    /**
     * @brief Connects to the Wayland display and binds required globals.
     * @return true if successful, false otherwise.
     */
    bool initialize();
    
    /**
     * @brief Returns the current list of windows mapped to LauncherItems.
     */
    QVector<LauncherItem> getWindows();
    
    /** @brief Requests the compositor to activate (focus) a window. */
    void activateWindow(const QString& handle);
    /** @brief Requests the compositor to close a window. */
    void closeWindow(const QString& handle);
    /** @brief Toggles the fullscreen state of a window. */
    void toggleFullscreen(const QString& handle);
    /** @brief Toggles the maximized state of a window. */
    void toggleMaximize(const QString& handle);
    /** @brief Requests the compositor to minimize a window. */
    void toggleMinimize(const QString& handle);
    /** 
     * @brief Moves a window to a specific output.
     * @note This uses a set_fullscreen(output) hack as a direct move is not yet supported in the protocol.
     */
    void moveToOutput(const QString& handle, const QString& outputName);
    
    /** @brief Returns a list of friendly names for all detected monitors. */
    QStringList getOutputNames() const;
    
signals:
    /** @brief Emitted when windows are added, removed, or changed. */
    void windowsChanged();
    
private:
    /** @brief Internal tracking for a single Wayland toplevel window. */
    struct WindowInfo {
        zwlr_foreign_toplevel_handle_v1* handle; /**< The Wayland protocol handle */
        QString title;                           /**< Current window title */
        QString appId;                           /**< Application identifier */
        QSet<int> state;                         /**< Active states (Maximized, Fullscreen, etc.) */
    };
    
    /** @brief Internal tracking for a Wayland output (monitor). */
    struct OutputInfo {
        wl_output* output;      /**< The Wayland output handle */
        QString name;           /**< User-friendly name (e.g. "eDP-1") */
        QString description;    /**< Verbose description */
        int32_t x, y;           /**< Logical coordinates */
        int32_t width, height;  /**< Logical dimensions */
    };
    
    wl_display* m_display;
    wl_registry* m_registry;
    wl_seat* m_seat;
    zwlr_foreign_toplevel_manager_v1* m_manager;
    QMap<zwlr_foreign_toplevel_handle_v1*, WindowInfo> m_windows;
    QMap<wl_output*, OutputInfo> m_outputs;
    
    // Wayland callbacks
    static void registryGlobal(void* data, wl_registry* registry,
                              uint32_t name, const char* interface,
                              uint32_t version);
    static void registryGlobalRemove(void* data, wl_registry* registry,
                                    uint32_t name);
    
    static void handleToplevel(void* data,
                              zwlr_foreign_toplevel_manager_v1* manager,
                              zwlr_foreign_toplevel_handle_v1* toplevel);
    static void handleFinished(void* data,
                              zwlr_foreign_toplevel_manager_v1* manager);
    
    static void toplevelTitle(void* data,
                             zwlr_foreign_toplevel_handle_v1* handle,
                             const char* title);
    static void toplevelAppId(void* data,
                             zwlr_foreign_toplevel_handle_v1* handle,
                             const char* appId);
    static void toplevelState(void* data,
                             zwlr_foreign_toplevel_handle_v1* handle,
                             wl_array* state);
    static void toplevelClosed(void* data,
                              zwlr_foreign_toplevel_handle_v1* handle);
    
    // Output callbacks
    static void outputGeometry(void* data, wl_output* output,
                              int32_t x, int32_t y,
                              int32_t physical_width, int32_t physical_height,
                              int32_t subpixel, const char* make,
                              const char* model, int32_t transform);
    static void outputMode(void* data, wl_output* output,
                          uint32_t flags, int32_t width, int32_t height,
                          int32_t refresh);
    static void outputDone(void* data, wl_output* output);
    static void outputScale(void* data, wl_output* output, int32_t factor);
    static void outputName(void* data, wl_output* output, const char* name);
    static void outputDescription(void* data, wl_output* output, const char* description);
};

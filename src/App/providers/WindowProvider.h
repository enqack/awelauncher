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

class WindowProvider : public QObject
{
    Q_OBJECT
public:
    explicit WindowProvider(QObject *parent = nullptr);
    ~WindowProvider();
    
    // Initialize connection to Wayland
    bool initialize();
    
    // Get list of windows as LauncherItems
    QVector<LauncherItem> getWindows();
    
    // Window actions
    void activateWindow(const QString& handle);
    void closeWindow(const QString& handle);
    void toggleFullscreen(const QString& handle);
    void toggleMaximize(const QString& handle);
    void toggleMinimize(const QString& handle);
    void moveToOutput(const QString& handle, const QString& outputName);
    
    // Output management
    QStringList getOutputNames() const;
    
signals:
    void windowsChanged();
    
private:
    struct WindowInfo {
        zwlr_foreign_toplevel_handle_v1* handle;
        QString title;
        QString appId;
        QSet<int> state; // Wayland state flags
    };
    
    struct OutputInfo {
        wl_output* output;
        QString name;
        QString description;
        int32_t x, y;
        int32_t width, height;
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

#include "WindowProvider.h"
#include <wayland-client.h>
#include "wlr-foreign-toplevel-client-protocol.h"
#include <QDebug>

WindowProvider::WindowProvider(QObject *parent)
    : QObject(parent)
    , m_display(nullptr)
    , m_registry(nullptr)
    , m_seat(nullptr)
    , m_manager(nullptr)
{
}

WindowProvider::~WindowProvider()
{
    // Clean up Wayland resources
    if (m_manager) {
        zwlr_foreign_toplevel_manager_v1_destroy(m_manager);
    }
    if (m_registry) {
        wl_registry_destroy(m_registry);
    }
    if (m_display) {
        wl_display_disconnect(m_display);
    }
}

bool WindowProvider::initialize()
{
    // Connect to Wayland display
    m_display = wl_display_connect(nullptr);
    if (!m_display) {
        qWarning() << "Failed to connect to Wayland display";
        return false;
    }
    
    // Get registry
    m_registry = wl_display_get_registry(m_display);
    if (!m_registry) {
        qWarning() << "Failed to get Wayland registry";
        return false;
    }
    
    // Set up registry listener
    static const wl_registry_listener registry_listener = {
        .global = registryGlobal,
        .global_remove = registryGlobalRemove
    };
    
    wl_registry_add_listener(m_registry, &registry_listener, this);
    
    // Roundtrip to get globals
    wl_display_roundtrip(m_display);
    
    if (!m_manager) {
        qWarning() << "Compositor doesn't support wlr-foreign-toplevel-management";
        return false;
    }
    
    // Another roundtrip to get initial toplevels
    wl_display_roundtrip(m_display);
    
    return true;
}

QVector<LauncherItem> WindowProvider::getWindows()
{
    QVector<LauncherItem> items;
    
    for (auto it = m_windows.begin(); it != m_windows.end(); ++it) {
        const WindowInfo& info = it.value();
        
        LauncherItem item;
        item.id = QString::number((quintptr)it.key()); // Use handle pointer as ID
        item.primary = info.title.isEmpty() ? info.appId : info.title;
        item.secondary = info.appId;
        item.iconKey = info.appId; // Use app_id for icon lookup
        item.exec = ""; // Not applicable for windows
        item.terminal = false;
        
        items.append(item);
    }
    
    return items;
}

void WindowProvider::activateWindow(const QString& handleStr)
{
    quintptr handlePtr = handleStr.toULongLong();
    auto* handle = reinterpret_cast<zwlr_foreign_toplevel_handle_v1*>(handlePtr);
    
    if (m_windows.contains(handle) && m_seat) {
        zwlr_foreign_toplevel_handle_v1_activate(handle, m_seat);
        wl_display_flush(m_display);
    }
}

void WindowProvider::closeWindow(const QString& handleStr)
{
    quintptr handlePtr = handleStr.toULongLong();
    auto* handle = reinterpret_cast<zwlr_foreign_toplevel_handle_v1*>(handlePtr);
    
    if (m_windows.contains(handle)) {
        zwlr_foreign_toplevel_handle_v1_close(handle);
        wl_display_flush(m_display);
    }
}

void WindowProvider::toggleFullscreen(const QString& handleStr)
{
    quintptr handlePtr = handleStr.toULongLong();
    auto* handle = reinterpret_cast<zwlr_foreign_toplevel_handle_v1*>(handlePtr);
    
    if (m_windows.contains(handle)) {
        const WindowInfo& info = m_windows[handle];
        bool isFullscreen = info.state.contains(ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN);
        
        if (isFullscreen) {
            zwlr_foreign_toplevel_handle_v1_unset_fullscreen(handle);
        } else {
            zwlr_foreign_toplevel_handle_v1_set_fullscreen(handle, nullptr);
        }
        wl_display_flush(m_display);
    }
}

void WindowProvider::toggleMaximize(const QString& handleStr)
{
    quintptr handlePtr = handleStr.toULongLong();
    auto* handle = reinterpret_cast<zwlr_foreign_toplevel_handle_v1*>(handlePtr);
    
    if (m_windows.contains(handle)) {
        const WindowInfo& info = m_windows[handle];
        bool isMaximized = info.state.contains(ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED);
        
        if (isMaximized) {
            zwlr_foreign_toplevel_handle_v1_unset_maximized(handle);
        } else {
            zwlr_foreign_toplevel_handle_v1_set_maximized(handle);
        }
        wl_display_flush(m_display);
    }
}

void WindowProvider::toggleMinimize(const QString& handleStr)
{
    quintptr handlePtr = handleStr.toULongLong();
    auto* handle = reinterpret_cast<zwlr_foreign_toplevel_handle_v1*>(handlePtr);
    
    if (m_windows.contains(handle)) {
        const WindowInfo& info = m_windows[handle];
        bool isMinimized = info.state.contains(ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED);
        
        if (isMinimized) {
            zwlr_foreign_toplevel_handle_v1_unset_minimized(handle);
        } else {
            zwlr_foreign_toplevel_handle_v1_set_minimized(handle);
        }
        wl_display_flush(m_display);
    }
}

void WindowProvider::moveToOutput(const QString& handleStr, const QString& outputName)
{
    quintptr handlePtr = handleStr.toULongLong();
    auto* handle = reinterpret_cast<zwlr_foreign_toplevel_handle_v1*>(handlePtr);
    
    if (!m_windows.contains(handle)) return;
    
    // Find output by name
    wl_output* targetOutput = nullptr;
    for (auto it = m_outputs.begin(); it != m_outputs.end(); ++it) {
        if (it.value().name == outputName) {
            targetOutput = it.key();
            break;
        }
    }
    
    if (targetOutput) {
        const WindowInfo& info = m_windows[handle];
        bool wasFullscreen = info.state.contains(ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN);

        zwlr_foreign_toplevel_handle_v1_set_fullscreen(handle, targetOutput);

        // If it wasn't fullscreen before, unset it so it just stays on the new monitor
        if (!wasFullscreen) {
            zwlr_foreign_toplevel_handle_v1_unset_fullscreen(handle);
        }

        // Activate the window after moving it
        if (m_seat) {
            zwlr_foreign_toplevel_handle_v1_activate(handle, m_seat);
        }
        wl_display_flush(m_display);
        qDebug() << "Moved window to output:" << outputName;
    } else {
        qWarning() << "Output not found:" << outputName;
    }
}

QStringList WindowProvider::getOutputNames() const
{
    QStringList names;
    for (const auto& output : m_outputs) {
        if (!output.name.isEmpty()) {
            names.append(output.name);
        } else if (!output.description.isEmpty()) {
            names.append(output.description);
        }
    }
    return names;
}

// Static callbacks
void WindowProvider::registryGlobal(void* data, wl_registry* registry,
                                   uint32_t name, const char* interface,
                                   uint32_t version)
{
    auto* provider = static_cast<WindowProvider*>(data);
    
    if (strcmp(interface, "wl_seat") == 0) {
        provider->m_seat = static_cast<wl_seat*>(
            wl_registry_bind(registry, name, &wl_seat_interface, 1));
    }
    else if (strcmp(interface, "wl_output") == 0) {
        wl_output* output = static_cast<wl_output*>(
            wl_registry_bind(registry, name, &wl_output_interface, 4));
        
        // Create output info
        OutputInfo info;
        info.output = output;
        info.x = info.y = 0;
        info.width = info.height = 0;
        provider->m_outputs[output] = info;
        
        // Set up output listener
        static const wl_output_listener output_listener = {
            .geometry = outputGeometry,
            .mode = outputMode,
            .done = outputDone,
            .scale = outputScale,
            .name = outputName,
            .description = outputDescription
        };
        
        wl_output_add_listener(output, &output_listener, provider);
    }
    else if (strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
        provider->m_manager = static_cast<zwlr_foreign_toplevel_manager_v1*>(
            wl_registry_bind(registry, name, &zwlr_foreign_toplevel_manager_v1_interface, 
                           qMin(version, 3u)));
        
        // Set up manager listener
        static const zwlr_foreign_toplevel_manager_v1_listener manager_listener = {
            .toplevel = handleToplevel,
            .finished = handleFinished
        };
        
        zwlr_foreign_toplevel_manager_v1_add_listener(provider->m_manager, 
                                                     &manager_listener, provider);
    }
}

void WindowProvider::registryGlobalRemove(void* data, wl_registry* registry,
                                         uint32_t name)
{
    // Handle global removal if needed
}

void WindowProvider::handleToplevel(void* data,
                                   zwlr_foreign_toplevel_manager_v1* manager,
                                   zwlr_foreign_toplevel_handle_v1* toplevel)
{
    auto* provider = static_cast<WindowProvider*>(data);
    
    // Create window info
    WindowInfo info;
    info.handle = toplevel;
    provider->m_windows[toplevel] = info;
    
    // Set up toplevel listener
    static const zwlr_foreign_toplevel_handle_v1_listener toplevel_listener = {
        .title = toplevelTitle,
        .app_id = toplevelAppId,
        .output_enter = [](void*, zwlr_foreign_toplevel_handle_v1*, wl_output*) {},
        .output_leave = [](void*, zwlr_foreign_toplevel_handle_v1*, wl_output*) {},
        .state = toplevelState,
        .done = [](void*, zwlr_foreign_toplevel_handle_v1*) {}, // Empty done handler
        .closed = toplevelClosed,
        .parent = nullptr
    };
    
    zwlr_foreign_toplevel_handle_v1_add_listener(toplevel, &toplevel_listener, provider);
    
    emit provider->windowsChanged();
}

void WindowProvider::handleFinished(void* data,
                                   zwlr_foreign_toplevel_manager_v1* manager)
{
    qDebug() << "Foreign toplevel manager finished";
}

void WindowProvider::toplevelTitle(void* data,
                                  zwlr_foreign_toplevel_handle_v1* handle,
                                  const char* title)
{
    auto* provider = static_cast<WindowProvider*>(data);
    if (provider->m_windows.contains(handle)) {
        provider->m_windows[handle].title = QString::fromUtf8(title);
        emit provider->windowsChanged();
    }
}

void WindowProvider::toplevelAppId(void* data,
                                  zwlr_foreign_toplevel_handle_v1* handle,
                                  const char* appId)
{
    auto* provider = static_cast<WindowProvider*>(data);
    if (provider->m_windows.contains(handle)) {
        provider->m_windows[handle].appId = QString::fromUtf8(appId);
        emit provider->windowsChanged();
    }
}

void WindowProvider::toplevelState(void* data,
                                  zwlr_foreign_toplevel_handle_v1* handle,
                                  wl_array* state)
{
    auto* provider = static_cast<WindowProvider*>(data);
    if (provider->m_windows.contains(handle)) {
        provider->m_windows[handle].state.clear();
        
        // Manual iteration instead of wl_array_for_each macro
        uint32_t* stateData = static_cast<uint32_t*>(state->data);
        size_t count = state->size / sizeof(uint32_t);
        for (size_t i = 0; i < count; ++i) {
            provider->m_windows[handle].state.insert(static_cast<int>(stateData[i]));
        }
        
        emit provider->windowsChanged();
    }
}

void WindowProvider::toplevelClosed(void* data,
                                   zwlr_foreign_toplevel_handle_v1* handle)
{
    auto* provider = static_cast<WindowProvider*>(data);
    provider->m_windows.remove(handle);
    zwlr_foreign_toplevel_handle_v1_destroy(handle);
    emit provider->windowsChanged();
}

// Output callbacks
void WindowProvider::outputGeometry(void* data, wl_output* output,
                                   int32_t x, int32_t y,
                                   int32_t physical_width, int32_t physical_height,
                                   int32_t subpixel, const char* make,
                                   const char* model, int32_t transform)
{
    auto* provider = static_cast<WindowProvider*>(data);
    if (provider->m_outputs.contains(output)) {
        provider->m_outputs[output].x = x;
        provider->m_outputs[output].y = y;
    }
}

void WindowProvider::outputMode(void* data, wl_output* output,
                               uint32_t flags, int32_t width, int32_t height,
                               int32_t refresh)
{
    auto* provider = static_cast<WindowProvider*>(data);
    if (provider->m_outputs.contains(output)) {
        provider->m_outputs[output].width = width;
        provider->m_outputs[output].height = height;
    }
}

void WindowProvider::outputDone(void* data, wl_output* output)
{
    // Output configuration is complete
    auto* provider = static_cast<WindowProvider*>(data);
    if (provider->m_outputs.contains(output)) {
        const auto& info = provider->m_outputs[output];
        qDebug() << "Output configured:" << info.name << info.width << "x" << info.height;
    }
}

void WindowProvider::outputScale(void* data, wl_output* output, int32_t factor)
{
    // HiDPI scale factor - we don't need to track this for window move
}

void WindowProvider::outputName(void* data, wl_output* output, const char* name)
{
    auto* provider = static_cast<WindowProvider*>(data);
    if (provider->m_outputs.contains(output)) {
        provider->m_outputs[output].name = QString::fromUtf8(name);
        qDebug() << "Output name:" << name;
    }
}

void WindowProvider::outputDescription(void* data, wl_output* output, const char* description)
{
    auto* provider = static_cast<WindowProvider*>(data);
    if (provider->m_outputs.contains(output)) {
        provider->m_outputs[output].description = QString::fromUtf8(description);
        qDebug() << "Output description:" << description;
    }
}

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import awelauncher
import org.kde.layershell as KWayland

Window {
    id: root
    visible: launcher.isVisible
    
    // LayerShell properties
    KWayland.Window.layer: AppTheme.windowLayer === 2 ? KWayland.Window.LayerOverlay : KWayland.Window.LayerTop
    KWayland.Window.keyboardInteractivity: KWayland.Window.KeyboardInteractivityOnDemand
    // Use Compositor placement by default (follow focus), but allow manual override if explicitScreen is set
    KWayland.Window.screenConfiguration: launcher.explicitScreen ? KWayland.Window.ScreenFromQWindow : KWayland.Window.ScreenFromCompositor
    KWayland.Window.exclusionZone: AppTheme.windowLayer === 2 ? -1 : 0
    KWayland.Window.scope: "awelauncher"
    
    flags: Qt.FramelessWindowHint
    
    KWayland.Window.anchors: {
        var a = 0;
        switch(AppTheme.windowAnchor) {
            case "top": a |= KWayland.Window.AnchorTop; break;
            case "bottom": a |= KWayland.Window.AnchorBottom; break;
            case "left": a |= KWayland.Window.AnchorLeft; break;
            case "right": a |= KWayland.Window.AnchorRight; break;
            case "center": a = 0; break;
            default: a |= KWayland.Window.AnchorTop;
        }
        return a;
    }
    
    KWayland.Window.margins.top: (AppTheme.windowAnchor === "top" ? AppTheme.windowMargin : 0)
    KWayland.Window.margins.bottom: (AppTheme.windowAnchor === "bottom" ? AppTheme.windowMargin : 0)
    KWayland.Window.margins.left: (AppTheme.windowAnchor === "left" ? AppTheme.windowMargin : 0)
    KWayland.Window.margins.right: (AppTheme.windowAnchor === "right" ? AppTheme.windowMargin : 0)

    // Direct sizing
    width: AppTheme.windowWidth
    height: AppTheme.windowHeight
    minimumWidth: width
    maximumWidth: width
    minimumHeight: height
    maximumHeight: height
    
    // Direct anchoring (Managed by LayerShell now)
    /*
    x: {
        switch(AppTheme.windowAnchor) {
            case "top": return (Screen.width - width) / 2;
            case "bottom": return (Screen.width - width) / 2;
            case "left": return 0;
            case "right": return Screen.width - width;
            case "center": 
            default: return (Screen.width - width) / 2;
        }
    }
    y: {
        switch(AppTheme.windowAnchor) {
            case "top": return 0;
            case "bottom": return Screen.height - height;
            case "left": return (Screen.height - height) / 2;
            case "right": return (Screen.height - height) / 2;
            case "center":
            default: return (Screen.height - height) / 2;
        }
    }
    */

    color: "transparent"
    
    // opacity: AppTheme.opacity // Removed: unsupported by LayerShell
    
    onActiveChanged: {
        if (!active) {
            launcher.hide()
        }
    }
    
    // Manage focus and state when visibility changes
    onVisibleChanged: {
        if (visible) {
            // Standard, safe focus request. 
            // We removed the aggressively repeating "Enforcer" as it caused crashes during interaction on some compositors.
            searchBar.forceInputFocus()
            launcher.requestFocus() 
            
            // Late Focus Attempt (Single Shot) to catch focus if lost during mapping
            delayedFocus.restart()
        }
    }
    
    Timer {
        id: delayedFocus
        interval: 100
        repeat: false
        onTriggered: {
            if (debugMode) console.log("Delayed Focus Triggered")
            searchBar.forceInputFocus()
            launcher.requestFocus()
        }
    }

    Component.onCompleted: {
        if (debugMode) console.log("AppTheme Debug - Anchor:", AppTheme.windowAnchor, "Margin:", AppTheme.windowMargin, "Layer:", AppTheme.windowLayer)
        // Handled by Enforcer on visible change (which defaults to false, then true via controller)
    }
    
    // ...
    
    property bool showHelp: false

    // Main Background
    Rectangle {
        anchors.fill: parent
        // Apply opacity to the background color instead of the window
        color: Qt.rgba(AppTheme.bg.r, AppTheme.bg.g, AppTheme.bg.b, AppTheme.opacity)
        radius: AppTheme.radius
        border.color: root.active ? AppTheme.accent : AppTheme.border
        border.width: AppTheme.borderWidth
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: AppTheme.padding
            spacing: AppTheme.padding

                // Header / Search
                SearchBar {
                    id: searchBar
                    prompt: launcher.prompt
                    
                    onCloseRequested: launcher.hide()
                    onNavigateDown: resultsList.currentIndex = Math.min(resultsList.count - 1, resultsList.currentIndex + 1)
                    onNavigateUp: resultsList.currentIndex = Math.max(0, resultsList.currentIndex - 1)
                    
                    onNavigatePageDown: {
                        var itemHeight = AppTheme.rowHeight + resultsList.spacing
                        var itemsPerPage = Math.floor(resultsList.height / itemHeight)
                        var currentItemY = resultsList.currentIndex * itemHeight
                        var relativeY = currentItemY - resultsList.contentY
                        
                        var nextIndex = Math.min(resultsList.count - 1, resultsList.currentIndex + itemsPerPage)
                        resultsList.currentIndex = nextIndex
                        
                        // Sync Viewport
                        resultsList.contentY = (nextIndex * itemHeight) - relativeY
                    }
                    onNavigatePageUp: {
                        var itemHeight = AppTheme.rowHeight + resultsList.spacing
                        var itemsPerPage = Math.floor(resultsList.height / itemHeight)
                        var currentItemY = resultsList.currentIndex * itemHeight
                        var relativeY = currentItemY - resultsList.contentY
                        
                        var nextIndex = Math.max(0, resultsList.currentIndex - itemsPerPage)
                        resultsList.currentIndex = nextIndex
                        
                        // Sync Viewport
                        resultsList.contentY = (nextIndex * itemHeight) - relativeY
                    }
                    onNavigateHome: resultsList.currentIndex = 0
                    onNavigateEnd: resultsList.currentIndex = resultsList.count - 1
                    onActivateCurrent: (forceTerminal) => launcher.activate(resultsList.currentIndex, forceTerminal)
                    onSearchChanged: (text) => {
                         resultsList.currentIndex = 0
                    }
                }

                // Results
                ListView {
                    id: resultsList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: LauncherModel
                    
                    delegate: ResultRow {
                        width: ListView.view.width
                    }
                    
                    highlight: Rectangle {
                        color: AppTheme.selected
                        radius: AppTheme.radius
                    }
                    highlightMoveDuration: 100
                    spacing: AppTheme.padding / 4
                }
                
                // Empty State Overlay
                Item {
                    id: emptyState
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    visible: resultsList.count === 0
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: AppTheme.padding
                        
                        Image {
                            Layout.alignment: Qt.AlignHCenter
                            source: "image://icon/" + launcher.icon
                            sourceSize.width: 96
                            sourceSize.height: 96
                            opacity: 0.3
                        }
                        
                        Text {
                            Layout.alignment: Qt.AlignHCenter
                            text: "No results found"
                            color: AppTheme.muted
                            font.pixelSize: AppTheme.fontSize
                            opacity: 0.7
                        }
                    }
                }
                
                // Footer
                Footer {
                   resultCount: resultsList.count
                   showMode: launcher.mode
                }
            }
        }
    
    MouseArea { 
        // Close if clicking outside the launcher in the transparent area
        anchors.fill: parent
        z: -1
        onClicked: {
            if (showHelp) showHelp = false;
            else launcher.hide();
        }
    }
    
    // Help overlay
    HelpOverlay {
        visible: showHelp
        showMode: launcher.mode
        onCloseRequested: showHelp = false
    }


    
    // Close on Escape
    Shortcut {
        sequence: "Esc"
        onActivated: {
            if (showHelp) {
                showHelp = false
            } else {
                launcher.hide()
            }
        }
    }
    
    Shortcut {
        sequence: "Ctrl+H"
        onActivated: showHelp = !showHelp
    }
    
    // Window mode actions
    Shortcut {
        enabled: launcher.mode === "window"
        sequence: "Ctrl+D"
        onActivated: launcher.closeWindow(resultsList.currentIndex)
    }
    
    Shortcut {
        enabled: launcher.mode === "window"
        sequence: "Ctrl+F"
        onActivated: launcher.toggleFullscreen(resultsList.currentIndex)
    }
    
    Shortcut {
        enabled: launcher.mode === "window"
        sequence: "Ctrl+X"
        onActivated: launcher.toggleMaximize(resultsList.currentIndex)
    }
    
    Shortcut {
        enabled: launcher.mode === "window"
        sequence: "Ctrl+N"
        onActivated: launcher.toggleMinimize(resultsList.currentIndex)
    }
    
    Shortcut {
        enabled: launcher.mode === "window"
        sequence: "Ctrl+M"
        onActivated: launcher.beginMoveToMonitor(resultsList.currentIndex)
    }
    
}


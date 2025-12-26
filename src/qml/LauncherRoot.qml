import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import awelauncher
import org.kde.layershell as KWayland

Window {
    id: root
    visible: true
    
    // LayerShell properties
    KWayland.Window.layer: AppTheme.windowLayer === 2 ? KWayland.Window.LayerOverlay : KWayland.Window.LayerTop
    KWayland.Window.keyboardInteractivity: KWayland.Window.KeyboardInteractivityOnDemand
    KWayland.Window.screenConfiguration: KWayland.Window.ScreenFromCompositor
    KWayland.Window.exclusionZone: AppTheme.windowLayer === 2 ? -1 : 0
    
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
            Qt.quit()
        }
    }

    Component.onCompleted: {
        if (debugMode) console.log("AppTheme Debug - Anchor:", AppTheme.windowAnchor, "Margin:", AppTheme.windowMargin, "Layer:", AppTheme.windowLayer)
        // Immediate attempt
        root.requestActivate()
        searchBar.forceInputFocus()
        
        // Delayed attempt to ensure focus after mapping
        focusTimer.start()
    }
    
    // ...

    Timer {
        id: focusTimer
        interval: 100
        repeat: false
        onTriggered: {
            if (debugMode) console.log("Retrying focus...")
            root.requestActivate()
            searchBar.forceInputFocus()
            root.raise()
        }
    }
    
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
                    
                    onCloseRequested: root.close()
                    onNavigateDown: resultsList.currentIndex = Math.min(resultsList.count - 1, resultsList.currentIndex + 1)
                    onNavigateUp: resultsList.currentIndex = Math.max(0, resultsList.currentIndex - 1)
                    onActivateCurrent: (forceTerminal) => Controller.activate(resultsList.currentIndex, forceTerminal)
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
                            source: "image://icon/face-sad" // or "dialog-information"
                            sourceSize.width: 64
                            sourceSize.height: 64
                            opacity: 0.5
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
                   showMode: cliShowMode
                }
            }
        }
    
    MouseArea { 
        // Close if clicking outside the launcher in the transparent area
        anchors.fill: parent
        z: -1
        onClicked: {
            if (showHelp) showHelp = false;
            else Qt.quit();
        }
    }
    
    // Help overlay
    HelpOverlay {
        visible: showHelp
        showMode: cliShowMode
        onCloseRequested: showHelp = false
    }


    
    // Close on Escape
    Shortcut {
        sequence: "Esc"
        onActivated: {
            if (showHelp) {
                showHelp = false
            } else {
                Qt.quit()
            }
        }
    }
    
    Shortcut {
        sequence: "Ctrl+H"
        onActivated: showHelp = !showHelp
    }
    
    // Window mode actions
    Shortcut {
        enabled: cliShowMode === "window"
        sequence: "Ctrl+D"
        onActivated: Controller.closeWindow(resultsList.currentIndex)
    }
    
    Shortcut {
        enabled: cliShowMode === "window"
        sequence: "Ctrl+F"
        onActivated: Controller.toggleFullscreen(resultsList.currentIndex)
    }
    
    Shortcut {
        enabled: cliShowMode === "window"
        sequence: "Ctrl+X"
        onActivated: Controller.toggleMaximize(resultsList.currentIndex)
    }
    
    Shortcut {
        enabled: cliShowMode === "window"
        sequence: "Ctrl+N"
        onActivated: Controller.toggleMinimize(resultsList.currentIndex)
    }
    
    Shortcut {
        enabled: cliShowMode === "window"
        sequence: "Ctrl+M"
        onActivated: Controller.beginMoveToMonitor(resultsList.currentIndex)
    }
    
}


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
    
    KWayland.Window.margins: ({
        "top": (AppTheme.windowAnchor === "top" ? AppTheme.windowMargin : 0),
        "bottom": (AppTheme.windowAnchor === "bottom" ? AppTheme.windowMargin : 0),
        "left": (AppTheme.windowAnchor === "left" ? AppTheme.windowMargin : 0),
        "right": (AppTheme.windowAnchor === "right" ? AppTheme.windowMargin : 0)
    })

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

    color: AppTheme.bg
    
    opacity: AppTheme.opacity
    
    onActiveChanged: {
        if (!active) {
            Qt.quit()
        }
    }

    Component.onCompleted: {
        // Immediate attempt
        root.requestActivate()
        searchInput.forceActiveFocus()
        
        // Delayed attempt to ensure focus after mapping
        focusTimer.start()
    }
    
    Timer {
        id: focusTimer
        interval: 100
        repeat: false
        onTriggered: {
            if (debugMode) console.log("Retrying focus...")
            root.requestActivate()
            searchInput.forceActiveFocus()
            root.raise()
        }
    }
    
    property bool showHelp: false
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: AppTheme.padding
        spacing: 16

            // Header / Search
            Rectangle {
                Layout.fillWidth: true
                height: AppTheme.rowHeight
                color: Qt.lighter(AppTheme.bg, 1.2)
                radius: 8
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 12
                    spacing: 10
                    
                    Text { 
                        text: "" // Nerd font icon or just search
                        color: AppTheme.muted ? AppTheme.muted : "#888" 
                        font.pixelSize: AppTheme.fontSize
                    }

                    TextInput {
                        id: searchInput
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        verticalAlignment: TextInput.AlignVCenter
                        font.pixelSize: AppTheme.fontSize
                        color: AppTheme.fg
                        focus: true
                        selectByMouse: true
                        
                        onTextChanged: Controller.filter(text)
                        
                        Keys.onDownPressed: resultsList.incrementCurrentIndex()
                        Keys.onUpPressed: resultsList.decrementCurrentIndex()
                        Keys.onEnterPressed: Controller.activate(resultsList.currentIndex)
                        Keys.onReturnPressed: Controller.activate(resultsList.currentIndex)
                    }
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
                    radius: 8
                }
                highlightMoveDuration: 100
                spacing: 4
            }
            
            // Footer
            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: resultsList.count + " results"
                    color: Qt.darker(AppTheme.fg, 1.5)
                    font.pixelSize: AppTheme.fontSize * 0.7
                    Layout.preferredWidth: implicitWidth 
                }
                Item { Layout.fillWidth: true }
                Text {
                    Layout.maximumWidth: AppTheme.windowWidth * 0.6
                    text: cliShowMode === "window" ? 

                          "Ctrl+H for help" :
                          "Enter to select • Esc to close"
                    color: Qt.darker(AppTheme.fg, 1.5)
                    font.pixelSize: AppTheme.fontSize * 0.7
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignRight
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
    Rectangle {
        anchors.fill: parent
        visible: showHelp
        color: Qt.rgba(0, 0, 0, 0.9)
        radius: AppTheme.radius
        
        MouseArea {
            anchors.fill: parent
            onClicked: showHelp = false
        }
        
        ColumnLayout {
            anchors.centerIn: parent
            spacing: 20
            
            Text {
                text: "Keybindings"
                color: AppTheme.accent
                font.pixelSize: AppTheme.fontSize * 1.5
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
            }
            
            ColumnLayout {
                spacing: 8
                visible: cliShowMode === "window"
                
                Text {
                    text: "Enter - Switch to window"
                    color: AppTheme.fg
                    font.pixelSize: AppTheme.fontSize
                }
                Text {
                    text: "Ctrl+D - Close window"
                    color: AppTheme.fg
                    font.pixelSize: AppTheme.fontSize
                }
                Text {
                    text: "Ctrl+F - Toggle fullscreen"
                    color: AppTheme.fg
                    font.pixelSize: AppTheme.fontSize
                }
                Text {
                    text: "Ctrl+X - Toggle maximize"
                    color: AppTheme.fg
                    font.pixelSize: AppTheme.fontSize
                }
                Text {
                    text: "Ctrl+N - Toggle minimize"
                    color: AppTheme.fg
                    font.pixelSize: AppTheme.fontSize
                }
                Text {
                    text: "Ctrl+M - Move to next monitor"
                    color: AppTheme.fg
                    font.pixelSize: AppTheme.fontSize
                }
            }
            
            ColumnLayout {
                spacing: 8
                
                Text {
                    text: "↑/↓ - Navigate results"
                    color: AppTheme.fg
                    font.pixelSize: AppTheme.fontSize
                }
                Text {
                    text: "Esc - Close launcher"
                    color: AppTheme.fg
                    font.pixelSize: AppTheme.fontSize
                }
                Text {
                    text: "Ctrl+H - Toggle this help"
                    color: AppTheme.fg
                    font.pixelSize: AppTheme.fontSize
                }
            }
            
            Text {
                text: "Press Esc or click anywhere to close"
                color: Qt.darker(AppTheme.fg, 1.5)
                font.pixelSize: AppTheme.fontSize * 0.8
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 20
            }
        }
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
        onActivated: {
            outputSelector.visible = true
        }
    }
    
    OutputSelector {
        id: outputSelector
        anchors.fill: parent
        visible: false
        z: 100
        
        onOutputSelected: (outputName) => {
            Controller.moveWindowToOutput(resultsList.currentIndex, outputName)
            visible = false
        }
        
        onCancelled: {
            visible = false
            root.requestActivate() // Restore focus to main window
        }
    }
}


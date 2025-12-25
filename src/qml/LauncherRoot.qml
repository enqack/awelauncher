import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import awelauncher

Window {
    id: root
    width: AppTheme.windowWidth
    height: AppTheme.windowHeight
    minimumWidth: AppTheme.windowWidth
    maximumWidth: AppTheme.windowWidth
    minimumHeight: AppTheme.windowHeight
    maximumHeight: AppTheme.windowHeight
    x: (Screen.width - width) / 2
    y: (Screen.height - height) / 2
    visible: true
    title: "Awelauncher"
    color: AppTheme.bg
    
    // Wayland: Qt.Dialog hint often forces floating mode in tiling/scrolling WMs.
    // Wayland: Let WM handle decorations (no FramelessWindowHint)
    flags: Qt.Window | Qt.WindowStaysOnTopHint

    onActiveChanged: {
        if (!active) {
            Qt.quit()
        }
    }
    
    opacity: AppTheme.opacity

    Component.onCompleted: {
        root.requestActivate()
    }
    
    property bool showHelp: false
    
    
    // Background wrapper for rounded corners on the whole window content
    Rectangle {
        id: bg
        anchors.fill: parent
        color: AppTheme.bg
        radius: AppTheme.radius
        // Removed manual border to rely on WM focus border

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
                }
                Item { Layout.fillWidth: true }
                Text {
                    Layout.fillWidth: true
                    Layout.maximumWidth: parent.width * 0.6
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
    }
    
    // Help overlay
    Rectangle {
        anchors.fill: parent
        visible: showHelp
        color: Qt.rgba(0, 0, 0, 0.9)
        
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
            var outputs = Controller.getOutputs();
            if (outputs.length > 1) {
                // Cycle to next output (simple implementation)
                Controller.moveWindowToOutput(resultsList.currentIndex, outputs[0]);
            }
        }
    }
}

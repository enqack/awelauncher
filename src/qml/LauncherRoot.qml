import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import awelauncher

Window {
    id: root
    width: 600
    height: 400
    minimumWidth: 600
    maximumWidth: 600
    minimumHeight: 400
    maximumHeight: 400
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
    
    Keys.onEscapePressed: {
        Qt.quit()
    }

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
                    text: "Enter to select  •  Esc to close"
                    color: Qt.darker(AppTheme.fg, 1.5)
                    font.pixelSize: AppTheme.fontSize * 0.7
                }
            }
        }
    }
    
    // Close on Escape
    Shortcut {
        sequence: "Esc"
        onActivated: Qt.quit()
    }
}

import QtQuick
import QtQuick.Layouts
import awelauncher

Rectangle {
    anchors.fill: parent
    visible: false
    color: Qt.rgba(AppTheme.bg.r, AppTheme.bg.g, AppTheme.bg.b, 0.9)
    radius: AppTheme.radius
    border.color: AppTheme.accent
    border.width: AppTheme.borderWidth
    
    // property bool visible is inherited from Item/Rectangle
    property string showMode: "drun"
    
    signal closeRequested()
    
    MouseArea {
        anchors.fill: parent
        onClicked: closeRequested()
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
            visible: showMode === "window"
            
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
                text: "Ctrl+M - Move to monitor (select window first)"
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

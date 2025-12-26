import QtQuick
import QtQuick.Layouts
import awelauncher

RowLayout {
    Layout.fillWidth: true
    
    property int resultCount: 0
    property string showMode: "drun"
    
    Text {
        text: resultCount + " results"
        color: Qt.darker(AppTheme.fg, 1.5)
        font.pixelSize: AppTheme.fontSize * 0.7
        Layout.preferredWidth: implicitWidth 
    }
    Item { Layout.fillWidth: true }
    Text {
        Layout.maximumWidth: AppTheme.windowWidth * 0.7
        text: {
            if (showMode === "window") return "Ctrl+D Close • Ctrl+M Move • Ctrl+H Help"
            if (showMode === "run" || showMode === "drun") return "Shift+Enter Term • Ctrl+Enter Hold • Enter Run"
            return "Enter Select • Esc Close"
        }
        color: Qt.darker(AppTheme.fg, 1.5)
        font.pixelSize: AppTheme.fontSize * 0.7
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignRight
    }
}

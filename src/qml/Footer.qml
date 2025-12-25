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
        Layout.maximumWidth: AppTheme.windowWidth * 0.6
        text: showMode === "window" ? "Ctrl+H for help" : "Enter to select • Esc to close"
        color: Qt.darker(AppTheme.fg, 1.5)
        font.pixelSize: AppTheme.fontSize * 0.7
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignRight
    }
}

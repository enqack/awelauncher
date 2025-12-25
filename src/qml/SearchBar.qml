import QtQuick
import QtQuick.Layouts
import awelauncher

Rectangle {
    Layout.fillWidth: true
    height: AppTheme.rowHeight
    color: Qt.lighter(AppTheme.bg, 1.2)
    radius: AppTheme.radius
    
    signal closeRequested()
    signal navigateDown()
    signal navigateUp()
    signal activateCurrent()
    signal searchChanged(string text)
    
    property alias text: searchInput.text
    property alias inputFocus: searchInput.focus
    
    function forceInputFocus() {
        searchInput.forceActiveFocus()
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: AppTheme.padding / 2
        spacing: AppTheme.padding / 2
        
        Text { 
            text: "" 
            color: AppTheme.muted ? AppTheme.muted : "#888" 
            font.pixelSize: AppTheme.fontSize
        }

        // Search Input
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                text: (Controller && Controller.promptOverride && Controller.promptOverride !== "") ? Controller.promptOverride : cliPrompt
                color: AppTheme.muted
                font.pixelSize: AppTheme.fontSize
                visible: searchInput.text === ""
            }

            TextInput {
                id: searchInput
                anchors.fill: parent
                verticalAlignment: TextInput.AlignVCenter
                font.pixelSize: AppTheme.fontSize
                color: AppTheme.fg
                focus: true
                selectByMouse: true
                
                onTextChanged: {
                    Controller.filter(text)
                    searchChanged(text)
                }
                
                Keys.onPressed: (event) => {
                    if (event.key === Qt.Key_Escape) {
                        closeRequested()
                    } else if (event.key === Qt.Key_Down) {
                        navigateDown()
                        event.accepted = true
                    } else if (event.key === Qt.Key_Up) {
                        navigateUp()
                        event.accepted = true
                    } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                        activateCurrent()
                        event.accepted = true
                    }
                }
                
                Connections {
                    target: Controller
                    function onClearSearch() {
                        searchInput.text = ""
                    }
                }
            }
        }
    }
}

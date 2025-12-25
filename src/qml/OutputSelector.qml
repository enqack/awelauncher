import QtQuick
import QtQuick.Layouts
import QtQuick.Controls.Basic
import awelauncher

Rectangle {
    id: root
    color: Qt.rgba(0, 0, 0, 0.85)
    
    signal outputSelected(string outputName)
    signal cancelled()
    
    property var outputs: []
    property int currentIndex: 0
    
    onVisibleChanged: {
        if (visible) {
            outputs = Controller.getOutputs()
            currentIndex = 0
            list.forceActiveFocus()
        }
    }
    
    MouseArea {
        anchors.fill: parent
        onClicked: root.cancelled()
    }
    
    Frame {
        anchors.centerIn: parent
        width: 400
        height: Math.min(outputs.length * AppTheme.rowHeight + 100, parent.height * 0.8)
        
        background: Rectangle {
            color: AppTheme.bg
            radius: AppTheme.radius
            border.color: AppTheme.border
            border.width: AppTheme.borderWidth
        }
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 0
            
            Text {
                text: "Move to Monitor"
                font.pixelSize: AppTheme.fontSize
                color: AppTheme.accent
                font.bold: true
                Layout.alignment: Qt.AlignHCenter
                Layout.margins: AppTheme.padding
            }
            
            ListView {
                id: list
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                model: root.outputs
                currentIndex: root.currentIndex
                
                delegate: Rectangle {
                    width: ListView.view.width
                    height: AppTheme.rowHeight
                    color: (ListView.isCurrentItem || ma.containsMouse) ? AppTheme.selected : "transparent"
                    radius: AppTheme.radius
                    
                    Text {
                        anchors.centerIn: parent
                        text: modelData
                        color: AppTheme.fg
                        font.pixelSize: AppTheme.fontSize
                    }
                    
                    MouseArea {
                        id: ma
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            root.currentIndex = index
                            root.outputSelected(modelData)
                        }
                    }
                }
                
                Keys.onDownPressed: incrementCurrentIndex()
                Keys.onUpPressed: decrementCurrentIndex()
                Keys.onEnterPressed: root.outputSelected(modelData[currentIndex])
                Keys.onReturnPressed: root.outputSelected(modelData[currentIndex])
                Keys.onEscapePressed: root.cancelled()
            }
        }
    }
}

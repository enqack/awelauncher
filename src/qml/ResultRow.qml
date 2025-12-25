import QtQuick
import QtQuick.Layouts
import awelauncher

Item {
    id: delegateRoot
    height: AppTheme.rowHeight
    
    // Mouse hover handling could go here
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 12
        
        Image {
            id: icon
            Layout.preferredWidth: AppTheme.iconSize
            Layout.preferredHeight: AppTheme.iconSize
            source: "image://icons/" + model.iconKey
            asynchronous: true
            cache: true
            fillMode: Image.PreserveAspectFit
        }
        
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 2
            
            Text {
                function highlightMatches(text, positions) {
                    if (!positions || positions.length === 0) {
                        return text;
                    }
                    
                    let result = "";
                    let posSet = new Set(positions);
                    
                    for (let i = 0; i < text.length; i++) {
                        if (posSet.has(i)) {
                            result += "<span style='color: " + AppTheme.accent + ";'>" + text[i] + "</span>";
                        } else {
                            result += text[i];
                        }
                    }
                    return result;
                }
                
                text: highlightMatches(model.primary, model.matchPositions)
                textFormat: Text.RichText
                color: AppTheme.fg
                font.pixelSize: AppTheme.fontSize
                elide: Text.ElideRight
                Layout.fillWidth: true
            }
            
            Text {
                text: model.secondary
                color: Qt.darker(AppTheme.fg, 1.3)
                font.pixelSize: AppTheme.fontSize * 0.8
                elide: Text.ElideRight
                Layout.fillWidth: true
                visible: text !== ""
            }
        }
    }
    
    MouseArea {
        anchors.fill: parent
        onClicked: {
            ListView.view.currentIndex = index
            Controller.activate(index)
        }
    }
}

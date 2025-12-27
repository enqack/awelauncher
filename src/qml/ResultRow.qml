import QtQuick
import QtQuick.Layouts
import awelauncher

Item {
    id: delegateRoot
    height: AppTheme.rowHeight
    
    // Mouse hover handling could go here
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: AppTheme.padding / 2
        anchors.rightMargin: AppTheme.padding / 2
        spacing: AppTheme.padding / 2
        
        Image {
            id: icon
            Layout.preferredWidth: AppTheme.iconSize
            Layout.preferredHeight: AppTheme.iconSize
            source: "image://icon/" + model.iconKey
            asynchronous: true
            cache: true
            fillMode: Image.PreserveAspectFit
            smooth: true
            mipmap: true
        }
        
        ColumnLayout {
            Layout.fillWidth: true
            spacing: AppTheme.padding / 8
            
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
                font.pixelSize: AppTheme.secondaryFontSize
                elide: Text.ElideRight
                Layout.fillWidth: true
                visible: text !== ""
            }
        }
    }
    
    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (ListView.view) ListView.view.currentIndex = index
            launcher.activate(index)
        }
    }
}

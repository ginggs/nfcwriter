import QtQuick 1.0
import com.nokia.meego 1.0

Page {
    id: pageRead
    Label {
        anchors.centerIn: parent
        text: qsTr("Not Yet<br> Implemented")
        scale: 3
    }
    tools: ToolBarLayout {
        ToolIcon {
            iconId: "icon-m-toolbar-back";
            onClicked: pageStack.pop()
        }
    }
}

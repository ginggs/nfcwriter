import QtQuick 1.0
import com.nokia.meego 1.0

Page {
    id: pageRead
    Label {
        id: msg
        anchors.centerIn: parent
        text: qsTr("Tap")
        scale: 3
    }
    Connections {
        target: nfc
        onNfcRead: {
            msg.scale = 1;
            msg.text = nfc.get()
        }
        onNfcLost: {
            pageStack.pop(Qt.resolvedUrl("pageRead.qml"))
        }
    }
    tools: ToolBarLayout {
        ToolIcon {
            iconId: "icon-m-toolbar-back";
            onClicked: pageStack.pop()
        }
    }
}

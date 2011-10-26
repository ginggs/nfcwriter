import QtQuick 1.0
import com.nokia.meego 1.0

Page {
    orientationLock: PageOrientation.LockPrevious

    Column {
        anchors.centerIn: parent
        spacing: 10
        Label {
            text: qsTr("URI")
            scale: 2
        }
        TextField {
            id: uri
            text: qsTr("http://www.")
        }
        Button {
            text: qsTr("Write !")
            onClicked: nfc.writeuri(uri.text)
        }
    }
    tools: ToolBarLayout {
        ToolIcon {
            iconId: "icon-m-toolbar-back";
            onClicked: pageStack.pop()
        }
    }
    Connections {
        target: nfc
        onNfcTap: pageStack.push(Qt.resolvedUrl("pageTap.qml"))
        onNfcWritten: pageStack.push(Qt.resolvedUrl("pageWritten.qml"))
        onNfcLost: {
            pageStack.pop(Qt.resolvedUrl("pageWritten.qml"))
            pageStack.pop(Qt.resolvedUrl("pageTap.qml"))
            pageStack.pop(Qt.resolvedUrl("pageURI.qml"))
        }
    }
}

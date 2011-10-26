import QtQuick 1.0
import com.nokia.meego 1.0

Page {
    id: pageSmartPoster
    Column {
        anchors.centerIn: parent
        spacing: 10
        Label {
            text: qsTr("Smart Poster")
            scale: 2
        }
        TextField {
            id: text
            text: qsTr("Hello World !")
        }
        TextField {
            id: uri
            text: qsTr("http://www.")
        }
        Column {
            spacing: 10
            Button {
                text: qsTr("Write as DoAction")
                onClicked: nfc.writesp(text.text, uri.text,0)
            }
            Button {
                text: qsTr("Write as SaveForLater")
                onClicked: nfc.writesp(text.text, uri.text,1)
            }
            Button {
                text: qsTr("Write as OpenForEditing")
                onClicked: nfc.writesp(text.text, uri.text,2)
            }
            Button {
                text: qsTr("Write as RFU")
                onClicked: nfc.writesp(text.text, uri.text,3)
            }
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
            pageStack.pop(Qt.resolvedUrl("pageSmartPoster.qml"))
        }
    }
}

import QtQuick 1.0
import com.nokia.meego 1.0

Page {
    orientationLock: PageOrientation.LockPrevious
    property bool cloning: false
    Label {
        id: tap
        anchors.centerIn: parent
        text: qsTr("Tap your \n NFC TAG")
        scale: 3
    }
    Flickable {
        anchors.fill: parent
        anchors.margins: 10
        contentHeight: msg.height
        contentWidth: msg.width
        Label {
            id: msg
            anchors.centerIn: parent
        }
    }
    tools: ToolBarLayout {
        ToolIcon {
            id: toolbar
            visible: false
            iconId: "icon-m-toolbar-back";
            onClicked: pageStack.pop()
        }
        ToolButton {
            id: clone
            text: qsTr("Clone it")
            visible: false
            onClicked: {
                cloning = true;
                clone.visible = false
                toolbar.visible = false
                nfc.clone()
            }
        }
    }
    Connections {
        target: nfc
        onNfcRead: {
            tap.visible = false
            msg.text = nfc.get()
            clone.visible = true
            toolbar.visible = true
        }
        onNfcTap: if (cloning) tap.visible = true
        onNfcWritten: if (cloning) tap.text = qsTr("Written your \n NFC TAG")
        onNfcLost: if (cloning) pageStack.pop(Qt.resolvedUrl("pageRead.qml"))
    }
}

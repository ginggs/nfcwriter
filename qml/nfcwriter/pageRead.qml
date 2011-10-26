import QtQuick 1.0
import com.nokia.meego 1.0

Page {
    orientationLock: PageOrientation.LockPrevious
    Label {
        id: tap
        anchors.centerIn: parent
        text: qsTr("Tap")
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
    Connections {
        target: nfc
        onNfcRead: {
            tap.visible = false
            msg.text = nfc.get()
        }
        onNfcLost: {
            pageStack.pop(Qt.resolvedUrl("pageRead.qml"))
        }
    }
}

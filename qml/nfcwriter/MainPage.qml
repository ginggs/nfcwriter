import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    orientationLock: PageOrientation.LockPortrait

    Column {
        anchors.centerIn: parent
        spacing: 10
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("NFC Tag Writer")
            scale: 3
        }
        Button {
            text: qsTr("Read Tag")
            onClicked: {
                pageStack.push(Qt.resolvedUrl("pageRead.qml"))
                nfc.read()
            }
        }
        Button {
            text: qsTr("Write Text Tag")
            onClicked: pageStack.push(Qt.resolvedUrl("pageText.qml"))
        }
        Button {
            text: qsTr("Write URI Tag")
            onClicked: pageStack.push(Qt.resolvedUrl("pageURI.qml"))
        }
        Button {
            text: qsTr("Write VCard Tag")
            onClicked: pageStack.push(Qt.resolvedUrl("pageVCard.qml"))
        }
        Button {
            text: qsTr("Write Smart Poster Tag")
            onClicked: pageStack.push(Qt.resolvedUrl("pageSmartPoster.qml"))
        }
        Button {
            text: qsTr("Write Bluetooth Audio Tag")
            onClicked: pageStack.push(Qt.resolvedUrl("pageBlueTooth.qml"))
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("NFC Available: ") + (nfc.check() ? qsTr("Yes") : qsTr("NO"))
            scale: 2
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: (nfc.check() ? qsTr("") : qsTr("Enable NFC in Setting, Device, NFC"))
            scale: 2
        }
    }
}

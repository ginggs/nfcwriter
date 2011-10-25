import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    Column {
        anchors.centerIn: parent
        spacing: 10
        Button {
            text: qsTr("Read Tag")
            onClicked: pageStack.push(Qt.resolvedUrl("pageRead.qml"))
        }
        Button {
            text: qsTr("Write Text")
            onClicked: pageStack.push(Qt.resolvedUrl("pageText.qml"))
        }
        Button {
            text: qsTr("Write URI")
            onClicked: pageStack.push(Qt.resolvedUrl("pageURI.qml"))
        }
        Button {
            text: qsTr("Write VCard")
            onClicked: pageStack.push(Qt.resolvedUrl("pageVCard.qml"))
        }
        Button {
            text: qsTr("Write Smart Poster")
            onClicked: pageStack.push(Qt.resolvedUrl("pageSmartPoster.qml"))
        }
        Text {
            text: qsTr("NFC Available: ") + (nfc.check() ? qsTr("Yes") : qsTr("NO"))
            scale: 2
        }
    }
}

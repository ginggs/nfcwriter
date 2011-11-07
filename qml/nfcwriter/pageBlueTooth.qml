import QtQuick 1.0
import com.nokia.meego 1.0
import QtMobility.connectivity 1.2

Page {
    orientationLock: PageOrientation.LockPrevious
    ListView {
        id: blueList
        anchors.fill: parent
        model: BluetoothDiscoveryModel {
            id: blueModel            
            onDiscoveryChanged: {
                help.visible = discovery;
                reset.visible = !discovery
            }
        }
        delegate: Item {
            height: name.height * 3 * 1.5 + blueline.height
            width: parent.width
            Rectangle {
                id: blueline
                width: parent.width
                height: 2
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
            }
            Text {
                id: name
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                scale: 3
                text: service.deviceName
            }
            MouseArea {
                anchors.fill: parent
                onClicked: nfc.writebt(service.deviceAddress)
                onEntered: name.color = "red"
                onExited: name.color = "black"
                onCanceled: name.color = "black"
            }
        }
    }
    Label {
        id: help
        anchors.centerIn: parent
        text: qsTr("Put your \n bluetooth device \n in discoverable \n or visible mode")
        scale: 2
    }
    Button {
        id: reset
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        text: qsTr("Search again")
        onClicked: blueModel.setDiscovery(true)
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
            pageStack.pop(Qt.resolvedUrl("pageBlueTooth.qml"))
        }
    }
}

import QtQuick 1.0
import com.nokia.meego 1.0
import QtMobility.contacts 1.1

Page {
    orientationLock: PageOrientation.LockPrevious

    ListView {
        id:  contactList
        anchors.fill: parent
        model: ContactModel {
            sortOrders: [
                SortOrder {
                    detail:ContactDetail.Name
                    field:Name.FirstName
                    direction:Qt.AscendingOrder
                },
                SortOrder {
                    detail:ContactDetail.Name
                    field:Name.LastName
                    direction:Qt.AscendingOrder
                },
                SortOrder {
                    detail:ContactDetail.DisplayLabel
                    field:DisplayLabel
                    direction:Qt.AscendingOrder
                }
            ]
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
                text: model.contact.displayLabel
            }
            MouseArea {
                anchors.fill: parent
                onClicked: nfc.writeid(model.contact.contactId )
                onEntered: name.color = "red"
                onExited: name.color = "black"
                onCanceled: name.color = "black"
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
            pageStack.pop(Qt.resolvedUrl("pageVCard.qml"))
        }
    }
}

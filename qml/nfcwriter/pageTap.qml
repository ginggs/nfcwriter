import QtQuick 1.0
import com.nokia.meego 1.0

Page {
    orientationLock: PageOrientation.LockPrevious

    Label {
        anchors.centerIn: parent
        text: qsTr("Tap your \n NFC TAG")
        scale: 3
    }
    tools: ToolBarLayout {
        ToolButton {
            text: qsTr("Cancel")
            onClicked: {
                nfc.cancel()
                pageStack.pop(Qt.resolvedUrl("pageTap.qml"))
                pageStack.pop()
            }
        }
    }
}

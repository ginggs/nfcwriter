import QtQuick 1.0
import com.nokia.meego 1.0

Page {
    orientationLock: PageOrientation.LockPrevious

    Label {
        anchors.centerIn: parent
        text: qsTr("Written your \n NFC TAG")
        scale: 3
    }
}

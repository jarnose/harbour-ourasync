import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components/Format.js" as Format

Page {
    id: page

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width

            PageHeader { title: qsTr("Settings") }

            SectionHeader { text: qsTr("About you") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Used only for the sleep score's age bands and for the calorie estimate. "
                           + "It is not sent to the ring and does not leave the phone.")
            }

            TextField {
                id: ageField
                width: parent.width
                label: qsTr("Age, years")
                text: AppController.profileAge
                inputMethodHints: Qt.ImhDigitsOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: weightField.focus = true
                onActiveFocusChanged: if (!activeFocus) page.saveProfile()
            }
            TextField {
                id: weightField
                width: parent.width
                label: qsTr("Weight, kg")
                text: AppController.profileWeight
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                EnterKey.onClicked: focus = false
                onActiveFocusChanged: if (!activeFocus) page.saveProfile()
            }

            Item { width: 1; height: Theme.paddingLarge }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Oura cloud and data export")
                onClicked: pageStack.push(Qt.resolvedUrl("CloudPage.qml"))
            }

            SectionHeader {
                visible: AppController.hasRing
                text: qsTr("Ring")
            }
            DetailItem {
                visible: AppController.hasRing
                label: qsTr("Name")
                value: AppController.ringName
            }
            DetailItem {
                visible: AppController.hasRing && AppController.ringSerial.length > 0
                label: qsTr("Serial")
                value: AppController.ringSerial
            }
            DetailItem {
                visible: AppController.hasRing && AppController.ringFirmware.length > 0
                label: qsTr("Firmware")
                value: AppController.ringFirmware
            }
            DetailItem {
                visible: AppController.hasRing && AppController.batteryPercent >= 0
                label: qsTr("Battery")
                value: AppController.batteryPercent + " %"
            }
            DetailItem {
                visible: AppController.hasRing && AppController.lastSyncMs > 0
                label: qsTr("Last sync")
                value: Qt.formatDateTime(new Date(AppController.lastSyncMs), "ddd d.M. hh:mm")
            }

            Item { width: 1; height: Theme.paddingMedium; visible: AppController.hasRing }

            Button {
                visible: AppController.hasRing
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Back up the key")
                onClicked: pageStack.push(Qt.resolvedUrl("KeyPage.qml"))
            }
            Item { width: 1; height: Theme.paddingMedium; visible: AppController.hasRing }
            Button {
                visible: AppController.hasRing
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Diagnostics")
                onClicked: pageStack.push(Qt.resolvedUrl("DiagnosticsPage.qml"))
            }
            Item { width: 1; height: Theme.paddingMedium; visible: AppController.hasRing }

            ButtonLayout {
                visible: AppController.hasRing
                Button {
                    id: forgetButton
                    text: qsTr("Forget this ring")
                    onClicked: remorse.execute(forgetButton, qsTr("Forgetting the ring and its history"),
                                               function () {
                                                   AppController.forgetRing()
                                                   pageStack.pop()
                                               }, 6000)
                }
            }
            RemorseItem { id: remorse }

            Label {
                visible: AppController.hasRing
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("Forgetting removes the stored history and the ring's key from this phone. The ring "
                           + "keeps its key, so back it up first if you may want to read the ring again.")
            }
        }

        VerticalScrollDecorator {}
    }

    function saveProfile() {
        var age = parseInt(ageField.text, 10)
        var weight = parseFloat(weightField.text.replace(",", "."))
        if (!isNaN(age) && !isNaN(weight))
            AppController.setProfile(age, weight)
    }
}

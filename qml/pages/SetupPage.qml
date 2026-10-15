import QtQuick 2.0
import Sailfish.Silica 1.0

// Finds rings and sets one up. Two ways in:
//  * a factory-reset ring: this app installs a new key and keeps it;
//  * a ring that already has a key: the user supplies it.
// Everything that talks to the ring happens in AppController; this page only
// chooses the ring and shows progress.
Page {
    id: page

    property bool importKey: false
    property string chosenPath: ""
    property string chosenName: ""

    // Set when this run installed a fresh key: the user must then be shown it.
    property bool keyWasInstalled: false

    Component.onCompleted: AppController.startScan()
    Component.onDestruction: AppController.stopScan()

    Connections {
        target: AppController
        onKeyInstalled: page.keyWasInstalled = true
        onOperationFinished: {
            if (!ok)
                return
            // A new key exists nowhere but here and the vault: show it once, so
            // it can be backed up before anything can go wrong.
            if (page.keyWasInstalled)
                pageStack.replace(Qt.resolvedUrl("KeyPage.qml"), { fresh: true })
            else
                pageStack.pop()
        }
    }

    function begin(path) {
        AppController.clearError()
        if (importKey)
            AppController.useExistingKey(path, keyField.text)
        else
            AppController.setUpNewRing(path)
    }

    SilicaListView {
        id: list
        anchors.fill: parent
        model: AppController.devices
        enabled: !AppController.busy

        header: Column {
            width: list.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Set up ring")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("A ring has one owner. To use it here it must be factory reset first: remove it "
                           + "from the Oura app, then reset it (on a Gen 3 or 4 dock: place the ring, wait two "
                           + "seconds, flip the dock upside down, up, down and up again, waiting for the light "
                           + "to turn blue, red, magenta and yellow). Do not finish the Oura app's set-up "
                           + "afterwards: that installs Oura's own key and locks this app out.")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("After set-up the Oura app can no longer use this ring until it is reset again.")
            }

            TextSwitch {
                id: importSwitch
                text: qsTr("My ring already has a key")
                description: qsTr("Use this after reinstalling the app, with the key you saved")
                checked: page.importKey
                automaticCheck: false
                onClicked: page.importKey = !page.importKey
            }

            TextField {
                id: keyField
                visible: page.importKey
                width: parent.width
                label: qsTr("Key, 32 hexadecimal characters")
                placeholderText: label
                font.family: "monospace"
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                EnterKey.onClicked: focus = false
            }

            SectionHeader {
                text: AppController.scanning ? qsTr("Looking for rings…") : qsTr("Rings nearby")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                visible: AppController.devices.length === 0
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("None yet. Keep the ring close to the phone and make sure no other phone "
                           + "is connected to it. A factory-reset ring starts advertising once the "
                           + "reset has finished.")
            }

            // Progress and errors of the operation in flight.
            Column {
                width: parent.width
                visible: AppController.busy
                spacing: Theme.paddingSmall
                BusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    size: BusyIndicatorSize.Medium
                    running: AppController.busy
                }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    color: Theme.secondaryHighlightColor
                    text: AppController.status
                }
            }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                visible: AppController.lastError.length > 0 && !AppController.busy
                wrapMode: Text.Wrap
                color: Theme.errorColor
                font.pixelSize: Theme.fontSizeSmall
                text: AppController.lastError
            }
        }

        delegate: ListItem {
            id: item
            contentHeight: Theme.itemSizeMedium
            enabled: !page.importKey || keyField.text.length >= 32

            onClicked: {
                page.chosenPath = modelData.path
                page.chosenName = modelData.name
                if (page.importKey)
                    page.begin(modelData.path)
                else
                    item.remorseAction(qsTr("Setting up %1").arg(modelData.name),
                                       function () { page.begin(modelData.path) }, 4000)
            }

            Column {
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: signal.left
                    rightMargin: Theme.paddingMedium
                    verticalCenter: parent.verticalCenter
                }
                Label {
                    width: parent.width
                    text: modelData.name
                    color: item.highlighted ? Theme.highlightColor : Theme.primaryColor
                    truncationMode: TruncationMode.Fade
                }
                Label {
                    width: parent.width
                    text: modelData.address + (modelData.paired ? " · " + qsTr("paired") : "")
                          + (modelData.known ? " · " + qsTr("this ring") : "")
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                    truncationMode: TruncationMode.Fade
                }
            }
            Label {
                id: signal
                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                visible: modelData.rssi !== 0
                text: modelData.rssi + " dBm"
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
            }
        }

        VerticalScrollDecorator {}
    }
}

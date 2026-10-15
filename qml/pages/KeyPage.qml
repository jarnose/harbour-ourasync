import QtQuick 2.0
import Sailfish.Silica 1.0

// The ring's app-auth key, shown for backup.
//
// This is the one secret in the system. It lives in the Sailfish Secrets vault
// and nowhere else, which means that if the app data or the phone is lost, so is
// the way into the ring: the only way back is a factory reset, and a reset
// erases the ring's stored history. A copy in a password manager is the cheap
// insurance, hence this page.
Page {
    id: page

    // True right after set-up, when the user has not yet had a chance to save it.
    property bool fresh: false
    property string key: ""
    property bool shown: false

    function formatted(hex) {
        // 8f3a… grouped in fours, easier to read back and to type
        var out = []
        for (var i = 0; i < hex.length; i += 4)
            out.push(hex.substr(i, 4))
        return out.join(" ")
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                title: page.fresh ? qsTr("Ring is ready") : qsTr("Ring key")
            }

            Label {
                visible: page.fresh
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.highlightColor
                text: qsTr("The ring is set up and its first history is on its way into the app.")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Your ring is protected by a key that only this phone knows. It is stored in the "
                           + "secure vault. If you lose this phone or its data you will need the key to read "
                           + "the ring again; without it the only way back is a factory reset, which erases "
                           + "the ring's stored history. Save a copy somewhere safe, such as a password manager.")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: !page.shown
                text: qsTr("Show key")
                onClicked: {
                    page.key = AppController.keyHex()
                    page.shown = true
                }
            }

            Label {
                visible: page.shown
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WrapAnywhere
                horizontalAlignment: Text.AlignHCenter
                font.family: "monospace"
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.primaryColor
                text: page.key.length > 0 ? page.formatted(page.key) : qsTr("The vault did not return a key.")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: page.shown && page.key.length > 0
                text: qsTr("Copy")
                onClicked: Clipboard.text = page.key
            }

            Label {
                visible: page.shown && page.key.length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("Anyone with this key who can pair with the ring can read it. Clear the "
                           + "clipboard after pasting it somewhere safe.")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: page.fresh
                text: qsTr("Done")
                onClicked: pageStack.pop()
            }
        }

        VerticalScrollDecorator {}
    }
}

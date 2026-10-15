import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components/Format.js" as Format

// Everything that leaves the app, and the backup that brings it back. Files go to the
// OuraSync folder in Documents; nothing is sent anywhere.
Page {
    id: page

    property string result: ""
    property var backupList: []

    function reload() { backupList = AppController.backups() }
    Component.onCompleted: reload()

    function run(kind) {
        AppController.clearError()
        var r = AppController.exportFiles(kind)
        result = r
        reload()
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width

            PageHeader { title: qsTr("Export and backup") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("The files are written to %1 and go nowhere else.").arg(AppController.exportDir())
            }

            SectionHeader { text: qsTr("For other programs") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("FHIR is the standard other health systems read: one file with every night and day. "
                           + "CSV opens in a spreadsheet. Oura's own scores and this app's estimates are kept "
                           + "apart in both.")
            }
            ButtonLayout {
                Button {
                    text: qsTr("Export FHIR")
                    enabled: !AppController.busy
                    onClicked: page.run("fhir")
                }
                Button {
                    text: qsTr("Export CSV")
                    enabled: !AppController.busy
                    onClicked: page.run("csv")
                }
            }

            SectionHeader { text: qsTr("Backup") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("A backup is the whole database: the ring's history, what was imported from Oura and "
                           + "your settings. The ring's key is not in it; it stays in the secrets vault, so "
                           + "back that up separately (Settings, Back up the key).")
            }
            ButtonLayout {
                Button {
                    text: qsTr("Back up now")
                    enabled: !AppController.busy
                    onClicked: page.run("backup")
                }
            }

            Label {
                visible: page.result.length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WrapAnywhere
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: page.result
            }
            Label {
                visible: AppController.lastError.length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.errorColor
                font.pixelSize: Theme.fontSizeSmall
                text: AppController.lastError
            }

            SectionHeader {
                visible: page.backupList.length > 0
                text: qsTr("Restore")
            }
            Label {
                visible: page.backupList.length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Tap a backup to restore it. It replaces what the app holds now, and takes effect "
                           + "when the app is started again.")
            }
            Repeater {
                model: page.backupList
                delegate: ListItem {
                    id: item
                    width: column.width
                    contentHeight: Theme.itemSizeMedium
                    onClicked: remorseAction(qsTr("Restoring %1").arg(modelData.name), function () {
                        if (AppController.stageRestore(modelData.path))
                            page.result = qsTr("Restore staged. Close the app and open it again.")
                    }, 6000)
                    Column {
                        anchors {
                            left: parent.left
                            leftMargin: Theme.horizontalPageMargin
                            right: parent.right
                            rightMargin: Theme.horizontalPageMargin
                            verticalCenter: parent.verticalCenter
                        }
                        Label {
                            width: parent.width
                            text: Qt.formatDateTime(new Date(modelData.modifiedMs), "ddd d.M.yyyy hh:mm")
                            color: item.highlighted ? Theme.highlightColor : Theme.primaryColor
                        }
                        Label {
                            width: parent.width
                            text: modelData.name + " · " + Math.round(modelData.size / 1048576 * 10) / 10 + " MB"
                            color: Theme.secondaryColor
                            font.pixelSize: Theme.fontSizeExtraSmall
                            truncationMode: TruncationMode.Fade
                        }
                    }
                }
            }
        }
        VerticalScrollDecorator {}
    }
}

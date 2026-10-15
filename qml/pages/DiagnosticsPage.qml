import QtQuick 2.0
import Sailfish.Silica 1.0

// What is actually in the database, by event type. For checking that a ring is
// delivering what the pages expect, and for bug reports: the counts say whether
// a missing chart is a decoder problem or simply a ring that never sent it.
Page {
    id: page

    property var stats: AppController.eventStats()

    SilicaListView {
        anchors.fill: parent
        model: page.stats

        header: Column {
            width: parent.width
            PageHeader { title: qsTr("Diagnostics") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.WrapAnywhere
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: AppController.databasePath()
            }
            SectionHeader { text: qsTr("Stored events by type") }
        }

        delegate: Item {
            width: parent.width
            height: Theme.itemSizeExtraSmall
            Label {
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 2 * Theme.horizontalPageMargin - countLabel.width
                font.pixelSize: Theme.fontSizeSmall
                truncationMode: TruncationMode.Fade
                text: "0x" + (modelData.tag < 16 ? "0" : "") + modelData.tag.toString(16) + "  " + modelData.name
            }
            Label {
                id: countLabel
                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.highlightColor
                text: modelData.count
            }
        }
        VerticalScrollDecorator {}
    }
}

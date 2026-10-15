import QtQuick 2.0
import Sailfish.Silica 1.0

Page {
    id: page

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader { title: qsTr("About Oura Sync") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                text: qsTr("Oura Sync reads an Oura ring directly over Bluetooth LE and shows what it "
                           + "recorded. It is an independent project: it is not made, endorsed or supported "
                           + "by Oura Health, and it is not a medical device.")
            }

            SectionHeader { text: qsTr("What you see, and where it comes from") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Measured by the ring: sleep stages and the sleep window, heart rate, heart rate "
                           + "variability, skin temperature, blood oxygen, breathing rate while asleep, and "
                           + "how hard you moved, minute by minute.\n\n"
                           + "Estimated by this app: the sleep score (from published sleep research), the "
                           + "temperature change against your own usual, active calories, and the hourly "
                           + "heart rate bands.\n\n"
                           + "Not available: Oura's own Sleep, Readiness and Activity scores, and step counts. "
                           + "Oura computes those on the phone with closed software, so a ring read "
                           + "directly does not carry them.")
            }

            SectionHeader { text: qsTr("Privacy") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("The app has no network permission. The history is kept in a database on this "
                           + "phone and the ring's key in the Sailfish secrets vault.")
            }

            SectionHeader { text: qsTr("Credits") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("The ring's protocol was reverse engineered by the open_oura project, and the "
                           + "sleep-score method and several decoders come from open_health, both by Thomas "
                           + "Marchand (MIT licence). The dashboards follow the ideas of Cracked Oura. "
                           + "The protocol details were checked against version 7.27.0 of Oura's Android app.")
            }
        }

        VerticalScrollDecorator {}
    }
}

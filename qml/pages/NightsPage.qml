import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components/Format.js" as Format

Page {
    id: page

    property var nights: []

    function reload() {
        nights = AppController.nights(90)
    }
    Component.onCompleted: reload()
    Connections {
        target: AppController
        onSummaryChanged: page.reload()
    }

    SilicaListView {
        anchors.fill: parent
        model: page.nights

        header: PageHeader {
            title: qsTr("Sleep")
            description: qsTr("%1 nights").arg(page.nights.length)
        }

        delegate: BackgroundItem {
            id: row
            height: Theme.itemSizeMedium
            onClicked: pageStack.push(Qt.resolvedUrl("NightPage.qml"), { date: modelData.date })

            Column {
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: scoreLabel.left
                    rightMargin: Theme.paddingMedium
                    verticalCenter: parent.verticalCenter
                }
                Label {
                    width: parent.width
                    text: Format.dayLabel(modelData.date) + (modelData.main ? "" : " · " + qsTr("nap"))
                    color: row.highlighted ? Theme.highlightColor : Theme.primaryColor
                }
                Label {
                    width: parent.width
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                    truncationMode: TruncationMode.Fade
                    text: Format.duration(modelData.asleepMin)
                          + (modelData.efficiency !== null && modelData.efficiency !== undefined
                             ? " · " + qsTr("%1 % efficiency").arg(Format.number(modelData.efficiency)) : "")
                          + " · " + Format.hm(modelData.startMs) + "–" + Format.hm(modelData.endMs)
                }
            }
            Label {
                id: scoreLabel
                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                font.pixelSize: Theme.fontSizeLarge
                color: Theme.highlightColor
                text: modelData.ouraScore !== null && modelData.ouraScore !== undefined ? modelData.ouraScore
                      : (modelData.score !== null && modelData.score !== undefined ? modelData.score : "")
            }
        }

        ViewPlaceholder {
            enabled: page.nights.length === 0
            text: qsTr("No nights yet")
            hintText: qsTr("Sync the ring after a night's sleep")
        }
        VerticalScrollDecorator {}
    }
}

import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components/Format.js" as Format

Page {
    id: page

    property var days: []

    function reload() {
        days = AppController.days(90)
    }
    Component.onCompleted: reload()
    Connections {
        target: AppController
        onSummaryChanged: page.reload()
    }

    SilicaListView {
        anchors.fill: parent
        model: page.days

        header: PageHeader {
            title: qsTr("Activity")
            description: qsTr("%1 days").arg(page.days.length)
        }

        delegate: BackgroundItem {
            id: row
            height: Theme.itemSizeMedium
            onClicked: pageStack.push(Qt.resolvedUrl("DayPage.qml"), { date: modelData.date })

            Column {
                anchors {
                    left: parent.left
                    leftMargin: Theme.horizontalPageMargin
                    right: kcal.left
                    rightMargin: Theme.paddingMedium
                    verticalCenter: parent.verticalCenter
                }
                Label {
                    width: parent.width
                    text: Format.dayLabel(modelData.date)
                    color: row.highlighted ? Theme.highlightColor : Theme.primaryColor
                }
                Label {
                    width: parent.width
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                    truncationMode: TruncationMode.Fade
                    text: (modelData.hrMin !== null && modelData.hrMin !== undefined
                           ? qsTr("Heart rate %1–%2 bpm").arg(Format.number(modelData.hrMin))
                             .arg(Format.number(modelData.hrMax)) : qsTr("No heart rate"))
                          + " · " + qsTr("%1 min active").arg(modelData.moderateMin + modelData.vigorousMin)
                }
            }
            Label {
                id: kcal
                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                color: Theme.highlightColor
                text: modelData.activeKcal > 0 ? qsTr("%1 kcal").arg(Format.number(modelData.activeKcal)) : ""
            }
        }

        ViewPlaceholder {
            enabled: page.days.length === 0
            text: qsTr("No activity yet")
            hintText: qsTr("Sync the ring after wearing it for a day")
        }
        VerticalScrollDecorator {}
    }
}

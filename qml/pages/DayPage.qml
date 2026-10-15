import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"
import "../components/Format.js" as Format

Page {
    id: page

    property string date: ""
    property var d: AppController.day(date)

    Connections {
        target: AppController
        onSummaryChanged: page.d = AppController.day(page.date)
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader { title: page.date.length > 0 ? Format.dayLabel(page.date) : "" }

            Flow {
                id: facts
                width: parent.width
                readonly property real cell: width / 3
                StatTile {
                    width: facts.cell
                    value: Format.number(page.d.activeKcal)
                    unit: "kcal"
                    caption: qsTr("Active, estimated")
                }
                StatTile {
                    width: facts.cell
                    value: Format.number(page.d.moderateMin + page.d.vigorousMin)
                    unit: "min"
                    caption: qsTr("Moderate or harder")
                }
                StatTile {
                    width: facts.cell
                    value: Format.number(page.d.lightMin)
                    unit: "min"
                    caption: qsTr("Light")
                }
                StatTile {
                    width: facts.cell
                    value: page.d.hrMin !== null && page.d.hrMin !== undefined ? Format.number(page.d.hrMin) : "–"
                    unit: "bpm"
                    caption: qsTr("Lowest heart rate")
                }
                StatTile {
                    width: facts.cell
                    value: page.d.hrMax !== null && page.d.hrMax !== undefined ? Format.number(page.d.hrMax) : "–"
                    unit: "bpm"
                    caption: qsTr("Highest heart rate")
                }
                StatTile {
                    width: facts.cell
                    value: page.d.avgMet !== null && page.d.avgMet !== undefined ? Format.number(page.d.avgMet, 1) : "–"
                    unit: "MET"
                    caption: qsTr("Average effort")
                }
            }

            SectionHeader { text: qsTr("Heart rate by hour (bpm)") }
            HourlyChart {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                hours: page.d.hr !== undefined ? page.d.hr : []
                barColor: "#ef6c6c"
            }

            SectionHeader { text: qsTr("Effort by hour (MET)") }
            HourlyChart {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                rangeMode: false
                hours: page.d.metByHour !== undefined ? page.d.metByHour : []
                barColor: Theme.highlightColor
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("The ring reports how hard you moved, minute by minute, as a MET value (1 is "
                           + "sitting still). It does not report steps. Active calories are an estimate: "
                           + "each minute above 1 MET counts (MET − 1) × your weight ÷ 60, with the "
                           + "weight from Settings. Heart rate bars show the 5th to 95th percentile of "
                           + "the hour, with a tick at the mean.")
            }
        }

        VerticalScrollDecorator {}
    }
}

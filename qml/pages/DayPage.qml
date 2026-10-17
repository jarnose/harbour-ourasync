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

            // Oura's own numbers for the day, from the cloud.
            Flow {
                id: official
                width: parent.width
                visible: page.d.steps !== null && page.d.steps !== undefined
                         || page.d.activityScore !== null && page.d.activityScore !== undefined
                         || page.d.readinessScore !== null && page.d.readinessScore !== undefined
                readonly property real cell: width / 3
                StatTile {
                    width: official.cell
                    visible: page.d.activityScore !== null && page.d.activityScore !== undefined
                    value: Format.number(page.d.activityScore)
                    caption: qsTr("Oura activity")
                }
                StatTile {
                    width: official.cell
                    visible: page.d.readinessScore !== null && page.d.readinessScore !== undefined
                    value: Format.number(page.d.readinessScore)
                    caption: qsTr("Oura readiness")
                }
                StatTile {
                    width: official.cell
                    visible: page.d.steps !== null && page.d.steps !== undefined
                    value: Format.number(page.d.steps)
                    caption: qsTr("Steps")
                }
                StatTile {
                    width: official.cell
                    visible: page.d.totalKcal !== null && page.d.totalKcal !== undefined
                    value: Format.number(page.d.totalKcal)
                    unit: "kcal"
                    caption: qsTr("Total burned")
                }
                StatTile {
                    width: official.cell
                    visible: page.d.temperatureDeviation !== null && page.d.temperatureDeviation !== undefined
                    value: Format.signed(page.d.temperatureDeviation, 2)
                    unit: "°C"
                    caption: qsTr("Temperature vs usual")
                }
            }

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

            // The app's own estimate, for a day Oura's cloud has no score for.
            readonly property bool hasEstimate: page.d.estimatedActivityScore !== null && page.d.estimatedActivityScore !== undefined
                                                && (page.d.activityScore === null || page.d.activityScore === undefined)
            StatTile {
                visible: parent.hasEstimate
                width: parent.width / 3
                value: "~" + Format.number(page.d.estimatedActivityScore)
                caption: qsTr("Activity (estimate)")
            }
            SectionHeader {
                visible: parent.hasEstimate
                text: qsTr("Activity, estimated contributors")
            }
            ContributorBars {
                visible: parent.hasEstimate
                items: page.d.estimatedActivityContributors !== undefined ? page.d.estimatedActivityContributors : []
            }
            Label {
                visible: parent.hasEstimate
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("Worked out from the ring's movement over this and the six days before, against "
                           + "your goal in Settings. Recovery time is taken as full. It is close to Oura's "
                           + "score but not the same.")
            }
            SectionHeader {
                visible: page.d.readinessContributors !== undefined && page.d.readinessContributors.length > 0
                text: qsTr("Readiness, Oura's contributors")
            }
            ContributorBars {
                visible: page.d.readinessContributors !== undefined && page.d.readinessContributors.length > 0
                items: page.d.readinessContributors !== undefined ? page.d.readinessContributors : []
            }
            SectionHeader {
                visible: page.d.activityContributors !== undefined && page.d.activityContributors.length > 0
                text: qsTr("Activity, Oura's contributors")
            }
            ContributorBars {
                visible: page.d.activityContributors !== undefined && page.d.activityContributors.length > 0
                items: page.d.activityContributors !== undefined ? page.d.activityContributors : []
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

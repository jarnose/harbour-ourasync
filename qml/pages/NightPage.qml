import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"
import "../components/Format.js" as Format

// One night, in the order a person asks about it: how long and how well, which
// stages, then what the body did (heart, HRV, breathing, temperature, oxygen).
Page {
    id: page

    property string date: ""
    property var n: AppController.night(date)
    readonly property bool hasStages: n.staged === true

    Connections {
        target: AppController
        onSummaryChanged: page.n = AppController.night(page.date)
    }

    function componentName(key) {
        if (key === "duration") return qsTr("Duration")
        if (key === "efficiency") return qsTr("Efficiency")
        if (key === "onset_latency") return qsTr("Falling asleep")
        if (key === "waso") return qsTr("Time awake in the night")
        if (key === "awakenings") return qsTr("Awakenings")
        if (key === "architecture") return qsTr("Deep and REM sleep")
        if (key === "physiology") return qsTr("Heart rate and HRV vs usual")
        return key
    }

    function scoreWord(score) {
        if (score >= 85) return qsTr("Optimal")
        if (score >= 70) return qsTr("Good")
        if (score >= 60) return qsTr("Fair")
        return qsTr("Pay attention")
    }

    function seriesYs(s) { return s && s.v ? s.v : [] }
    function seriesXs(s) { return s && s.t ? s.t : [] }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: page.date.length > 0 ? Format.dayLabel(page.date) : ""
                description: page.n.startMs !== undefined
                             ? Format.hm(page.n.startMs) + " – " + Format.hm(page.n.endMs) : ""
            }

            Label {
                visible: page.n.startMs === undefined
                x: Theme.horizontalPageMargin
                text: qsTr("No data for this night")
                color: Theme.secondaryColor
            }

            // ---- headline ----
            Item {
                visible: page.n.startMs !== undefined
                width: parent.width
                height: Math.max(gauge.height, headline.height) + Theme.paddingMedium

                ScoreGauge {
                    id: gauge
                    x: Theme.horizontalPageMargin
                    width: Theme.itemSizeHuge * 1.15
                    value: page.n.score !== null && page.n.score !== undefined ? page.n.score : -1
                    caption: qsTr("Sleep")
                }
                Column {
                    id: headline
                    anchors {
                        left: gauge.right
                        leftMargin: Theme.paddingLarge
                        right: parent.right
                        rightMargin: Theme.horizontalPageMargin
                        verticalCenter: gauge.verticalCenter
                    }
                    spacing: Theme.paddingSmall
                    Label {
                        width: parent.width
                        font.pixelSize: Theme.fontSizeLarge
                        text: Format.duration(page.n.asleepMin)
                        truncationMode: TruncationMode.Fade
                    }
                    Label {
                        width: parent.width
                        color: Theme.secondaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        text: page.n.asleepEstimated ? qsTr("asleep, estimated") : qsTr("asleep")
                    }
                    Label {
                        width: parent.width
                        color: Theme.secondaryColor
                        font.pixelSize: Theme.fontSizeSmall
                        text: qsTr("%1 in bed").arg(Format.duration(page.n.inBedMin))
                    }
                    Label {
                        width: parent.width
                        color: Theme.secondaryHighlightColor
                        font.pixelSize: Theme.fontSizeExtraSmall
                        text: page.n.score !== null && page.n.score !== undefined
                              ? page.scoreWord(page.n.score) : ""
                    }
                }
            }

            Flow {
                id: facts
                visible: page.n.startMs !== undefined
                width: parent.width
                readonly property real cell: width / 3
                StatTile {
                    width: facts.cell
                    visible: page.n.efficiency !== null && page.n.efficiency !== undefined
                    value: Format.number(page.n.efficiency)
                    unit: "%"
                    caption: qsTr("Efficiency")
                }
                StatTile {
                    width: facts.cell
                    visible: page.n.latencyMin !== null && page.n.latencyMin !== undefined
                    value: Format.number(page.n.latencyMin)
                    unit: "min"
                    caption: qsTr("To fall asleep")
                }
                StatTile {
                    width: facts.cell
                    visible: page.n.wasoMin !== null && page.n.wasoMin !== undefined
                    value: Format.number(page.n.wasoMin)
                    unit: "min"
                    caption: qsTr("Awake in the night")
                }
                StatTile {
                    width: facts.cell
                    visible: page.n.awakenings !== null && page.n.awakenings !== undefined
                    value: Format.number(page.n.awakenings)
                    caption: qsTr("Awakenings")
                }
                StatTile {
                    width: facts.cell
                    visible: page.n.restingHr !== null && page.n.restingHr !== undefined
                    value: Format.number(page.n.restingHr)
                    unit: "bpm"
                    caption: qsTr("Lowest heart rate")
                }
                StatTile {
                    width: facts.cell
                    visible: page.n.hrv !== null && page.n.hrv !== undefined
                    value: Format.number(page.n.hrv)
                    unit: "ms"
                    caption: qsTr("HRV")
                }
            }

            // ---- stages ----
            SectionHeader {
                visible: page.hasStages
                text: qsTr("Sleep stages")
            }
            Hypnogram {
                visible: page.hasStages
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                stages: page.n.hypnogram !== undefined ? page.n.hypnogram : []
                startMs: page.n.hypnogramStartMs !== undefined ? page.n.hypnogramStartMs : 0
                epochSeconds: page.n.epochSeconds !== undefined ? page.n.epochSeconds : 30
            }
            Flow {
                id: stageFacts
                visible: page.hasStages
                width: parent.width
                readonly property real cell: width / 4
                StatTile { width: stageFacts.cell; value: Format.hoursMinutes(page.n.deepMin); unit: "h";  caption: qsTr("Deep") }
                StatTile { width: stageFacts.cell; value: Format.hoursMinutes(page.n.lightMin); unit: "h"; caption: qsTr("Light") }
                StatTile { width: stageFacts.cell; value: Format.hoursMinutes(page.n.remMin); unit: "h";   caption: qsTr("REM") }
                StatTile { width: stageFacts.cell; value: Format.hoursMinutes(page.n.awakeMin); unit: "h"; caption: qsTr("Awake") }
            }
            Label {
                visible: page.n.startMs !== undefined && !page.hasStages
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("The ring has not reported sleep stages for this night. Its own sleep window is shown, "
                           + "and the time asleep is estimated from it.")
            }

            // ---- body ----
            SectionHeader {
                visible: page.seriesYs(page.n.hrSeries).length > 0
                text: qsTr("Heart rate (bpm)")
            }
            LineChart {
                visible: page.seriesYs(page.n.hrSeries).length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                xs: page.seriesXs(page.n.hrSeries)
                ys: page.seriesYs(page.n.hrSeries)
                lineColor: "#ef6c6c"
            }

            SectionHeader {
                visible: page.seriesYs(page.n.hrvSeries).length > 0
                text: qsTr("Heart rate variability (ms)")
            }
            LineChart {
                visible: page.seriesYs(page.n.hrvSeries).length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                xs: page.seriesXs(page.n.hrvSeries)
                ys: page.seriesYs(page.n.hrvSeries)
                lineColor: "#7fd18b"
            }

            SectionHeader {
                visible: page.seriesYs(page.n.breathSeries).length > 0
                text: qsTr("Breathing (per minute)")
            }
            LineChart {
                visible: page.seriesYs(page.n.breathSeries).length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                xs: page.seriesXs(page.n.breathSeries)
                ys: page.seriesYs(page.n.breathSeries)
                decimals: 1
                lineColor: "#5ec2e6"
            }

            SectionHeader {
                visible: page.seriesYs(page.n.tempSeries).length > 0
                text: page.n.tempDeviation !== null && page.n.tempDeviation !== undefined
                      ? qsTr("Skin temperature (°C), %1 vs usual").arg(Format.signed(page.n.tempDeviation, 1))
                      : qsTr("Skin temperature (°C)")
            }
            LineChart {
                visible: page.seriesYs(page.n.tempSeries).length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                xs: page.seriesXs(page.n.tempSeries)
                ys: page.seriesYs(page.n.tempSeries)
                decimals: 1
                lineColor: "#f0a35e"
            }

            SectionHeader {
                visible: page.seriesYs(page.n.spo2Series).length > 0
                text: qsTr("Blood oxygen (%)")
            }
            LineChart {
                visible: page.seriesYs(page.n.spo2Series).length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                xs: page.seriesXs(page.n.spo2Series)
                ys: page.seriesYs(page.n.spo2Series)
                fixedMin: 85
                fixedMax: 100
                lineColor: "#b69bf0"
            }

            // ---- how the score came about ----
            SectionHeader {
                visible: page.n.components !== undefined && page.n.components.length > 0
                text: qsTr("How the score is made")
            }
            Repeater {
                model: page.n.components !== undefined ? page.n.components : []
                delegate: Item {
                    width: column.width
                    height: Theme.itemSizeSmall
                    Label {
                        x: Theme.horizontalPageMargin
                        width: parent.width * 0.55 - Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        text: page.componentName(modelData.key)
                        font.pixelSize: Theme.fontSizeSmall
                        truncationMode: TruncationMode.Fade
                    }
                    Rectangle {
                        id: track
                        x: parent.width * 0.55
                        width: parent.width * 0.45 - Theme.horizontalPageMargin - scoreText.width - Theme.paddingSmall
                        height: Theme.paddingMedium
                        radius: height / 2
                        anchors.verticalCenter: parent.verticalCenter
                        color: Theme.rgba(Theme.primaryColor, 0.15)
                        Rectangle {
                            width: parent.width * Math.max(0.02, Math.min(1, modelData.score / 100))
                            height: parent.height
                            radius: height / 2
                            color: Theme.highlightColor
                        }
                    }
                    Label {
                        id: scoreText
                        anchors {
                            right: parent.right
                            rightMargin: Theme.horizontalPageMargin
                            verticalCenter: parent.verticalCenter
                        }
                        font.pixelSize: Theme.fontSizeSmall
                        color: Theme.secondaryColor
                        text: Math.round(modelData.score)
                    }
                }
            }
            Label {
                visible: page.n.components !== undefined && page.n.components.length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("This score is this app's own estimate, built from published sleep research "
                           + "(National Sleep Foundation guidelines). Oura computes its score with closed "
                           + "software on the phone, so the two numbers will not match.")
            }
            Label {
                visible: page.n.approximateClock === true
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("The ring's clock could not be tied to the calendar for this night, so its times "
                           + "are placed from the moment of the download and may be off by hours.")
            }
        }

        VerticalScrollDecorator {}
    }
}

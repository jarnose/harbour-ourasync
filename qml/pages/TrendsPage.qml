import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"
import "../components/Format.js" as Format

Page {
    id: page

    readonly property var metrics: [
        { key: "score", name: qsTr("Sleep score"), unit: "", decimals: 0, color: "#8d9bf0", fixedMin: 0, fixedMax: 100 },
        { key: "asleep", name: qsTr("Time asleep"), unit: "h", decimals: 1, color: "#8d9bf0" },
        { key: "efficiency", name: qsTr("Sleep efficiency"), unit: "%", decimals: 0, color: "#8d9bf0" },
        { key: "deep", name: qsTr("Deep sleep"), unit: "min", decimals: 0, color: "#4a5bd0" },
        { key: "rem", name: qsTr("REM sleep"), unit: "min", decimals: 0, color: "#22b8cf" },
        { key: "restingHr", name: qsTr("Lowest heart rate"), unit: "bpm", decimals: 0, color: "#ef6c6c" },
        { key: "hrv", name: qsTr("HRV"), unit: "ms", decimals: 0, color: "#7fd18b" },
        { key: "tempDeviation", name: qsTr("Skin temperature vs usual"), unit: "°C", decimals: 1, color: "#f0a35e" },
        { key: "skinTemp", name: qsTr("Skin temperature"), unit: "°C", decimals: 1, color: "#f0a35e" },
        { key: "spo2", name: qsTr("Blood oxygen"), unit: "%", decimals: 1, color: "#b69bf0" },
        { key: "breath", name: qsTr("Breathing rate"), unit: "/min", decimals: 1, color: "#5ec2e6" }
    ]
    readonly property var counts: [14, 30, 90]

    property int metricIndex: 0
    property int countIndex: 1
    property var points: []

    function reload() {
        points = AppController.trend(metrics[metricIndex].key, counts[countIndex])
    }
    Component.onCompleted: reload()
    Connections {
        target: AppController
        onSummaryChanged: page.reload()
    }

    function stat(kind) {
        if (points.length === 0)
            return "–"
        var lo = Infinity, hi = -Infinity, sum = 0
        for (var i = 0; i < points.length; ++i) {
            lo = Math.min(lo, points[i].value)
            hi = Math.max(hi, points[i].value)
            sum += points[i].value
        }
        var v = kind === "min" ? lo : (kind === "max" ? hi : sum / points.length)
        return Format.number(v, metrics[metricIndex].decimals)
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader { title: qsTr("Trends") }

            ComboBox {
                label: qsTr("Measure")
                currentIndex: page.metricIndex
                menu: ContextMenu {
                    Repeater {
                        model: page.metrics
                        MenuItem { text: modelData.name }
                    }
                }
                onCurrentIndexChanged: {
                    page.metricIndex = currentIndex
                    page.reload()
                }
            }
            ComboBox {
                label: qsTr("Period")
                currentIndex: page.countIndex
                menu: ContextMenu {
                    MenuItem { text: qsTr("14 nights") }
                    MenuItem { text: qsTr("30 nights") }
                    MenuItem { text: qsTr("90 nights") }
                }
                onCurrentIndexChanged: {
                    page.countIndex = currentIndex
                    page.reload()
                }
            }

            LineChart {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: Theme.itemSizeHuge * 1.6
                ys: page.points.map(function (p) { return p.value })
                labels: page.points.map(function (p) { return Format.shortDay(p.date) })
                lineColor: page.metrics[page.metricIndex].color
                decimals: page.metrics[page.metricIndex].decimals
                fixedMin: page.metrics[page.metricIndex].fixedMin !== undefined
                          ? page.metrics[page.metricIndex].fixedMin : NaN
                fixedMax: page.metrics[page.metricIndex].fixedMax !== undefined
                          ? page.metrics[page.metricIndex].fixedMax : NaN
            }

            Flow {
                id: summary
                width: parent.width
                readonly property real cell: width / 3
                StatTile {
                    width: summary.cell
                    value: page.stat("min")
                    unit: page.metrics[page.metricIndex].unit
                    caption: qsTr("Lowest")
                }
                StatTile {
                    width: summary.cell
                    value: page.stat("avg")
                    unit: page.metrics[page.metricIndex].unit
                    caption: qsTr("Average")
                }
                StatTile {
                    width: summary.cell
                    value: page.stat("max")
                    unit: page.metrics[page.metricIndex].unit
                    caption: qsTr("Highest")
                }
            }

            Label {
                visible: page.points.length === 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                text: qsTr("Nothing to show for this measure yet.")
            }
        }

        VerticalScrollDecorator {}
    }
}

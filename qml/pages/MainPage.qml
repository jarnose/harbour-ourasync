import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"
import "../components/Format.js" as Format

// The overview. What it shows is whatever the last sync brought: the app never
// asks the ring for anything while this page is merely open, so a stale number
// here means the ring has not been synced, not that something failed.
Page {
    id: page

    property var overview: ({})
    readonly property var night: overview.night
    readonly property var today: overview.today
    // The three scores of the gauges: sleep, readiness, activity, each with its value, whether it is
    // Oura's own or this app's estimate, and the day it is for.
    readonly property var scores: overview.scores !== undefined ? overview.scores : ({})
    // The sleep score is Oura's own when the cloud has it.
    readonly property bool officialNight: scores.sleep !== undefined && scores.sleep.official

    // Bumped to make what depends on the time of day look again.
    property int clock: 0
    Timer {
        interval: 30000
        repeat: true
        running: page.status === PageStatus.Active
        onTriggered: page.clock++
    }
    onStatusChanged: if (status === PageStatus.Active) clock++

    function reload() {
        overview = AppController.overview()
    }

    Component.onCompleted: reload()

    Connections {
        target: AppController
        onSummaryChanged: page.reload()
        onRingChanged: page.reload()
    }

    // A score for a gauge: -1 when there is none.
    function scoreValue(one) {
        return one !== undefined ? one.value : -1
    }
    // Its caption; a "~" marks the app's own estimate where Oura's cloud has no score.
    function scoreCaption(name, one) {
        return one !== undefined && !one.official ? name + " ~" : name
    }

    function syncedText() {
        if (!AppController.hasRing)
            return overview.hasData ? qsTr("From Oura's cloud") : ""
        if (AppController.lastSyncMs <= 0)
            return qsTr("Not synced yet")
        var a = Format.ago(AppController.lastSyncMs)
        return a === "now" ? qsTr("Synced just now") : qsTr("Synced %1 ago").arg(a)
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        PullDownMenu {
            busy: AppController.busy
            MenuItem {
                text: qsTr("About")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
            MenuItem {
                text: qsTr("Settings")
                onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
            }
            MenuItem {
                text: qsTr("Oura cloud")
                onClicked: pageStack.push(Qt.resolvedUrl("CloudPage.qml"))
            }
            MenuItem {
                visible: !AppController.hasRing || !AppController.keyStored
                text: qsTr("Set up ring")
                onClicked: pageStack.push(Qt.resolvedUrl("SetupPage.qml"))
            }
            MenuItem {
                visible: AppController.hasRing
                enabled: !AppController.busy
                text: qsTr("Sync now")
                onClicked: AppController.syncNow()
            }
        }

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader {
                title: qsTr("Oura Sync")
                // The age of the last sync is a clock reading, so it is read again every half minute
                // and whenever the page is shown: a binding would not notice time passing.
                description: { page.clock; return page.syncedText() }
            }

            // ---- activity of the conversation with the ring ----
            Column {
                width: parent.width
                visible: AppController.busy
                spacing: Theme.paddingSmall
                BusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    size: BusyIndicatorSize.Medium
                    running: AppController.busy
                }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    horizontalAlignment: Text.AlignHCenter
                    color: Theme.secondaryHighlightColor
                    text: AppController.status
                    wrapMode: Text.Wrap
                }
            }

            Label {
                id: errorLabel
                visible: AppController.lastError.length > 0
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.errorColor
                font.pixelSize: Theme.fontSizeSmall
                text: AppController.lastError
                MouseArea {
                    anchors.fill: parent
                    onClicked: AppController.clearError()
                }
            }

            // ---- first run ----
            Column {
                width: parent.width
                visible: !AppController.hasRing && !overview.hasData
                spacing: Theme.paddingLarge

                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    color: Theme.highlightColor
                    font.pixelSize: Theme.fontSizeLarge
                    text: qsTr("Your ring, without the cloud")
                }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    color: Theme.secondaryColor
                    text: qsTr("Oura Sync reads your ring over Bluetooth and shows its sleep, heart rate, "
                               + "HRV, temperature and activity. There is no account and no subscription: "
                               + "the data stays on this phone. You can also bring in your Oura history "
                               + "from a data export.")
                }
                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Set up ring")
                    onClicked: pageStack.push(Qt.resolvedUrl("SetupPage.qml"))
                }
                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Import Oura history")
                    onClicked: pageStack.push(Qt.resolvedUrl("CloudPage.qml"))
                }
            }

            // ---- nothing synced yet ----
            Label {
                visible: AppController.hasRing && !overview.hasData && !AppController.busy
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                text: AppController.summaryBusy
                      ? qsTr("Reading the history…")
                      : qsTr("No history yet. Pull down and choose Sync now, with the ring nearby. "
                             + "The first sync downloads everything the ring holds, which takes a few minutes.")
            }

            // ---- last night ----
            Column {
                width: parent.width
                visible: night !== undefined
                spacing: Theme.paddingMedium

                SectionHeader {
                    text: night ? qsTr("Last night, %1").arg(Format.dayLabel(night.date)) : ""
                }

                Row {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    spacing: Theme.paddingLarge
                    readonly property real cell: (width - 2 * spacing) / 3

                    BackgroundItem {
                        width: parent.cell
                        height: sleepGauge.height
                        onClicked: pageStack.push(Qt.resolvedUrl("NightPage.qml"), { date: night.date })
                        ScoreGauge {
                            id: sleepGauge
                            width: parent.width
                            value: page.scoreValue(page.scores.sleep)
                            caption: page.scoreCaption(qsTr("Sleep"), page.scores.sleep)
                        }
                    }
                    BackgroundItem {
                        width: parent.cell
                        height: readinessGauge.height
                        enabled: page.scores.readiness !== undefined
                        onClicked: pageStack.push(Qt.resolvedUrl("DayPage.qml"), { date: page.scores.readiness.date })
                        ScoreGauge {
                            id: readinessGauge
                            width: parent.width
                            value: page.scoreValue(page.scores.readiness)
                            caption: page.scoreCaption(qsTr("Readiness"), page.scores.readiness)
                        }
                    }
                    BackgroundItem {
                        width: parent.cell
                        height: activityGauge.height
                        enabled: page.scores.activity !== undefined
                        onClicked: pageStack.push(Qt.resolvedUrl("DayPage.qml"), { date: page.scores.activity.date })
                        ScoreGauge {
                            id: activityGauge
                            width: parent.width
                            value: page.scoreValue(page.scores.activity)
                            caption: page.scoreCaption(qsTr("Activity"), page.scores.activity)
                        }
                    }
                }

                BackgroundItem {
                    width: parent.width
                    height: nightStats.height + Theme.paddingMedium * 2
                    onClicked: pageStack.push(Qt.resolvedUrl("NightPage.qml"), { date: night.date })

                    Column {
                        id: nightStats
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2 * Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: Theme.paddingSmall
                        Label {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            font.pixelSize: Theme.fontSizeLarge
                            text: night ? Format.duration(night.asleepMin) + " "
                                          + (night.asleepEstimated ? qsTr("asleep (estimated)") : qsTr("asleep")) : ""
                            truncationMode: TruncationMode.Fade
                        }
                        Label {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            color: Theme.secondaryColor
                            font.pixelSize: Theme.fontSizeSmall
                            text: {
                                if (!night)
                                    return ""
                                var t = Format.hm(night.startMs) + " – " + Format.hm(night.endMs)
                                if (night.efficiency !== null && night.efficiency !== undefined)
                                    t += " · " + qsTr("Efficiency %1 %").arg(Format.number(night.efficiency))
                                return t
                            }
                        }
                        Label {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            color: Theme.secondaryColor
                            font.pixelSize: Theme.fontSizeExtraSmall
                            visible: page.officialNight && night.score !== null && night.score !== undefined
                            text: night && night.score !== null && night.score !== undefined
                                  ? qsTr("This app's estimate: %1").arg(night.score) : ""
                        }
                        Label {
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter
                            color: Theme.secondaryColor
                            font.pixelSize: Theme.fontSizeExtraSmall
                            visible: page.scores.steps !== undefined
                            text: page.scores.steps !== undefined ? qsTr("%1 steps").arg(page.scores.steps) : ""
                        }
                    }
                }

                Flow {
                    id: vitals
                    width: parent.width
                    readonly property real cell: width / 3

                    StatTile {
                        width: vitals.cell
                        value: night ? Format.number(night.restingHr) : "–"
                        unit: "bpm"
                        caption: qsTr("Lowest heart rate")
                    }
                    StatTile {
                        width: vitals.cell
                        value: night ? Format.number(night.hrv) : "–"
                        unit: "ms"
                        caption: qsTr("HRV")
                    }
                    StatTile {
                        width: vitals.cell
                        value: night && night.tempDeviation !== null && night.tempDeviation !== undefined
                               ? Format.signed(night.tempDeviation, 1)
                               : (night ? Format.number(night.skinTemp, 1) : "–")
                        unit: "°C"
                        caption: night && night.tempDeviation !== null && night.tempDeviation !== undefined
                                 ? qsTr("Skin temp. vs usual") : qsTr("Skin temperature")
                    }
                    StatTile {
                        width: vitals.cell
                        value: night ? Format.number(night.spo2) : "–"
                        unit: "%"
                        caption: qsTr("Blood oxygen")
                    }
                    StatTile {
                        width: vitals.cell
                        value: night ? Format.number(night.breath, 1) : "–"
                        unit: "/min"
                        caption: qsTr("Breathing")
                    }
                    StatTile {
                        width: vitals.cell
                        value: overview.latestHr !== undefined ? Format.number(overview.latestHr) : "–"
                        unit: "bpm"
                        caption: overview.latestHrMs !== undefined
                                 ? qsTr("Heart rate, %1 ago").arg(Format.ago(overview.latestHrMs))
                                 : qsTr("Heart rate")
                    }
                }
            }

            // ---- today ----
            Column {
                width: parent.width
                visible: today !== undefined
                spacing: Theme.paddingMedium

                SectionHeader { text: qsTr("Today") }

                BackgroundItem {
                    width: parent.width
                    height: todayColumn.height + Theme.paddingMedium * 2
                    onClicked: pageStack.push(Qt.resolvedUrl("DayPage.qml"), { date: today.date })
                    Column {
                        id: todayColumn
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2 * Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        spacing: Theme.paddingSmall
                        Label {
                            color: Theme.secondaryColor
                            font.pixelSize: Theme.fontSizeSmall
                            text: qsTr("Heart rate by hour")
                        }
                        HourlyChart {
                            width: parent.width
                            hours: today ? today.hr : []
                            barColor: Theme.highlightColor
                        }
                        Label {
                            width: parent.width
                            color: Theme.secondaryColor
                            font.pixelSize: Theme.fontSizeSmall
                            wrapMode: Text.Wrap
                            text: today
                                  ? qsTr("About %1 kcal active · %2 min moderate or harder")
                                    .arg(Format.number(today.activeKcal))
                                    .arg(today.moderateMin + today.vigorousMin)
                                  : ""
                        }
                    }
                }
            }

            // ---- trend ----
            Column {
                width: parent.width
                visible: overview.nightCount !== undefined && overview.nightCount >= 2
                spacing: Theme.paddingMedium

                SectionHeader { text: qsTr("Sleep score, recent nights") }

                BackgroundItem {
                    width: parent.width
                    height: scoreTrend.height + Theme.paddingLarge
                    onClicked: pageStack.push(Qt.resolvedUrl("TrendsPage.qml"))
                    LineChart {
                        id: scoreTrend
                        x: Theme.horizontalPageMargin
                        width: parent.width - 2 * Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        property var points: AppController.trend("score", 14)
                        ys: points.map(function (p) { return p.value })
                        labels: points.map(function (p) { return Format.shortDay(p.date) })
                        fixedMin: 0
                        fixedMax: 100
                        Connections {
                            target: AppController
                            onSummaryChanged: scoreTrend.points = AppController.trend("score", 14)
                        }
                    }
                }
            }

            // ---- navigation ----
            Column {
                width: parent.width
                visible: AppController.hasRing || overview.hasData
                BackgroundItem {
                    width: parent.width
                    onClicked: pageStack.push(Qt.resolvedUrl("NightsPage.qml"))
                    Label {
                        x: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Sleep history")
                        color: parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                    }
                }
                BackgroundItem {
                    width: parent.width
                    onClicked: pageStack.push(Qt.resolvedUrl("DaysPage.qml"))
                    Label {
                        x: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Activity history")
                        color: parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                    }
                }
                BackgroundItem {
                    width: parent.width
                    onClicked: pageStack.push(Qt.resolvedUrl("TrendsPage.qml"))
                    Label {
                        x: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        text: qsTr("Trends")
                        color: parent.highlighted ? Theme.highlightColor : Theme.primaryColor
                    }
                }
            }

            Label {
                visible: AppController.hasRing
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: AppController.batteryPercent >= 0
                      ? qsTr("%1 · battery %2 % · firmware %3")
                        .arg(AppController.ringName).arg(AppController.batteryPercent)
                        .arg(AppController.ringFirmware)
                      : AppController.ringName
            }
        }
    }
}

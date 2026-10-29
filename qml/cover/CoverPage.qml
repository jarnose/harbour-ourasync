import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components/Format.js" as Format

CoverBackground {
    id: cover

    property var overview: ({})
    readonly property var scores: overview.scores !== undefined ? overview.scores : ({})
    readonly property var night: overview.night
    // What Settings has chosen to show.
    readonly property var chosen: AppController.coverItems

    function refresh() {
        overview = AppController.overview()
    }

    function shown(key) {
        return chosen.indexOf(key) >= 0
    }

    // The scores chosen, in the order sleep, readiness, activity: each a number and a small caption.
    readonly property var gauges: {
        var out = []
        var keys = [ "sleep", "readiness", "activity" ]
        var names = [ qsTr("Sleep"), qsTr("Readiness"), qsTr("Activity") ]
        for (var i = 0; i < keys.length; ++i) {
            var one = scores[keys[i]]
            if (shown(keys[i]) && one !== undefined)
                out.push({ value: (one.official ? "" : "~") + one.value, name: names[i] })
        }
        return out
    }

    // The other lines chosen.
    readonly property var lines: {
        var out = []
        if (shown("duration") && night)
            out.push(qsTr("Slept %1").arg(Format.duration(night.asleepMin)))
        if (shown("heartrate") && overview.latestHr !== undefined)
            out.push(Format.number(overview.latestHr) + " bpm")
        if (shown("lowest") && night && night.restingHr !== null && night.restingHr !== undefined)
            out.push(qsTr("Lowest %1 bpm").arg(Format.number(night.restingHr)))
        if (shown("hrv") && night && night.hrv !== null && night.hrv !== undefined)
            out.push(qsTr("HRV %1 ms").arg(Format.number(night.hrv)))
        if (shown("battery") && AppController.batteryPercent >= 0)
            out.push(qsTr("Ring %1 %").arg(AppController.batteryPercent))
        return out
    }

    Component.onCompleted: refresh()

    Connections {
        target: AppController
        onSummaryChanged: cover.refresh()
    }

    Label {
        id: title
        anchors {
            top: parent.top
            topMargin: Theme.paddingLarge
            horizontalCenter: parent.horizontalCenter
        }
        text: "Oura Sync"
        color: Theme.secondaryColor
        font.pixelSize: Theme.fontSizeExtraSmall
    }

    Column {
        anchors {
            centerIn: parent
            verticalCenterOffset: Theme.paddingMedium
        }
        width: parent.width - 2 * Theme.paddingMedium
        spacing: Theme.paddingSmall

        Row {
            width: parent.width
            visible: cover.gauges.length > 0
            Repeater {
                model: cover.gauges
                Column {
                    width: parent.width / cover.gauges.length
                    Label {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        // One score is big, three have to fit side by side.
                        font.pixelSize: cover.gauges.length === 1 ? Theme.fontSizeHuge
                                        : cover.gauges.length === 2 ? Theme.fontSizeExtraLarge : Theme.fontSizeLarge
                        fontSizeMode: Text.HorizontalFit
                        minimumPixelSize: Theme.fontSizeSmall
                        text: modelData.value
                    }
                    Label {
                        width: parent.width
                        horizontalAlignment: Text.AlignHCenter
                        color: Theme.secondaryColor
                        font.pixelSize: Theme.fontSizeTiny
                        truncationMode: TruncationMode.Fade
                        text: modelData.name
                    }
                }
            }
        }

        Repeater {
            model: cover.lines
            Label {
                width: parent.width
                horizontalAlignment: Text.AlignHCenter
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                truncationMode: TruncationMode.Fade
                text: modelData
            }
        }

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            visible: !overview.hasData
            wrapMode: Text.Wrap
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeSmall
            text: AppController.hasRing ? qsTr("No history yet") : qsTr("No ring set up")
        }
        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            visible: AppController.busy
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeExtraSmall
            wrapMode: Text.Wrap
            text: AppController.status
        }
    }

    CoverActionList {
        enabled: AppController.hasRing && !AppController.busy
        CoverAction {
            iconSource: "image://theme/icon-cover-sync"
            onTriggered: AppController.syncNow()
        }
    }
}

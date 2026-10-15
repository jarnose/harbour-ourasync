import QtQuick 2.0
import Sailfish.Silica 1.0

// Contributors of a score as labelled bars. `items` is [{key, value}] in 0-100.
// With `compare` (same shape) a second, dimmer number is shown beside each, which
// is how this app's estimate is set next to Oura's own.
Column {
    id: bars

    property var items: []
    property var compare: []
    property string compareLabel: ""

    function nameOf(key) {
        // Sleep
        if (key === "total_sleep") return qsTr("Total sleep")
        if (key === "deep_sleep") return qsTr("Deep sleep")
        if (key === "rem_sleep") return qsTr("REM sleep")
        if (key === "efficiency") return qsTr("Efficiency")
        if (key === "latency") return qsTr("Latency")
        if (key === "restfulness") return qsTr("Restfulness")
        if (key === "restfulness_a" || key === "restfulness_b" || key === "restfulness_c") return qsTr("Restfulness part")
        if (key === "timing") return qsTr("Timing")
        // Readiness
        if (key === "activity_balance") return qsTr("Activity balance")
        if (key === "body_temperature") return qsTr("Body temperature")
        if (key === "hrv_balance") return qsTr("HRV balance")
        if (key === "previous_day_activity") return qsTr("Previous day activity")
        if (key === "previous_night") return qsTr("Previous night")
        if (key === "recovery_index") return qsTr("Recovery index")
        if (key === "resting_heart_rate") return qsTr("Resting heart rate")
        if (key === "sleep_balance") return qsTr("Sleep balance")
        // Activity
        if (key === "meet_daily_targets") return qsTr("Daily targets")
        if (key === "move_every_hour") return qsTr("Move every hour")
        if (key === "recovery_time") return qsTr("Recovery time")
        if (key === "stay_active") return qsTr("Stay active")
        if (key === "training_frequency") return qsTr("Training frequency")
        if (key === "training_volume") return qsTr("Training volume")
        return key
    }

    function compareFor(key) {
        for (var i = 0; i < compare.length; ++i)
            if (compare[i].key === key)
                return compare[i].value
        return undefined
    }

    width: parent ? parent.width : 540

    Repeater {
        model: bars.items
        delegate: Item {
            width: bars.width
            height: Theme.itemSizeSmall

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width * 0.5 - Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                text: bars.nameOf(modelData.key)
                font.pixelSize: Theme.fontSizeSmall
                truncationMode: TruncationMode.Fade
            }
            Rectangle {
                id: track
                x: parent.width * 0.5
                width: parent.width * 0.5 - Theme.horizontalPageMargin - values.width - Theme.paddingSmall
                height: Theme.paddingMedium
                radius: height / 2
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.rgba(Theme.primaryColor, 0.15)
                Rectangle {
                    width: parent.width * Math.max(0.02, Math.min(1, modelData.value / 100))
                    height: parent.height
                    radius: height / 2
                    color: Theme.highlightColor
                }
            }
            Row {
                id: values
                anchors {
                    right: parent.right
                    rightMargin: Theme.horizontalPageMargin
                    verticalCenter: parent.verticalCenter
                }
                spacing: Theme.paddingSmall
                Label {
                    font.pixelSize: Theme.fontSizeSmall
                    color: Theme.primaryColor
                    text: Math.round(modelData.value)
                }
                Label {
                    visible: bars.compareFor(modelData.key) !== undefined
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    anchors.baseline: parent.children[0].baseline
                    text: "(" + bars.compareFor(modelData.key) + ")"
                }
            }
        }
    }
}

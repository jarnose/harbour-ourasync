import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components/Format.js" as Format

CoverBackground {
    id: cover

    property var overview: ({})

    function refresh() {
        overview = AppController.overview()
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

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            visible: overview.night !== undefined
                     && (overview.night.ouraScore !== null || overview.night.score !== null)
            font.pixelSize: Theme.fontSizeHuge
            text: overview.night && overview.night.ouraScore !== null && overview.night.ouraScore !== undefined
                  ? overview.night.ouraScore
                  : (overview.night && overview.night.score !== null && overview.night.score !== undefined
                     ? overview.night.score : "")
        }
        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            visible: overview.night !== undefined
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeSmall
            text: overview.night ? qsTr("Slept %1").arg(Format.duration(overview.night.asleepMin)) : ""
        }
        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            visible: overview.latestHr !== undefined
            color: Theme.secondaryColor
            font.pixelSize: Theme.fontSizeSmall
            text: overview.latestHr !== undefined ? Format.number(overview.latestHr) + " bpm" : ""
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

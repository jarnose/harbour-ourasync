import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"
import "../components/Format.js" as Format

// This app's sleep arithmetic set beside Oura's own, night by night. Where the two
// disagree is where the arithmetic is incomplete, so this is the page to look at
// when judging how far to trust a score computed from the ring alone.
Page {
    id: page

    property var rows: []

    function reload() {
        var list = AppController.nights(120)
        var out = []
        for (var i = 0; i < list.length; ++i) {
            var n = list[i]
            if (!n.main || n.ouraScore === null || n.ouraScore === undefined)
                continue
            var full = AppController.night(n.date)
            out.push({ date: n.date, official: n.ouraScore, estimate: n.score,
                       formula: full.ouraFormula, officialContributors: full.ouraContributors })
        }
        rows = out
    }
    Component.onCompleted: reload()

    // Mean absolute difference per contributor over the nights where both exist.
    function contributorErrors() {
        var sum = {}, count = {}
        for (var i = 0; i < rows.length; ++i) {
            var f = rows[i].formula
            if (!f || !f.contributors)
                continue
            for (var k = 0; k < rows[i].officialContributors.length; ++k) {
                var key = rows[i].officialContributors[k].key
                var mine = f.contributors[key]
                if (mine === undefined)
                    continue
                sum[key] = (sum[key] || 0) + Math.abs(mine - rows[i].officialContributors[k].value)
                count[key] = (count[key] || 0) + 1
            }
        }
        var out = []
        for (var name in sum)
            out.push({ key: name, value: Math.round(sum[name] / count[name] * 10) / 10, nights: count[name] })
        return out
    }

    SilicaListView {
        anchors.fill: parent
        model: page.rows

        header: Column {
            width: parent.width
            PageHeader { title: qsTr("Score check") }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Oura's sleep score against this app's estimate, and against Oura's own arithmetic "
                           + "applied to your night (age from Settings). Contributor errors are the mean "
                           + "difference in points. Restfulness needs the ring's own movement data, so it "
                           + "appears only for nights recorded on the ring, not for nights imported from Oura's cloud.")
            }
            Repeater {
                model: page.contributorErrors()
                delegate: Item {
                    width: parent.width
                    height: Theme.itemSizeExtraSmall
                    Label {
                        x: Theme.horizontalPageMargin
                        anchors.verticalCenter: parent.verticalCenter
                        font.pixelSize: Theme.fontSizeSmall
                        text: modelData.key.replace(/_/g, " ")
                    }
                    Label {
                        anchors { right: parent.right; rightMargin: Theme.horizontalPageMargin; verticalCenter: parent.verticalCenter }
                        font.pixelSize: Theme.fontSizeSmall
                        color: modelData.value <= 2 ? Theme.highlightColor : Theme.primaryColor
                        text: "±" + modelData.value + "  (" + modelData.nights + ")"
                    }
                }
            }
            SectionHeader { text: qsTr("Night by night") }
        }

        delegate: Item {
            width: parent.width
            height: Theme.itemSizeMedium
            Label {
                x: Theme.horizontalPageMargin
                anchors.verticalCenter: parent.verticalCenter
                text: Format.dayLabel(modelData.date)
            }
            Row {
                anchors { right: parent.right; rightMargin: Theme.horizontalPageMargin; verticalCenter: parent.verticalCenter }
                spacing: Theme.paddingLarge
                Column {
                    Label { text: modelData.official; color: Theme.highlightColor; anchors.horizontalCenter: parent.horizontalCenter }
                    Label { text: qsTr("Oura"); font.pixelSize: Theme.fontSizeExtraSmall; color: Theme.secondaryColor }
                }
                Column {
                    Label {
                        text: modelData.formula && modelData.formula.score !== undefined ? modelData.formula.score : "–"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Label { text: qsTr("formula"); font.pixelSize: Theme.fontSizeExtraSmall; color: Theme.secondaryColor }
                }
                Column {
                    Label {
                        text: modelData.estimate !== null && modelData.estimate !== undefined ? modelData.estimate : "–"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }
                    Label { text: qsTr("estimate"); font.pixelSize: Theme.fontSizeExtraSmall; color: Theme.secondaryColor }
                }
            }
        }

        ViewPlaceholder {
            enabled: page.rows.length === 0
            text: qsTr("Nothing to compare")
            hintText: qsTr("Import your Oura data first")
        }
        VerticalScrollDecorator {}
    }
}

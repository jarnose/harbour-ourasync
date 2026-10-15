import QtQuick 2.0
import Sailfish.Silica 1.0

// A 0-100 score in a three-quarter ring. Drawn with a Canvas rather than
// ProgressCircle: Silica's own circle animates and has no room for a caption.
Item {
    id: gauge

    property real value: -1          // negative: no score
    property string caption: ""
    property color accent: Theme.highlightColor
    property real thickness: Theme.paddingMedium

    width: Theme.itemSizeHuge
    height: width

    onValueChanged: canvas.requestPaint()
    onAccentChanged: canvas.requestPaint()

    Canvas {
        id: canvas
        anchors.fill: parent
        // A page that was left and is back again has lost the canvas's contents
        // (the texture is released while it is not shown): paint again.
        onVisibleChanged: if (visible) requestPaint()
        onAvailableChanged: if (available) requestPaint()
        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            var r = Math.min(width, height) / 2 - gauge.thickness / 2
            var cx = width / 2
            var cy = height / 2
            // gap at the bottom: from 135 deg round to 45 deg
            var start = Math.PI * 0.75
            var span = Math.PI * 1.5
            ctx.lineWidth = gauge.thickness
            ctx.lineCap = "round"
            ctx.strokeStyle = Theme.rgba(Theme.primaryColor, 0.15)
            ctx.beginPath()
            ctx.arc(cx, cy, r, start, start + span, false)
            ctx.stroke()
            if (gauge.value >= 0) {
                var f = Math.max(0.02, Math.min(1, gauge.value / 100))
                ctx.strokeStyle = gauge.accent
                ctx.beginPath()
                ctx.arc(cx, cy, r, start, start + span * f, false)
                ctx.stroke()
            }
        }
    }

    Label {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -Theme.paddingSmall
        text: gauge.value >= 0 ? Math.round(gauge.value) : "–"
        font.pixelSize: gauge.width * 0.3
        color: Theme.primaryColor
    }
    Label {
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
            bottomMargin: gauge.thickness
        }
        text: gauge.caption
        font.pixelSize: Theme.fontSizeExtraSmall
        color: Theme.secondaryColor
    }
}

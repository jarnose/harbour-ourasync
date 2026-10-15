import QtQuick 2.0
import Sailfish.Silica 1.0

// Sleep stages as horizontal blocks, in Oura's order: awake on top, then REM,
// light, deep. One column per epoch; codes are 1 deep, 2 light, 3 REM, 4 awake,
// and 0 for an epoch the ring never staged (drawn as a gap, not as wakefulness).
Item {
    id: chart

    property var stages: []
    property real startMs: 0
    property int epochSeconds: 30

    implicitHeight: Theme.itemSizeLarge * 2
    height: implicitHeight

    onStagesChanged: canvas.requestPaint()

    readonly property real leftMargin: Theme.paddingLarge * 3
    readonly property real bottomMargin: Theme.fontSizeExtraSmall * 1.6

    // row index from the top: awake, rem, light, deep
    function rowOf(code) {
        if (code === 4) return 0
        if (code === 3) return 1
        if (code === 2) return 2
        if (code === 1) return 3
        return -1
    }

    function colorOf(code) {
        if (code === 1) return "#4a5bd0"
        if (code === 2) return "#8d9bf0"
        if (code === 3) return "#22b8cf"
        return "#ff9a6b"
    }

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
            var n = chart.stages.length
            var plotW = width - chart.leftMargin
            var plotH = height - chart.bottomMargin
            var rowH = plotH / 4

            ctx.font = Theme.fontSizeExtraSmall + "px sans-serif"
            ctx.fillStyle = Theme.secondaryColor
            var names = [qsTr("Awake"), qsTr("REM"), qsTr("Light"), qsTr("Deep")]
            ctx.strokeStyle = Theme.rgba(Theme.primaryColor, 0.10)
            ctx.lineWidth = 1
            for (var r = 0; r < 4; ++r) {
                ctx.fillText(names[r], 0, r * rowH + rowH / 2 + Theme.fontSizeExtraSmall / 3)
                ctx.beginPath(); ctx.moveTo(chart.leftMargin, r * rowH + rowH); ctx.lineTo(width, r * rowH + rowH); ctx.stroke()
            }
            if (n === 0) return

            var colW = plotW / n
            var drawW = colW + 0.75 // overlap a little: sub-pixel columns would leave seams
            for (var i = 0; i < n; ++i) {
                var code = chart.stages[i]
                var row = chart.rowOf(code)
                if (row < 0) continue
                ctx.fillStyle = chart.colorOf(code)
                ctx.fillRect(chart.leftMargin + i * colW, row * rowH + rowH * 0.12, drawW, rowH * 0.76)
            }

            ctx.fillStyle = Theme.secondaryColor
            var a = new Date(chart.startMs)
            var b = new Date(chart.startMs + n * chart.epochSeconds * 1000)
            function hm(d) { return d.getHours() + ":" + (d.getMinutes() < 10 ? "0" : "") + d.getMinutes() }
            ctx.fillText(hm(a), chart.leftMargin, height - 2)
            var label = hm(b)
            ctx.fillText(label, width - ctx.measureText(label).width, height - 2)
        }
    }
}

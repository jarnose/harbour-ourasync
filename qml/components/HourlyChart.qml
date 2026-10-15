import QtQuick 2.0
import Sailfish.Silica 1.0

// Twenty-four columns, one per local hour. Two modes:
//  * range: each column spans min..max with a tick at the mean (heart rate);
//  * bars: each column is a single value rising from zero (MET, activity).
Item {
    id: chart

    property var hours: []           // 24 entries
    property bool rangeMode: true
    property color barColor: Theme.highlightColor
    property real fixedMax: NaN
    property string unit: ""

    implicitHeight: Theme.itemSizeLarge * 1.6
    height: implicitHeight

    onHoursChanged: canvas.requestPaint()
    onBarColorChanged: canvas.requestPaint()

    readonly property real leftMargin: Theme.paddingLarge * 2
    readonly property real bottomMargin: Theme.fontSizeExtraSmall * 1.6

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
            var plotW = width - chart.leftMargin
            var plotH = height - chart.bottomMargin
            var lo = Infinity, hi = -Infinity
            for (var i = 0; i < chart.hours.length; ++i) {
                var h = chart.hours[i]
                if (chart.rangeMode) {
                    if (!h || h.count === 0) continue
                    lo = Math.min(lo, h.min); hi = Math.max(hi, h.max)
                } else {
                    var m = (h === undefined || h === null) ? 0 : h
                    hi = Math.max(hi, m)
                }
            }
            if (!isFinite(hi)) { lo = 0; hi = 1 }
            if (!chart.rangeMode) lo = 0
            if (!isNaN(chart.fixedMax)) hi = chart.fixedMax
            if (chart.rangeMode) { var pad = Math.max(3, (hi - lo) * 0.1); lo -= pad; hi += pad }
            if (hi - lo < 1e-9) hi = lo + 1

            function yOf(v) { return plotH - (v - lo) / (hi - lo) * plotH }

            ctx.font = Theme.fontSizeExtraSmall + "px sans-serif"
            ctx.fillStyle = Theme.secondaryColor
            ctx.strokeStyle = Theme.rgba(Theme.primaryColor, 0.12)
            ctx.lineWidth = 1
            for (var g = 0; g <= 2; ++g) {
                var gv = lo + (hi - lo) * g / 2
                var gy = yOf(gv)
                ctx.beginPath(); ctx.moveTo(chart.leftMargin, gy); ctx.lineTo(width, gy); ctx.stroke()
                ctx.fillText(Math.round(gv), 0, Math.max(gy - 2, Theme.fontSizeExtraSmall))
            }

            var colW = plotW / 24
            var barW = Math.max(2, colW * 0.62)
            for (var k = 0; k < 24; ++k) {
                var item = chart.hours[k]
                var x = chart.leftMargin + k * colW + (colW - barW) / 2
                if (chart.rangeMode) {
                    if (!item || item.count === 0) continue
                    ctx.fillStyle = Theme.rgba(chart.barColor, 0.55)
                    var top = yOf(item.max), bottom = yOf(item.min)
                    ctx.fillRect(x, top, barW, Math.max(2, bottom - top))
                    ctx.fillStyle = chart.barColor
                    ctx.fillRect(x, yOf(item.mean) - 1.5, barW, 3)
                } else {
                    var v = (item === undefined || item === null) ? 0 : item
                    if (v <= 0) continue
                    ctx.fillStyle = chart.barColor
                    ctx.fillRect(x, yOf(v), barW, plotH - yOf(v))
                }
            }

            ctx.fillStyle = Theme.secondaryColor
            var marks = [0, 6, 12, 18]
            for (var mk = 0; mk < marks.length; ++mk)
                ctx.fillText(marks[mk] < 10 ? "0" + marks[mk] : "" + marks[mk],
                             chart.leftMargin + marks[mk] * colW, height - 2)
        }
    }
}

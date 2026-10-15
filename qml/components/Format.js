.pragma library

// Formatting helpers shared by the pages. QML's JavaScript is ES5 (Qt 5.6):
// no let/const, arrow functions or template strings in this file.

function pad2(n) {
    return n < 10 ? "0" + n : "" + n
}

// 7 h 32 min from minutes.
function duration(minutes) {
    if (minutes === undefined || minutes === null || isNaN(minutes))
        return "–"
    var total = Math.round(minutes)
    var h = Math.floor(total / 60)
    var m = total % 60
    if (h === 0)
        return m + " min"
    return h + " h " + pad2(m) + " min"
}

// 7:32 from minutes: compact enough for a four-column row of tiles.
function hoursMinutes(minutes) {
    if (minutes === undefined || minutes === null || isNaN(minutes))
        return "–"
    var total = Math.round(minutes)
    return Math.floor(total / 60) + ":" + pad2(total % 60)
}

function hm(ms) {
    var d = new Date(ms)
    return pad2(d.getHours()) + ":" + pad2(d.getMinutes())
}

// "Tue 14.10."
function dayLabel(date) {
    // date is YYYY-MM-DD in the phone's local calendar.
    var p = date.split("-")
    var d = new Date(parseInt(p[0], 10), parseInt(p[1], 10) - 1, parseInt(p[2], 10))
    return Qt.formatDate(d, "ddd d.M.")
}

function shortDay(date) {
    var p = date.split("-")
    return parseInt(p[2], 10) + "." + parseInt(p[1], 10) + "."
}

function number(value, decimals) {
    if (value === undefined || value === null || isNaN(value))
        return "–"
    return Number(value).toFixed(decimals === undefined ? 0 : decimals)
}

function signed(value, decimals) {
    if (value === undefined || value === null || isNaN(value))
        return "–"
    var s = Number(value).toFixed(decimals === undefined ? 1 : decimals)
    return value > 0 ? "+" + s : s
}

// "3 h ago", "yesterday 21:14" — enough to see whether a sync is stale.
function ago(ms) {
    if (!ms)
        return ""
    var seconds = Math.max(0, (Date.now() - ms) / 1000)
    if (seconds < 90)
        return "now"
    if (seconds < 3600)
        return Math.round(seconds / 60) + " min"
    if (seconds < 86400)
        return Math.round(seconds / 3600) + " h"
    return Math.round(seconds / 86400) + " d"
}

// Index of a stage code to its display name, in the order the hypnogram shows them.
var stageNames = ["", "Deep", "Light", "REM", "Awake"]

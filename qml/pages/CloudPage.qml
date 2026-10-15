import QtQuick 2.0
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0

// History from Oura's cloud. Two routes, and the first needs nothing:
//  * the data export: the ZIP of CSV files that Oura gives every ring owner at
//    membership.ouraring.com/data-export, opened from a file;
//  * the Oura Cloud API: for owners with an active Oura Membership, signing in
//    with an application of their own.
// Both fill in the time before the ring was first read here, and bring Oura's own
// scores, which a ring cannot report.
Page {
    id: page

    property string status: ""
    property bool ok: true
    readonly property var stats: AppController.cloudStats
    readonly property var docs: stats.docs !== undefined ? stats.docs : ({})

    function count(kind) { return docs[kind] !== undefined ? docs[kind] : 0 }

    Connections {
        target: AppController
        onCloudFinished: {
            page.ok = ok
            page.status = message
        }
    }

    function pick(path) {
        page.status = ""
        AppController.importOuraExport(path)
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width

            PageHeader { title: qsTr("Oura cloud") }

            // ---- what is here ----
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: page.count("daily_sleep") + page.count("sleep") + page.count("daily_activity") > 0
                      ? qsTr("%1 nights, %2 days of activity and %3 readiness scores, from %4 to %5.")
                        .arg(page.count("sleep")).arg(page.count("daily_activity"))
                        .arg(page.count("daily_readiness")).arg(page.stats.from).arg(page.stats.to)
                      : qsTr("Nothing imported yet.")
            }

            // ---- busy / result ----
            Column {
                width: parent.width
                visible: AppController.cloudBusy
                BusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    size: BusyIndicatorSize.Medium
                    running: AppController.cloudBusy
                }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    color: Theme.secondaryHighlightColor
                    text: AppController.cloudStatus
                }
            }
            Label {
                visible: page.status.length > 0 && !AppController.cloudBusy
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: page.ok ? Theme.highlightColor : Theme.errorColor
                text: page.status
            }
            Label {
                visible: AppController.lastError.length > 0 && !AppController.cloudBusy
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.errorColor
                font.pixelSize: Theme.fontSizeSmall
                text: AppController.lastError
            }

            // ---- the export file ----
            SectionHeader { text: qsTr("Data export (no account needed)") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Every Oura user can download their own data from membership.ouraring.com/data-export "
                           + "(sign in with the email code, choose Request export, wait for the email). Save the "
                           + "ZIP on the phone and open it here: the scores, sleep stages, heart rate and "
                           + "activity of all your history are read, and nothing leaves the phone.")
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: !AppController.cloudBusy
                text: qsTr("Open from Downloads")
                onClicked: pageStack.push(downloadPicker)
            }
            Item { width: 1; height: Theme.paddingMedium }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: !AppController.cloudBusy
                text: qsTr("Open from Documents")
                onClicked: pageStack.push(documentPicker)
            }

            TextField {
                id: pathField
                width: parent.width
                label: qsTr("Or a file or folder path")
                placeholderText: label
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: {
                    focus = false
                    if (text.length > 0)
                        page.pick(text)
                }
            }

            // ---- the API ----
            SectionHeader { text: qsTr("Oura Cloud API") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                text: qsTr("Needs an active Oura Membership. Oura no longer issues personal tokens, so you "
                           + "register a small application of your own at cloud.ouraring.com/oauth/applications "
                           + "with the redirect address below, and enter its client id and secret here. They "
                           + "stay in this phone's secure vault.")
            }

            TextField {
                id: clientId
                width: parent.width
                label: qsTr("Client id")
                text: AppController.cloudClientId
                placeholderText: label
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhSensitiveData
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: clientSecret.focus = true
            }
            PasswordField {
                id: clientSecret
                width: parent.width
                label: qsTr("Client secret")
                placeholderText: label
                EnterKey.iconSource: "image://theme/icon-m-enter-next"
                EnterKey.onClicked: portField.focus = true
            }
            TextField {
                id: portField
                width: parent.width
                label: qsTr("Redirect address: http://localhost:%1/callback").arg(text)
                text: AppController.cloudPort
                inputMethodHints: Qt.ImhDigitsOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-close"
                EnterKey.onClicked: focus = false
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                enabled: !AppController.cloudBusy && clientId.text.length > 0
                         && (clientSecret.text.length > 0 || AppController.cloudConnected)
                text: AppController.cloudConnected ? qsTr("Sign in again") : qsTr("Sign in with Oura")
                onClicked: {
                    page.status = ""
                    var url = AppController.cloudBeginAuth(clientId.text, clientSecret.text, parseInt(portField.text, 10))
                    if (url.length > 0)
                        Qt.openUrlExternally(url)
                }
            }

            TextField {
                id: pasted
                width: parent.width
                label: qsTr("If the browser could not reach the app: paste the address or code")
                placeholderText: qsTr("Address or code")
                inputMethodHints: Qt.ImhNoPredictiveText | Qt.ImhNoAutoUppercase | Qt.ImhUrlCharactersOnly
                EnterKey.iconSource: "image://theme/icon-m-enter-accept"
                EnterKey.onClicked: {
                    focus = false
                    if (text.length > 0)
                        AppController.cloudCompleteAuth(text)
                }
            }

            Label {
                visible: AppController.cloudConnected
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.highlightColor
                text: qsTr("Connected to Oura.")
            }

            ComboBox {
                id: span
                visible: AppController.cloudConnected
                label: qsTr("Import")
                menu: ContextMenu {
                    MenuItem { text: qsTr("the last 30 days") }
                    MenuItem { text: qsTr("the last 90 days") }
                    MenuItem { text: qsTr("the last year") }
                    MenuItem { text: qsTr("everything (up to 10 years)") }
                }
            }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: AppController.cloudConnected
                enabled: !AppController.cloudBusy
                text: qsTr("Import now")
                onClicked: {
                    page.status = ""
                    var days = [30, 90, 365, 3650]
                    AppController.cloudImport(days[span.currentIndex])
                }
            }
            Item { width: 1; height: Theme.paddingMedium; visible: AppController.cloudConnected }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: AppController.cloudConnected
                text: qsTr("Disconnect")
                onClicked: AppController.cloudDisconnect()
            }

            // ---- housekeeping ----
            SectionHeader {
                visible: page.count("daily_sleep") + page.count("sleep") + page.count("daily_activity") > 0
                text: qsTr("Imported data")
            }
            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                visible: page.count("daily_sleep") + page.count("sleep") + page.count("daily_activity") > 0
                text: qsTr("Check my scores against Oura's")
                onClicked: pageStack.push(Qt.resolvedUrl("ScoreCheckPage.qml"))
            }
            Item { width: 1; height: Theme.paddingMedium }
            Button {
                id: clearButton
                anchors.horizontalCenter: parent.horizontalCenter
                visible: page.count("daily_sleep") + page.count("sleep") + page.count("daily_activity") > 0
                text: qsTr("Remove imported data")
                onClicked: remorse.execute(clearButton, qsTr("Removing imported data"),
                                           function () { AppController.cloudClear() }, 5000)
            }
            RemorseItem { id: remorse }
            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeExtraSmall
                text: qsTr("Removing leaves the history read from the ring untouched.")
                visible: clearButton.visible
            }
        }

        VerticalScrollDecorator {}
    }

    Component {
        id: downloadPicker
        DownloadPickerPage {
            title: qsTr("Choose the export")
            onSelectedContentPropertiesChanged: {
                if (selectedContentProperties && selectedContentProperties.filePath)
                    page.pick(selectedContentProperties.filePath)
            }
        }
    }
    Component {
        id: documentPicker
        DocumentPickerPage {
            title: qsTr("Choose the export")
            onSelectedContentPropertiesChanged: {
                if (selectedContentProperties && selectedContentProperties.filePath)
                    page.pick(selectedContentProperties.filePath)
            }
        }
    }
}

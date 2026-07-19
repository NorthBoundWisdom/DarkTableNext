import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import GeoControls 1.0 as Geo

ApplicationWindow {
    id: window

    width: 1280
    height: 800
    minimumWidth: 960
    minimumHeight: 640
    visible: true
    title: qsTr("DarkTableNext")
    color: Geo.Theme.windowColor

    property bool showingWelcome: true
    property var startupComponents: [
        {
            id: "qml-shell",
            title: qsTr("QML application shell"),
            detail: qsTr("Welcome and workspace routes are ready."),
            state: "ready",
            progress: 1.0
        },
        {
            id: "component-host",
            title: qsTr("Component host"),
            detail: qsTr("Ready to register future startup component loaders."),
            state: "ready",
            progress: 1.0
        }
    ]

    function showWorkspace() {
        showingWelcome = false;
    }

    function showWelcome() {
        showingWelcome = true;
    }

    StackLayout {
        anchors.fill: parent
        currentIndex: window.showingWelcome ? 0 : 1

        WelcomeScreen {
            Layout.fillWidth: true
            Layout.fillHeight: true
            componentStates: window.startupComponents
            onContinueRequested: window.showWorkspace()
        }

        MainShell {
            Layout.fillWidth: true
            Layout.fillHeight: true
            onWelcomeRequested: window.showWelcome()
        }
    }
}

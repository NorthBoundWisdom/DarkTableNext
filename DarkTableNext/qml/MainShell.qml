pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.15
import GeoControls 1.0 as Geo
import GeoControls.AppShell 1.0 as GeoAppShell

Item {
    id: root

    property string selectedSection: qsTr("Library")

    signal welcomeRequested

    function selectSection(sectionName) {
        selectedSection = sectionName;
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        GeoAppShell.MainToolBar {
            Layout.fillWidth: true
            documentTitle: qsTr("DarkTableNext")
            actionModel: []
            showShortcutsButton: false
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 220
                color: Geo.Theme.contentSurfaceColor
                border.color: Geo.Theme.dividerColor
                border.width: 1

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 16
                    spacing: 10

                    Geo.CustomLabel {
                        text: qsTr("Workspace")
                        font.bold: true
                        font.pixelSize: 18
                    }

                    Repeater {
                        model: [qsTr("Library"), qsTr("Darkroom"), qsTr("Exports")]

                        delegate: Geo.CustomButton {
                            required property string modelData

                            Layout.fillWidth: true
                            text: modelData
                            buttonColor: root.selectedSection === modelData ? Geo.Theme.highlightColor : Geo.Theme.buttonColor
                            buttonTextColor: root.selectedSection === modelData ? Geo.Theme.highlightedTextColor : Geo.Theme.buttonTextColor
                            onClicked: root.selectSection(modelData)
                        }
                    }

                    Item {
                        Layout.fillHeight: true
                    }

                    Geo.CustomButton {
                        Layout.fillWidth: true
                        text: qsTr("Startup components")
                        onClicked: root.welcomeRequested()
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Geo.Theme.pageSurfaceColor

                ColumnLayout {
                    anchors.centerIn: parent
                    width: Math.min(parent.width - 80, 560)
                    spacing: 12

                    Geo.CustomLabel {
                        Layout.fillWidth: true
                        text: root.selectedSection
                        font.pixelSize: 30
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Geo.CustomLabel {
                        Layout.fillWidth: true
                        text: qsTr("This workspace shell is ready for domain adapters, image canvas, and view models.")
                        color: Geo.Theme.placeholderTextColor
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                    }

                    Geo.CustomButton {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Return to startup components")
                        onClicked: root.welcomeRequested()
                    }
                }
            }
        }

        GeoAppShell.MainStatusBar {
            Layout.fillWidth: true
            statusText: qsTr("QML shell ready")
            viewerText: qsTr("No photo is loaded")
        }
    }
}

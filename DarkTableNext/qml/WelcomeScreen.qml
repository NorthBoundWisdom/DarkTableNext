pragma ComponentBehavior: Bound

import QtQuick 2.15
import QtQuick.Layouts 1.15
import GeoControls 1.0 as Geo

Item {
    id: root

    property var componentStates: []
    readonly property bool hasBlockingFailure: componentStates.some(function (component) {
        return component.state === "failed";
    })
    readonly property real overallProgress: {
        if (componentStates.length === 0)
            return 1.0;

        var total = 0.0;
        for (var index = 0; index < componentStates.length; ++index)
            total += Number(componentStates[index].progress || 0.0);
        return total / componentStates.length;
    }

    signal continueRequested

    function replaceComponentStates(states) {
        componentStates = states || [];
    }

    function accentForState(state) {
        if (state === "ready")
            return Geo.Theme.successColor;
        if (state === "loading")
            return Geo.Theme.infoColor;
        if (state === "failed")
            return Geo.Theme.errorColor;
        return Geo.Theme.warningColor;
    }

    function labelForState(state) {
        if (state === "ready")
            return qsTr("Ready");
        if (state === "loading")
            return qsTr("Loading");
        if (state === "failed")
            return qsTr("Needs attention");
        return qsTr("Waiting");
    }

    Rectangle {
        anchors.fill: parent
        color: Geo.Theme.pageSurfaceColor
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - 80, 760)
        spacing: 20

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 8

            Geo.CustomLabel {
                Layout.fillWidth: true
                text: qsTr("Welcome to")
                color: Geo.Theme.placeholderTextColor
                font.pixelSize: 18
                horizontalAlignment: Text.AlignHCenter
            }

            Geo.CustomLabel {
                Layout.fillWidth: true
                text: qsTr("DarkTableNext")
                color: Geo.Theme.textColor
                font.pixelSize: 42
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }

            Geo.CustomLabel {
                Layout.fillWidth: true
                text: qsTr("The new photo workspace is starting as an independent QML shell.")
                color: Geo.Theme.placeholderTextColor
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: componentLayout.implicitHeight + 40
            radius: 12
            color: Geo.Theme.contentSurfaceColor
            border.color: Geo.Theme.dividerColor
            border.width: 1

            ColumnLayout {
                id: componentLayout
                anchors.fill: parent
                anchors.margins: 20
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true

                    Geo.CustomLabel {
                        text: qsTr("Startup components")
                        font.bold: true
                        font.pixelSize: 18
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Geo.CustomLabel {
                        text: Math.round(root.overallProgress * 100) + "%"
                        color: Geo.Theme.placeholderTextColor
                    }
                }

                Rectangle {
                    id: progressRail

                    Layout.fillWidth: true
                    implicitHeight: 6
                    radius: 3
                    color: Geo.Theme.railSurfaceColor

                    Rectangle {
                        width: parent.width * root.overallProgress
                        height: parent.height
                        radius: progressRail.radius
                        color: root.hasBlockingFailure ? Geo.Theme.errorColor : Geo.Theme.highlightColor

                        Behavior on width {
                            NumberAnimation {
                                duration: 160
                            }
                        }
                    }
                }

                Repeater {
                    model: root.componentStates

                    delegate: Rectangle {
                        id: componentRow

                        required property var modelData
                        readonly property var componentState: componentRow.modelData || ({})

                        Layout.fillWidth: true
                        implicitHeight: stateLayout.implicitHeight + 24
                        radius: 8
                        color: Geo.Theme.inputSurfaceColor
                        border.color: root.accentForState(componentState.state)
                        border.width: 1

                        RowLayout {
                            id: stateLayout
                            anchors.fill: parent
                            anchors.margins: 12
                            spacing: 12

                            Rectangle {
                                Layout.minimumWidth: 10
                                Layout.minimumHeight: 10
                                Layout.preferredWidth: 10
                                Layout.preferredHeight: 10
                                radius: 5
                                color: root.accentForState(componentRow.componentState.state)
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2

                                Geo.CustomLabel {
                                    Layout.fillWidth: true
                                    text: componentRow.componentState.title || qsTr("Unnamed component")
                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Geo.CustomLabel {
                                    Layout.fillWidth: true
                                    text: componentRow.componentState.detail || ""
                                    color: Geo.Theme.placeholderTextColor
                                    wrapMode: Text.WordWrap
                                }
                            }

                            Geo.CustomLabel {
                                text: root.labelForState(componentRow.componentState.state)
                                color: root.accentForState(componentRow.componentState.state)
                            }
                        }
                    }
                }
            }
        }

        Geo.CustomButton {
            Layout.alignment: Qt.AlignHCenter
            text: root.hasBlockingFailure ? qsTr("Component loading needs attention") : qsTr("Open workspace")
            enabled: !root.hasBlockingFailure
            onClicked: root.continueRequested()
        }
    }
}

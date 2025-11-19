/*
* Lysa - Photo Organizer
* Copyright (C) 2025 vorks. DEV (Jeremy Voß)
* 
* This file is part of Lysa.
* 
* Licensed under GNU GPL-3
* Full license: see LICENSE.txt
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import QtCore
import "."

ApplicationWindow {
    id: window
    visible: false
    width: 600
    height: 400
    flags: Qt.FramelessWindowHint
    title: "Settings"
    color: UI.background
    font.pixelSize: 12
    palette: Palette {
        text: UI.font
        buttonText: UI.font
        windowText: UI.font
    }

    Rectangle {
        id: titleBar
        height: 32
        width: parent.width
        color: UI.backgroundLite
        anchors.top: parent.top
        z: 99

        DragHandler {
            onActiveChanged: if (active) window.startSystemMove()
            target: null
        }

        TapHandler {
            onDoubleTapped: window.visibility = window.visibility === Window.Maximized ? Window.Windowed : Window.Maximized
        }

        Text {
            text: window.title
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 10
            color: UI.font
            font.bold: true
        }

        Button {
            id: closeButton
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 6
            text: "✕"
            onClicked: window.close()
            background: Rectangle {
                color: closeButton.hovered ? UI.background : "transparent"
                radius: 4
            }
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
    }


    property string currentSection: sectionList.currentIndex >= 0 ? sectionList.model[sectionList.currentIndex] : ""

    RowLayout {
        anchors.top: titleBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        //----- Left pane: Sections -----//
        ListView {
            id: sectionList
            Layout.preferredWidth: 150
            Layout.fillHeight: true
            model: settingsModel.getSections()
            delegate: ItemDelegate {
                id: sectionItem
                text: modelData
                width: parent.width
                checked: sectionList.currentIndex === index
                background: Rectangle { color: sectionItem.checked ? UI.backgroundLite : UI.background }
                onClicked: sectionList.currentIndex = index
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }

        }

        //----- Right pane: Settings of selected section -----//
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.topMargin: 8
            Layout.leftMargin: 8
            Layout.bottomMargin: 8

            ColumnLayout {
                id: settingsContainer
                spacing: 10
                Layout.margins: 20

                Repeater {
                    model: settingsModel.getSectionSettings(currentSection)
                    delegate: RowLayout {
                        spacing: modelData.label ? 10 : 0
                        visible: modelData.visible

                        Label {
                            text: modelData.label
                            Layout.preferredWidth: text ? 150 : 0
                            wrapMode: Text.WordWrap
                        }

                        Loader {
                            property var setting: modelData

                            sourceComponent:
                                modelData.type === "Switch" ? switchDelegate :
                                modelData.type === "TextField" ? textFieldDelegate :
                                modelData.type === "ComboBox" ? comboBoxDelegate :
                                modelData.type === "Link" ? linkDelegate :
                                modelData.type === "TxtArea" ? txtAreaDelegate :
                                modelData.type === "FolderDialog" ? folderDialogDelegate :
                                null
                        }
                    }
                }
            }
        }
    }

    //----- Control delegates -----//
    Component {
        id: switchDelegate
        Switch {
            checked: setting.value
            onToggled: settingsModel.setValue(setting.id, checked)
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
    }

    Component {
        id: textFieldDelegate
        TextField {
            text: setting.value
            width: 250
            height: font.pixelSize + 14
            background: Rectangle { 
                color: UI.backgroundLite
                border.color: parent && parent.activeFocus ? UI.main : "transparent"
                radius: 4
            }
            onEditingFinished: settingsModel.setValue(setting.id, text)
        }
    }

    Component {
        id: comboBoxDelegate
        ComboBox {
            id: control

            width: 250
            height: font.pixelSize + 14

            property bool initialized: false
            Component.onCompleted: initialized = true

            model: setting.options
            currentIndex: setting.options.indexOf(setting.value)
            onCurrentIndexChanged: if(initialized) settingsModel.setValue(setting.id, setting.options[currentIndex])
            delegate: ItemDelegate {
                id: delegateItem

                width: control.width
                hoverEnabled: true
                background: Rectangle {
                    color: delegateItem.hovered ? Qt.lighter(UI.backgroundLite, 1.2) : UI.backgroundLite
                    Behavior on color { ColorAnimation { duration: 150 } }
                }
                contentItem: Text {
                    text: modelData
                    color: UI.font
                    font.pixelSize: control.font.pixelSize
                }
                onClicked: {
                    control.currentIndex = index
                    control.popup.close()
                }
                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }

            background: Rectangle {
                color: UI.backgroundLite
                border.color: parent && parent.activeFocus ? UI.main : "transparent"
                radius: 4
            }
            popup: Popup {
                y: control.height - 1
                width: control.width
                implicitHeight: contentItem.implicitHeight
                padding: 1

                contentItem: ListView {
                    clip: true
                    implicitHeight: contentHeight
                    model: control.model
                    currentIndex: control.highlightedIndex

                    delegate: control.delegate
                    ScrollIndicator.vertical: ScrollIndicator {}
                }

                background: Rectangle {
                    color: UI.backgroundLite
                    border.color: parent && parent.activeFocus ? UI.main : "transparent"
                    radius: 4
                }
            }
            HoverHandler {
                cursorShape: Qt.PointingHandCursor
            }
        }
    }
    Component {
        id: linkDelegate
        Item {
            id: root
            width: linkButton.implicitWidth
            height: linkButton.implicitHeight

            Button {
                id: linkButton
                property string type: setting.options[0]
                text: `Open ${type === "https://" ? `${setting.options[1]} in Browser` : setting.value}`
                width: setting.label ? 250 : 410
                height: font.pixelSize + 14
                background: Rectangle {
                    color: linkButton.hovered ? Qt.lighter(UI.backgroundLite, 1.2) : UI.backgroundLite
                    radius: 4
                }
                contentItem: Text {
                    text: linkButton.text
                    color: UI.font
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    anchors.fill: parent
                    leftPadding: 8
                }
                onClicked: {
                    let resolvedLink = setting.value
                    if(type === "file:///") resolvedLink = fileService.resolveRelativePath(setting.value)
                    Qt.openUrlExternally(type + resolvedLink)
                }

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }
        }
    }
    Component {
        id: txtAreaDelegate

        Item {
            id: root
            width: column.implicitWidth
            height: column.implicitHeight

            property string readPath

            Column {
                id: column

                Button {
                    id: txtAreaButton
                    text: setting.value
                    width: setting.label ? 250 : 410
                    height: font.pixelSize + 14

                    background: Rectangle {
                        color: txtAreaButton.hovered ? Qt.lighter(UI.backgroundLite, 1.2) : UI.backgroundLite
                        radius: 4
                        antialiasing: true
                        clip: true

                        layer.enabled: true
                        layer.smooth: true
                        Rectangle { anchors.bottom: parent.bottom; height: textArea.visible ? parent.radius : 0; width: parent.width; color: parent.color }
                    }

                    leftPadding: 0
                    rightPadding: 0
                    contentItem: RowLayout {
                        Layout.leftMargin: 0
                        Layout.rightMargin: 0
                        spacing: 0

                        Text {
                            text: txtAreaButton.text
                            color: UI.font
                            Layout.fillHeight: true
                            elide: Text.ElideRight
                            leftPadding: 8
                        }
                        Item { Layout.fillWidth: true }
                        Text {
                            text: "\u2BC6"
                            color: UI.font
                            Layout.fillHeight: true
                            rightPadding: 8
                        }
                    }

                    onClicked: {
                        if (!textArea.visible && setting.value != root.readPath) {
                            textArea.text = fileService.readTxt(setting.value)
                            root.readPath = setting.value
                        }
                        textArea.visible = !textArea.visible
                    }

                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }
                }

                // Separator
                Rectangle {
                    height: textArea.visible ? 1 : 0
                    width: setting.label ? 250 : 410
                    color: UI.background
                }

                TextArea {
                    id: textArea
                    width: setting.label ? 250 : 410
                    color: UI.font
                    readOnly: true
                    wrapMode: Text.Wrap
                    visible: false
                    background: Rectangle {
                        color: UI.backgroundLite
                        radius: 4
                        antialiasing: true
                        clip: true
                        Rectangle { anchors.top: parent.top; height: parent.radius; width: parent.width; color: parent.color }
                    }
                }
            }
        }
    }
    Component {
        id: folderDialogDelegate
        
        Item {
            id: folderDialogItem
            implicitWidth: folderDialogButton.implicitWidth
            implicitHeight: folderDialogButton.implicitHeight

            property alias folderDialog: folderDialog
            property string settingsId

            function showDialog() {
                folderDialog.open()
            }

            Button {
                id: folderDialogButton
                text: settingsId ? "" : setting.value ? setting.value : "Select the Root Folder"
                width: 250
                height: font.pixelSize + 14
                background: Rectangle {
                    color: folderDialogButton.hovered ? Qt.lighter(UI.backgroundLite, 1.2) : UI.backgroundLite
                    radius: 4
                }
                contentItem: Text {
                    text: folderDialogButton.text
                    color: UI.font
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    anchors.fill: parent
                    leftPadding: 8
                }
                onClicked: folderDialogItem.showDialog()

                HoverHandler {
                    cursorShape: Qt.PointingHandCursor
                }
            }
            FolderDialog {
                id: folderDialog
                title: "Select the Root Folder"
                currentFolder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
                onAccepted: settingsModel.setValue(folderDialogItem.settingsId ? folderDialogItem.settingsId : setting.id, toLocalPath(selectedFolder.toString()))
                
                function toLocalPath(url) {
                    return url && url.startsWith("file://") ? url.substring(8) : url
                }
            }
        }
    }

    //----- Functions -----//
    function openRootFolderDialog() {
        const dialogItem = folderDialogDelegate.createObject(window)
        dialogItem.settingsId = "rootFolder"
        dialogItem.showDialog()
        dialogItem.folderDialog.onAccepted.connect(() => dialogItem.destroy())
        dialogItem.folderDialog.onRejected.connect(() => dialogItem.destroy())
    }
}

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
import "."

ApplicationWindow {
    id: window
    visible: true
    width: 1200
    height: 800
    flags: Qt.Window | Qt.FramelessWindowHint
    color: UI.background
    font.pixelSize: 12
    palette: Palette {
        text: UI.font
        buttonText: UI.font
        windowText: UI.font
    }

    property var settings

    function toggleMaximized() {
        if(window.visibility === Window.Maximized) window.showNormal()
        else window.showMaximized()

        UI.mainWindowWidth = window.width
    }

    ToolBar {
        id: toolBar
        RowLayout {
            anchors.fill: parent

            Image {
                source: "icons/lysa.svg"
                Layout.preferredHeight: toolBar.height * 0.5
                Layout.preferredWidth: toolBar.height * 0.5
                fillMode: Image.PreserveAspectFit
                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: 10
                Layout.rightMargin: 5
            }

            MenuButton {
                text: "Albums"
                toolTip: "Show Album Sidebar"
                onClicked: UI.showSidebar = !UI.showSidebar
            }
            MenuButton {
                text: "Settings"
                toolTip: "Open App Settings"
                onClicked: {
                    if (!window.settings) {
                        window.instantiateSettings()
                        return;
                    }
                    if (!window.settings.visible) window.settings.show()
                    window.settings.raise()
                    window.settings.requestActivate()
                }
            }

            // Spacer
            Item { Layout.fillWidth: true }

            // Window controls
            MenuButton { text: "—"; onClicked: window.showMinimized() }
            MenuButton { text: window.visibility === Window.Maximized ? "🗗" : "🗖"; onClicked: window.toggleMaximized() }
            MenuButton {
                text: "✕"
                onClicked: window.close()
            }
        }
    }

    Rectangle {
        id: sideBar
        width: UI.showSidebar ? 300 : 0
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        color: UI.background

        Behavior on width { NumberAnimation { duration: 100; easing.type: Easing.InOutQuad } }

        Item {
            id: root
            anchors.top: parent.top
            anchors.topMargin: 32
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            clip: true

            implicitHeight: content.implicitHeight + 8
            implicitWidth: directoryTree.width

            Rectangle {
                anchors.fill: parent
                color: directoryModel.activePath === directoryModel.rootPath ? UI.backgroundLite : "transparent"
                radius: 4
            }

            Row {
                id: content
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4

                Text {
                    text: "Root"
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                    anchors.verticalCenter: parent.verticalCenter
                    color: UI.font

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: directoryModel.setActivePath(directoryModel.rootPath)
                    }
                }
            }
        }

        TreeView {
            id: directoryTree
            anchors.top: root.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            model: directoryModel
            clip: true

            property var chainToActive: []
            function calcChainToActive() { chainToActive = directoryModel.parentPathChain(directoryModel.activePath) }

            Component.onCompleted: calcChainToActive()
            Connections {
                target: directoryModel
                function onActivePathChanged() { directoryTree.calcChainToActive() }
                function onRowsInserted() { directoryTree.calcChainToActive() }
                function onModelReset() { directoryTree.calcChainToActive() }
            }

            delegate: Item {
                id: delegateItem
                required property string name
                required property string path
                required property bool hasChildren
                required property bool expanded
                required property int depth

                implicitHeight: row.implicitHeight + 8
                implicitWidth: directoryTree.width

                onPathChanged: updateExpansion()
                Component.onCompleted: updateExpansion()
                function updateExpansion() {
                    if (directoryModel.activePath === path) {
                        directoryTree.expand(index)
                        return
                    }
                    for (let p of directoryTree.chainToActive) {
                        if (p === path) {
                            directoryTree.expand(index)
                        }
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: directoryModel.activePath === delegateItem.path ? UI.backgroundLite : "transparent"
                    radius: 4
                }

                Row {
                    id: row
                    anchors.fill: parent
                    anchors.margins: 4
                    spacing: 4
                    leftPadding: 16 * delegateItem.depth

                    MouseArea {
                        id: expandArea
                        width: 12
                        height: parent.height
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            if(!delegateItem.expanded) directoryTree.expand(index)
                            else directoryTree.collapseRecursively(index)
                        }

                        Text {
                            anchors.centerIn: parent
                            text: delegateItem.hasChildren ? (delegateItem.expanded ? "\u2BC6" : "\u2BC8") : ""
                            color: UI.font
                        }
                    }

                    Text {
                        text: delegateItem.name
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        anchors.verticalCenter: parent.verticalCenter
                        color: UI.font
                        
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                directoryModel.setActivePath(delegateItem.path)
                                directoryTree.expand(index)
                            }
                        }
                    }
                }
            }
        }
    }

    GridView {
        id: photoGrid

        anchors.top: UI.autoHideToolbar ? parent.top : toolBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: sideBar.right
        anchors.right: parent.right

        cellWidth: width / itemsPerRow
        cellHeight: cellWidth //Always squared

        property int itemsPerRow: Math.ceil(width / UI.galleryTargetWidth)
        property int oldFirstRow: 0
        property int firstRow: Math.floor((visibleArea.yPosition * contentHeight) / cellHeight)
        property int firstVisibleIndex: firstRow * itemsPerRow
        property int lastVisibleIndex: (firstRow + height / cellHeight) * itemsPerRow + itemsPerRow - 1

        onFirstVisibleIndexChanged: requestThumbnails()
        onLastVisibleIndexChanged: requestThumbnails()

        onWidthChanged: requestThumbnails()
        onHeightChanged: requestThumbnails()

        function requestThumbnails()  {
            galleryModel.loadThumbnails(firstVisibleIndex, lastVisibleIndex, itemsPerRow, firstRow - oldFirstRow)
            oldFirstRow = firstRow
        }

        Shortcut {
            sequence: "Ctrl++"
            onActivated: {
                UI.galleryTargetWidth *= 5/4 //+1/4
            }
        }

        Shortcut {
            sequence: "Ctrl+-"
            onActivated: {
                UI.galleryTargetWidth *= 3/4 //-1/4
            }
        }

        model: galleryModel
        delegate: Item {
            width: GridView.view.cellWidth
            height: GridView.view.cellHeight
            clip: true  // important: clips content to square

            Image {
                anchors.centerIn: parent
                source: thumbPath ? Qt.url("file:///" + thumbPath) : ""
                fillMode: Image.PreserveAspectCrop // scales and crops to fill square
                width: parent.width
                height: parent.height

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        let component = Qt.createComponent("photo.qml")
                        let window = component.createObject()
                        window.provider = galleryModel.getProvider(index)
                        window.galleryModel = galleryModel
                        UI.photoWindowMaximized ? window.showMaximized() : window.show()
                    }
                }
            }
        }

        WheelHandler {
            id: zoomHandler
            acceptedModifiers: Qt.ControlModifier
            onWheel: (wheel) => {
                const factor = wheel.angleDelta.y > 0 ? 1.1 : 0.9
                UI.galleryTargetWidth *= factor
            }
        }

        Connections {
            target: galleryModel
            function onModelChanged() {
                galleryModel.loadThumbnails(photoGrid.firstVisibleIndex, photoGrid.lastVisibleIndex, photoGrid.itemsPerRow, 0)
            }
        }

        Component.onCompleted: requestThumbnails()
    }

    Rectangle {
        id: galleryLoadingScreen
        color: UI.background
        anchors.top: UI.autoHideToolbar ? parent.top : toolBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: sideBar.right
        anchors.right: parent.right

        opacity: 1.0
        Behavior on opacity { NumberAnimation { duration: 300 } }

        property int itemCount: galleryModel.size()
        property string albumName: directoryModel.getLongName(directoryModel.activePath)

        Text {
            anchors.centerIn: parent
            color: UI.font
            text: `Loading ${galleryLoadingScreen.itemCount >= 0 ? galleryLoadingScreen.itemCount : "your"} images from Album "${galleryLoadingScreen.albumName}"...`
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.AllButtons  //blocks all mouse buttons
            hoverEnabled: true
            cursorShape: Qt.BusyCursor
        }
    }

    Connections {
        target: galleryModel

        function onLoadingFinished() {
            galleryLoadingScreen.opacity = 0
            galleryLoadingScreen.visible = false
        }

        function onLoadingStarted() {
            galleryLoadingScreen.itemCount = galleryModel.size()
            galleryLoadingScreen.albumName = directoryModel.getLongName(directoryModel.activePath)
            galleryLoadingScreen.opacity = 1.0
            galleryLoadingScreen.visible = true
        }
    }

    Component.onCompleted: {
        UI.settingsModel = settingsModel
        UI.mainWindowWidth = window.width

        instantiateSettings()
        if(settingsModel.getValue("rootFolder") === "") window.settings.openRootFolderDialog()
    }

    function instantiateSettings() {
        let component = Qt.createComponent("settings.qml")
        let obj = component.createObject()
        window.settings = obj
    }
}

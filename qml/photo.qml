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
import Qt.labs.qmlmodels
import "."

ApplicationWindow {
    id: window
    visible: true
    width: 1000
    height: 600
    flags: Qt.Window | Qt.FramelessWindowHint
    title: provider ? provider.fileData ? provider.fileData.fileName : provider.filePath : "[not found]"
    color: UI.photoBackground === "black" ? "#000000" : UI.background
    font.pixelSize: 12
    palette: Palette {
        text: UI.font
        buttonText: UI.font
        windowText: UI.font
    }

    property var provider
    property var newProvider
    property var galleryModel

    function toggleMaximized() {
        let maximized = window.visibility === Window.Maximized
        if(maximized) window.showNormal()
        else window.showMaximized()
        UI.photoWindowMaximized = !maximized
    }

    //----- Keyboard Shortcuts -----//
    Shortcut {
        sequence: "Escape"
        onActivated: window.close()
    }
    Shortcut {
        sequence: "Right"
        onActivated: window.switchPhoto(1)
    }
    Shortcut {
        sequence: "Left"
        onActivated: window.switchPhoto(-1)
    }

    function switchPhoto(next) {
        if(next === 0) return;
        // End still running animations
        swipeNextEnd.cancel()
        swipePreviousEnd.cancel()
        swipeNext.cancel()
        swipePrevious.cancel()

        let nextIndex = window.galleryModel.getIndex(window.provider.filePath || "") + next
        window.newProvider = window.galleryModel.getProvider(nextIndex)

        let forward = next > 0
        if(!window.newProvider) {
            if(forward) swipeNextEnd.start()
            else swipePreviousEnd.start()
        }
        else {
            if(forward) swipeNext.start()
            else swipePrevious.start()
        }
    }

    function requestThumbnail() {
        if(window.provider.thumbPath) return

        let currentIndex = window.galleryModel.getIndex(window.provider.filePath || "")
        window.galleryModel.loadThumbnails(currentIndex, currentIndex, 1, 0)
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
                text: window.title
                toolTip: "Open in File Explorer"
                onClicked: if(window.provider.filePath) Qt.openUrlExternally("file:///" + window.provider.fileData.directoryPath)
            }

            MenuButton {
                text: "Metadata"
                toolTip: "Show Metadata Sidebar"
                onClicked: UI.showPhotoMetadata = !UI.showPhotoMetadata
            }

            // Spacer
            Item { Layout.fillWidth: true }

            // Window controls
            MenuButton { text: "—"; onClicked: window.showMinimized() }
            MenuButton { text: window.visibility === Window.Maximized ? "🗗" : "🗖"; onClicked: window.toggleMaximized() }
            MenuButton {
                text: "✕"
                onClicked: {
                    if(window.provider) window.provider.setActive(false);
                    window.close()
                }
            }
        }
    }

    Rectangle {
        id: sideBar
        width: UI.showPhotoMetadata ? 300 : 0
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        color: UI.background

        Behavior on width { NumberAnimation { duration: 100; easing.type: Easing.InOutQuad } }

        TableView {
            id: tableView
            anchors.top: parent.top
            anchors.topMargin: 32
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.bottom: parent.bottom

            visible: sideBar.width > 0

            property var columnRatios: [0.4, 0.6]

            columnWidthProvider: function(column) {
                return tableView.width * columnRatios[column];
            }

            model: TableModel {
                TableModelColumn { display: "name" }
                TableModelColumn { display: "value" }

                rows: buildRows()

                function buildRows() {
                    if(!window.provider) return []

                    return [
                        { "name": "File Name", "value": window.provider.fileData.fileName },
                        { "name": "File Path", "value": window.provider.fileData.filePath },
                        { "name": "Software", "value": window.provider.exifData.software },
                        { "name": "Camera Maker",  "value": window.provider.exifData.maker },
                        { "name": "Camera Model",  "value": window.provider.exifData.cameraModel },
                        { "name": "Lens Model", "value": window.provider.exifData.lensModel },
                        { "name": "Date", "value": window.provider.exifData.dateTaken },
                        { "name": "ISO", "value": window.provider.exifData.iso },
                        { "name": "Aperture", "value": window.provider.exifData.aperture.formatted },
                        { "name": "Focal Length", "value": window.provider.exifData.focalLength.formatted },
                        { "name": "Exposure Time", "value": window.provider.exifData.exposureTime.formatted },
                        { "name": "Exposure Compensation", "value": window.provider.exifData.exposureBias.formatted },
                        { "name": "Width", "value": window.provider.exifData.width },
                        { "name": "Height", "value": window.provider.exifData.height }
                    ]
                }
            }

            delegate: Rectangle {
                implicitHeight: cell.implicitHeight + 8
                color: UI.background
                clip: true

                Text {
                    id: cell
                    anchors.fill: parent
                    anchors.margins: 4
                    wrapMode: Text.Wrap
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    color: UI.font
                    // qmllint disable unqualified
                    text: model.display
                    // qmllint enable unqualified
                }
            }
        }
    }

    Rectangle {
        id: content
        anchors.top: UI.autoHideToolbar ? parent.top : toolBar.bottom
        anchors.bottom: parent.bottom
        anchors.left: sideBar.right
        anchors.right: parent.right

        color: "transparent"
        clip: true

        Image {
            id: photo
            width: parent.width
            height: parent.height
            z: 2 //over nextPhoto
            smooth: true
            fillMode: Image.PreserveAspectFit

            source: window.provider?.thumbPath ? window.provider.loadedPath ? Qt.url("file:///" + window.provider.loadedPath) : Qt.url("file:///" + window.provider.thumbPath) : ""
            transform: [
                Scale {
                    id: zoom
                    xScale: scale
                    yScale: scale
                    property real scale: 1
                },
                Translate {
                    id: pan
                    x: 0
                    y: 0
                }
            ]

            // Start centered
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2

            function resetTransform() {
                if(zoom.scale === 1) return
                zoom.scale = 1
                pan.x = 0
                pan.y = 0
            }
        }

        Image {
            id: nextPhoto
            width: parent.width
            height: parent.height
            smooth: true
            fillMode: Image.PreserveAspectFit
            source: window.newProvider?.thumbPath ? window.newProvider.loadedPath ? Qt.url("file:///" + window.newProvider.loadedPath) : Qt.url("file:///" + window.newProvider.thumbPath) : ""
        }

        //----- Swipe Animation -----//
        SwipeAnimation {
            id: swipeNext
            photo: photo
            nextPhoto: nextPhoto
            direction: 1
            onFinish: function() { window.provider = window.newProvider; window.newProvider = null }
            onCompleted: window.requestThumbnail
        }

        SwipeAnimation {
            id: swipePrevious
            photo: photo
            nextPhoto: nextPhoto
            direction: -1
            onFinish: function() { window.provider = window.newProvider; window.newProvider = null }
            onCompleted: window.requestThumbnail
        }

        SwipeAnimation {
            id: swipeNextEnd
            photo: photo
            direction: 1
            isEnd: true
        }

        SwipeAnimation {
            id: swipePreviousEnd
            photo: photo
            direction: -1
            isEnd: true
        }
        

        //----- Photo Zooming/Panning -----//
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            hoverEnabled: true
            cursorShape: zoom.scale === 1 ? Qt.ArrowCursor : Qt.OpenHandCursor

            Binding {
                target: mouseArea
                property: "cursorShape"
                value: zoom.scale === 1 ? Qt.ArrowCursor : Qt.OpenHandCursor
                when: !mouseArea.pressed
            }

            property real lastX
            property real lastY
            property real minScale: 1
            property real maxScale: 5

            onWheel: (e) => handleZoom(e.x, e.y, e.angleDelta.y)
            onPressed: (e) => { lastX = e.x; lastY = e.y; cursorShape = Qt.ClosedHandCursor }
            onReleased: cursorShape = (zoom.scale === 1 ? Qt.ArrowCursor : Qt.OpenHandCursor)
            onPositionChanged: (e) => handlePan(e)

            function handleZoom(centerX, centerY, delta) {
                let factor = delta > 0 ? 1.1 : 0.9
                let newScale = Math.max(minScale, Math.min(maxScale, zoom.scale * factor))
                let scaleFactor = newScale / zoom.scale

                // Adjust position so the point under the cursor stays fixed
                pan.x = centerX - scaleFactor * (centerX - pan.x)
                pan.y = centerY - scaleFactor * (centerY - pan.y)

                zoom.scale = newScale
                constrainPosition()
            }

            function handlePan(e) {
                if (e.buttons & Qt.LeftButton) {
                    pan.x += e.x - lastX
                    pan.y += e.y - lastY
                    lastX = e.x
                    lastY = e.y
                    constrainPosition()
                }
            }

            function constrainPosition() {
                let scaledWidth = photo.width * zoom.xScale
                let scaledHeight = photo.height * zoom.yScale

                // Horizontal
                if (scaledWidth <= parent.width) {
                    // Center if smaller
                    pan.x = (parent.width - scaledWidth) / 2
                } else {
                    // Clamp edges
                    if (pan.x > 0) pan.x = 0
                    if (pan.x < parent.width - scaledWidth) pan.x = parent.width - scaledWidth
                }

                // Vertical
                if (scaledHeight <= parent.height) {
                    // Center if smaller
                    pan.y = (parent.height - scaledHeight) / 2
                } else {
                    // Clamp edges
                    if (pan.y > 0) pan.y = 0
                    if (pan.y < parent.height - scaledHeight) pan.y = parent.height - scaledHeight
                }
            }
        }

        PinchArea {
            anchors.fill: parent
            pinch.minimumScale: mouseArea.minScale
            pinch.maximumScale: mouseArea.maxScale
            pinch.target: null

            property real startScale: 1
            property real startX: 0
            property real startY: 0
            property real lastCenterX: 0
            property real lastCenterY: 0

            onPinchStarted: (e) => {
                startScale = zoom.scale
                startX = pan.x
                startY = pan.y
                lastCenterX = e.center.x
                lastCenterY = e.center.y
            }

            onPinchUpdated: (e) => {
                let newScale = Math.max(pinch.minimumScale, Math.min(pinch.maximumScale, startScale * e.scale))
                let scaleFactor = newScale / zoom.scale

                // Adjust pan for scaling
                pan.x = e.center.x - scaleFactor * (lastCenterX - pan.x)
                pan.y = e.center.y - scaleFactor * (lastCenterY - pan.y)

                // Track the last center for smooth panning
                lastCenterX = e.center.x
                lastCenterY = e.center.y

                zoom.scale = newScale
                mouseArea.constrainPosition()
            }

            onPinchFinished: (e) => {
                mouseArea.constrainPosition()
            }
        }
    }
}
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
import "."

Rectangle {
    id: shell
    height: 32
    width: window.width
    color: "transparent"
    anchors.top: window.top
    z: 99

    property var window: Window.window
    default property alias content: toolBar.data //All children should be attached to toolBar

    property bool show: !UI.autoHideToolbar
    property bool showLocked: !UI.autoHideToolbar
    onShowLockedChanged: if(show && !showLocked) hideTimer.restart()

    // Hover area at top edge
    MouseArea {
        id: topHover
        height: 32
        width: parent.width
        anchors.top: parent.top
        hoverEnabled: true
        onEntered: shell.show = true
    }

    Rectangle {
        id: toolBar
        height: shell.show ? 32 : 0
        opacity: shell.show ? 1.0 : 0.0
        width: parent.width
        color: UI.background
        anchors.top: shell.top

        Behavior on opacity { NumberAnimation { duration: 200 } }
        Behavior on height { NumberAnimation { duration: 200 } }     

        // Check Mouse inside toolBar
        HoverHandler {
            id: toolBarArea
            acceptedDevices: PointerDevice.Mouse
            onHoveredChanged: {
                if (hovered) {
                    hideTimer.stop()
                } else {
                    hideTimer.restart()
                }
            }
        }

        Timer {
            id: hideTimer
            interval: 300
            repeat: false
            onTriggered: {
                if (!toolBarArea.hovered && !topHover.containsMouse && !shell.showLocked)
                    shell.show = false
            }
        }

        DragHandler {
            onActiveChanged: if(active) {
                shell.window.startSystemMove()
                if(shell.window.visibility === Window.Maximized) shell.window.visibility = Window.Windowed
            }
            target: null
        }

        TapHandler {
            onDoubleTapped: shell.window.toggleMaximized()
        }
    }
}
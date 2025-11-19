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

Button {
    id: control
    leftPadding: 10
    rightPadding: 10

    Layout.fillHeight: true

    property string toolTip

    ToolTip {
        text: control.toolTip
        delay: 1000
        timeout: 5000
        visible: control.hovered && text !== ""

        background: Rectangle {
            color: UI.backgroundLite
            radius: 6
            border.color: UI.font
            border.width: 1
        }

        contentItem: Text {
            text: control.toolTip
            color: UI.font
            font.pixelSize: 12
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Rectangle {
        color: control.down || control.hovered ? UI.backgroundLite : "transparent"
    }
    contentItem: Text {
        text: control.text
        color: UI.font
        anchors.centerIn: parent
        verticalAlignment: Text.AlignVCenter
        font: control.font
    }

    HoverHandler {
        cursorShape: Qt.PointingHandCursor
    }
}
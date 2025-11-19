/*
* Lysa - Photo Organizer
* Copyright (C) 2025 vorks. DEV (Jeremy Voß)
* 
* This file is part of Lysa.
* 
* Licensed under GNU GPL-3
* Full license: see LICENSE.txt
*/

import QtQuick 2.15

Item {
    id: swipe

    // Custom properties
    property var photo
    property var nextPhoto
    property real direction: 1 // 1 = next, -1 = previous
    property bool isEnd: false // false = full swipe, true = short end bounce
    property int duration: 250
    property var onFinish: null
    property bool cancelled: false
    property var onCompleted: null

    function start() {
        if(!isEnd) fullSwipe.start()
        else endBounce.start()
    }

    function stop() {
        if(!isEnd) fullSwipe.stop()
        else endBounce.stop()
    }

    function complete() {
        if(!isEnd) fullSwipe.complete()
        else endBounce.complete()
    }

    // Used to cancel previous Animation before starting a new one
    function cancel() {
        cancelled = true
        if(!isEnd) fullSwipe.complete()
        else endBounce.stop()
    }

    property bool running: isEnd ? fullSwipe.running : endBounce.running

    SequentialAnimation {
        id: fullSwipe
        ScriptAction {
            script: {
                photo.resetTransform()
                nextPhoto.x = direction * nextPhoto.width
            }
        }
        ParallelAnimation {
            NumberAnimation {
                target: swipe.photo
                property: "x"
                to: -swipe.direction * swipe.photo.width
                duration: swipe.duration
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: swipe.nextPhoto ? swipe.nextPhoto : null
                property: "x"
                to: 0
                duration: swipe.duration
                easing.type: Easing.InOutQuad
            }
        }
        ScriptAction {
            script: {
                if(swipe.onFinish) swipe.onFinish()
                if(!swipe.cancelled && swipe.onCompleted) swipe.onCompleted()
                photo.x = 0
            }
        }
    }

    SequentialAnimation {
        id: endBounce
        ScriptAction {
            script: {
                photo.resetTransform()
            }
        }
        NumberAnimation {
            target: swipe.photo
            property: "x"
            to: -swipe.direction * swipe.photo.width / 4
            duration: swipe.duration
            easing.type: Easing.InOutQuad
        }
        NumberAnimation {
            target: swipe.photo
            property: "x"
            to: 0
            duration: swipe.duration
            easing.type: Easing.InOutQuad
        }
    }
}

/*
* Lysa - Photo Organizer
* Copyright (C) 2025 vorks. DEV (Jeremy Voß)
* 
* This file is part of Lysa.
* 
* Licensed under GNU GPL-3
* Full license: see LICENSE.txt
*/

pragma Singleton
import QtQuick

QtObject {
    //----- Property Initialization -----//
    property string currentTheme: "dark"
    property bool autoHideToolbar: false

    property bool showSidebar: false
    onShowSidebarChanged: settingsModel.setValue("showSidebar", showSidebar)

    property int galleryTargetWidth: 200
    onGalleryTargetWidthChanged: {
        galleryTargetWidth = Math.min(galleryTargetWidth, mainWindowWidth) //Maximum of windowWidth
        galleryTargetWidth = Math.max(galleryTargetWidth, mainWindowWidth * 0.02) //Minimum of 2% of windowWidth
        settingsModel.setValue("galleryTargetWidth", galleryTargetWidth)
    }

    property bool photoWindowMaximized: false
    onPhotoWindowMaximizedChanged: settingsModel.setValue("photoWindowMaximized", photoWindowMaximized)
    property string photoBackground: "black"
    property bool showPhotoMetadata: false
    onShowPhotoMetadataChanged: settingsModel.setValue("showPhotoMetadata", showPhotoMetadata)

    property int mainWindowWidth

    //----- settingsModel initialization -----//
    property var settingsModel: null
    onSettingsModelChanged: {
        if (!settingsModel) {
            console.warn("UI.qml: settingsModel not set")
            return
        }

        // Load initial values after settingsModel is set
        Qt.callLater(() => {
            currentTheme = settingsModel.getValue("theme")
            autoHideToolbar = settingsModel.getValue("autoHideToolbar")
            showSidebar = settingsModel.getValue("showSidebar")
            galleryTargetWidth = settingsModel.getValue("galleryTargetWidth")
            photoWindowMaximized = settingsModel.getValue("photoWindowMaximized")
            photoBackground = settingsModel.getValue("photoBackground")
            showPhotoMetadata = settingsModel.getValue("showPhotoMetadata")
        })

        // Connect for runtime changes
        settingsModel.settingChanged.connect(function(id, value) {
            switch (id) {
                case "theme": currentTheme = value; break
                case "autoHideToolbar": autoHideToolbar = value; break
                case "showSidebar": showSidebar = value; break
                case "galleryTargetWidth": galleryTargetWidth = value; break
                case "photoWindowMaximized": photoWindowMaximized = value; break
                case "photoBackground": photoBackground = value; break
                case "showPhotoMetadata": showPhotoMetadata = value; break
            }
        })
    }


    //----- THEMES -----//
    readonly property var themes: ({
        "dark": {
            "background": "#121216",
            "backgroundLite": "#2E2E38",
            "font": "#fefefe"
        },
        "light": {
            "background": "#ffffff",
            "backgroundLite": "#f0f0f0",
            "font": "#000000"
        }
    })

    property color main: "#A7D49B"
    property color background: themes[currentTheme].background
    property color backgroundLite: themes[currentTheme].backgroundLite
    property color font: themes[currentTheme].font
}

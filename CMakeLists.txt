cmake_minimum_required(VERSION 3.16)
project(lysa LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt automatic features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 Library
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Widgets Concurrent)

# Exiv2 Library
find_package(Exiv2 CONFIG REQUIRED)

qt_standard_project_setup()

# Add the executable
qt_add_executable(lysa WIN32
    src/main.cpp
    src/structs.h
    src/appsettings.cpp
    src/appsettings.h
    src/photocontroller.cpp
    src/photocontroller.h
    src/photomodel.cpp
    src/photomodel.h
    src/exifregistry.cpp
    src/exifregistry.h
    src/thumbnailworker.cpp
    src/thumbnailworker.h
    src/photoprovider.cpp
    src/photoprovider.h
    src/gallerymodel.cpp
    src/gallerymodel.h
    src/directorymodel.cpp
    src/directorymodel.h
    src/fileservice.h
    qml/qml.qrc
)

if(WIN32)
    # Add the icon resource to the executable
    set(ICON_RC "${CMAKE_CURRENT_SOURCE_DIR}/resources/icon.rc")
    target_sources(lysa PRIVATE ${ICON_RC})
endif()

# Link libraries
target_link_libraries(lysa
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
        Qt6::Concurrent
        Exiv2::exiv2lib
)

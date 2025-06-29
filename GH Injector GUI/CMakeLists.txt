cmake_minimum_required(VERSION 3.16)

project(GuidedHackingGui VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets)

set(PROJECT_SOURCES
    GuiMain.h
    GuiMain.cpp
    pch.h
    pch.cpp
    GuiMain.qrc
    GuiMain.ui
    GuiMain.h
    GuiMain.cpp
    framelesswindow.qrc
    framelesswindow/framelesswindow.ui
    framelesswindow/framelesswindow.h
    framelesswindow/framelesswindow.cpp
)

add_executable(GuidedHackingGui
    ${PROJECT_SOURCES}
)

target_link_libraries(GuidedHackingGui
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
)

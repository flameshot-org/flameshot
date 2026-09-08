cmake_minimum_required(VERSION 3.16)
project(qrplugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_library(qrplugin MODULE
    qrplugin.h
    qrplugin.cpp
    qrtool.h
    qrtool.cpp
    qrwidget.h
    qrwidget.cpp
    qrresources.qrc
)

target_include_directories(qrplugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(qrplugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    /lib/x86_64-linux-gnu/libzbar.so.0
)

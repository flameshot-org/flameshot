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

find_library(ZBAR_LIB
    NAMES zbar libzbar.so.0
    HINTS
        /usr/lib
        /usr/lib/x86_64-linux-gnu
        /usr/local/lib
        /lib/x86_64-linux-gnu
        /usr/lib/aarch64-linux-gnu
        /usr/lib/arm-linux-gnueabihf
)

if(NOT ZBAR_LIB)
    message(FATAL_ERROR
        "libzbar not found. Install it with:\n"
        "  Debian/Ubuntu: sudo apt install libzbar-dev\n"
        "  Fedora:        sudo dnf install zbar-devel\n"
        "  Arch:          sudo pacman -S zbar\n"
        "  macOS:         brew install zbar")
endif()

target_link_libraries(qrplugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    ${ZBAR_LIB}
)

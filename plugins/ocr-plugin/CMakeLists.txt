cmake_minimum_required(VERSION 3.16)
project(ocrplugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Tesseract REQUIRED IMPORTED_TARGET tesseract)
pkg_check_modules(Leptonica REQUIRED IMPORTED_TARGET lept)

add_library(ocrplugin MODULE
    ocrplugin.h
    ocrplugin.cpp
    ocrtool.h
    ocrtool.cpp
    ocrwidget.h
    ocrwidget.cpp
    ocrresources.qrc
)

target_include_directories(ocrplugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(ocrplugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    PkgConfig::Tesseract
    PkgConfig::Leptonica
)

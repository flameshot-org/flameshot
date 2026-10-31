cmake_minimum_required(VERSION 3.16)
project(sampleplugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_library(sampleplugin MODULE
    sampleplugin.h
    sampleplugin.cpp
)

target_include_directories(sampleplugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

target_link_libraries(sampleplugin PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)

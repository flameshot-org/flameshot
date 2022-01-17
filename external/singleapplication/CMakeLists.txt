cmake_minimum_required(VERSION 3.7.0)

project(SingleApplication LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

add_library(${PROJECT_NAME} STATIC
    singleapplication.cpp
    singleapplication_p.cpp
)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

if(NOT QT_DEFAULT_MAJOR_VERSION)
    set(QT_DEFAULT_MAJOR_VERSION 5 CACHE STRING "Qt version to use (5 or 6), defaults to 5")
endif()

# Find dependencies
set(QT_COMPONENTS Core Network)
set(QT_LIBRARIES Qt${QT_DEFAULT_MAJOR_VERSION}::Core Qt${QT_DEFAULT_MAJOR_VERSION}::Network)

if(QAPPLICATION_CLASS STREQUAL QApplication)
    list(APPEND QT_COMPONENTS Widgets)
    list(APPEND QT_LIBRARIES Qt${QT_DEFAULT_MAJOR_VERSION}::Widgets)
elseif(QAPPLICATION_CLASS STREQUAL QGuiApplication)
    list(APPEND QT_COMPONENTS Gui)
    list(APPEND QT_LIBRARIES Qt${QT_DEFAULT_MAJOR_VERSION}::Gui)
else()
    set(QAPPLICATION_CLASS QCoreApplication)
endif()

find_package(Qt${QT_DEFAULT_MAJOR_VERSION} COMPONENTS ${QT_COMPONENTS} REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC ${QT_LIBRARIES})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE advapi32)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC QAPPLICATION_CLASS=${QAPPLICATION_CLASS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

cmake_minimum_required(VERSION 3.7.0)

project(SingleApplication LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

add_library(${PROJECT_NAME} STATIC
    singleapplication.cpp
    singleapplication_p.cpp
)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# Find dependencies
find_package(Qt5 COMPONENTS Network REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Network)

if(QAPPLICATION_CLASS STREQUAL QApplication)
    find_package(Qt5 COMPONENTS Widgets REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Widgets)
elseif(QAPPLICATION_CLASS STREQUAL QGuiApplication)
    find_package(Qt5 COMPONENTS Gui REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Gui)
else()
    set(QAPPLICATION_CLASS QCoreApplication)
    find_package(Qt5 COMPONENTS Core REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core)
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE advapi32)
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC QAPPLICATION_CLASS=${QAPPLICATION_CLASS})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

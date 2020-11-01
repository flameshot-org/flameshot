cmake_minimum_required(VERSION 3.7.0)

project(sending_arguments LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

# SingleApplication base class
set(QAPPLICATION_CLASS QCoreApplication)

add_executable(${PROJECT_NAME}
    main.cpp
    messagereceiver.cpp
    messagereceiver.h
    main.cpp
)

find_package(Qt5 COMPONENTS Core REQUIRED)
add_subdirectory(../.. SingleApplication)
target_link_libraries(${PROJECT_NAME} SingleApplication::SingleApplication)

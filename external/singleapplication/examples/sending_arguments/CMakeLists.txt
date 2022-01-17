cmake_minimum_required(VERSION 3.7.0)

project(sending_arguments LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

# SingleApplication base class
set(QAPPLICATION_CLASS QCoreApplication)
add_subdirectory(../.. SingleApplication)

find_package(Qt${QT_DEFAULT_MAJOR_VERSION} COMPONENTS Core REQUIRED)

add_executable(${PROJECT_NAME}
    main.cpp
    messagereceiver.cpp
    messagereceiver.h
    main.cpp
)

target_link_libraries(${PROJECT_NAME} SingleApplication::SingleApplication)

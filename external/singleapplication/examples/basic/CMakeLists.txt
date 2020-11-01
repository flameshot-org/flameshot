cmake_minimum_required(VERSION 3.7.0)

project(basic LANGUAGES CXX)

# SingleApplication base class
set(QAPPLICATION_CLASS QCoreApplication)

add_executable(basic main.cpp)

find_package(Qt5 COMPONENTS Core REQUIRED)
add_subdirectory(../.. SingleApplication)
target_link_libraries(${PROJECT_NAME} SingleApplication::SingleApplication)


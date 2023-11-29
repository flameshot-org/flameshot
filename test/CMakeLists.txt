cmake_minimum_required(VERSION 3.13)
project(FlameshotTest LANGUAGES CXX)

# Find the Qt5Test library
find_package(Qt5Test REQUIRED)

# Handle generating the .moc file automatically
set(CMAKE_AUTOMOC ON)

# Define your test executable
add_executable(testexample TestExample.cpp)

# Link your test executable with the Qt5Test library
target_link_libraries(testexample Qt5::Test)


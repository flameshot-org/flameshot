cmake_minimum_required(VERSION 3.16)
project(flameshot-action-plugin LANGUAGES CXX)
add_executable(flameshot-plugin src/main.cpp)
target_compile_features(flameshot-plugin PRIVATE cxx_std_17)

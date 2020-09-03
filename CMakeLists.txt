cmake_minimum_required(VERSION 3.13)

project(
  flameshot
  VERSION 0.8.0
  LANGUAGES CXX)

include(cmake/StandardProjectSettings.cmake)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
# set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
# enable_sanitizers(project_options)

# allow for static analysis options include(cmake/StaticAnalyzers.cmake)

add_subdirectory(src)

# Copyright (C) 2013-2020 Mattia Basaglia
#
#
# This software is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Color Widgets.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required (VERSION 3.1 FATAL_ERROR)

project(QtColorWidgetsPlugin CXX)


set (CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_SOURCE_DIR}/cmake/modules
  ${CMAKE_CURRENT_LIST_DIR}/..)


include (CheckCXXCompilerFlag)

include (../cmake/versioning.cmake)


SET (${PROJECT_NAME}_VERSION_MAJOR 2)
SET (${PROJECT_NAME}_VERSION_MINOR 2)
SET (${PROJECT_NAME}_VERSION_PATCH 0)
SET (${PROJECT_NAME}_VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}.${${PROJECT_NAME}_VERSION_PATCH}")


find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Designer REQUIRED)

# Auto generate moc files
set(CMAKE_AUTOMOC ON)
# Auto generate moc files
set(CMAKE_AUTOUIC ON)
# Auto generate moc files
set(CMAKE_AUTORCC ON)

check_cxx_compiler_flag ("-Wall" Wall_FLAG_SUPPORTED)

if (Wall_FLAG_SUPPORTED)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
endif (Wall_FLAG_SUPPORTED)

check_cxx_compiler_flag ("-pedantic" pedantic_FLAG_SUPPORTED)

if (pedantic_FLAG_SUPPORTED)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pedantic")
endif (pedantic_FLAG_SUPPORTED)

check_cxx_compiler_flag ("-Wextra" Wextra_FLAG_SUPPORTED)

if (Wextra_FLAG_SUPPORTED)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wextra")
endif (Wextra_FLAG_SUPPORTED)

# Library
set (TARGET_NAME ${PROJECT_NAME})
set (TARGET_OUTPUT_SUFFIX "-Qt${QT_VERSION}${${PROJECT_NAME}_VERSION_MAJOR}")

add_library (${TARGET_NAME} SHARED "")
set_target_properties(${TARGET_NAME}
  PROPERTIES
  EXPORT_NAME "${TARGET_NAME}${TARGET_OUTPUT_SUFFIX}")

set_target_properties (${TARGET_NAME}
  PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON
  VERSION ${${PROJECT_NAME}_VERSION}
  SOVERSION ${${PROJECT_NAME}_VERSION_MAJOR}
  "INTERFACE_${PROJECT_NAME}_MAJOR_VERSION" ${${PROJECT_NAME}_VERSION_MAJOR}
  COMPATIBLE_INTERFACE_STRING "${PROJECT_NAME}_MAJOR_VERSION"
  OUTPUT_NAME "${TARGET_NAME}${TARGET_OUTPUT_SUFFIX}")


target_link_libraries(${TARGET_NAME}
  PRIVATE
  ${COLOR_WIDGETS_LIBRARY}
  Qt${QT_VERSION_MAJOR}::Designer
  )


# Sources
set(SOURCES
  color_preview_plugin.cpp
  color_wheel_plugin.cpp
  color_widget_plugin_collection.cpp
  gradient_slider_plugin.cpp
  hue_slider_plugin.cpp
  color_selector_plugin.cpp
  color_list_plugin.cpp
  swatch_plugin.cpp
  color_palette_widget_plugin.cpp
  color_2d_slider_plugin.cpp
  color_line_edit_plugin.cpp
  gradient_editor_plugin.cpp
  # add new sources above this line
  )

foreach (SOURCE IN LISTS SOURCES)
  target_sources (${TARGET_NAME}
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${SOURCE}>)
endforeach (SOURCE IN SOURCES)

set(HEADERS
  color_preview_plugin.hpp
  color_wheel_plugin.hpp
  color_widget_plugin_collection.hpp
  gradient_slider_plugin.hpp
  hue_slider_plugin.hpp
  color_selector_plugin.hpp
  color_list_plugin.hpp
  swatch_plugin.hpp
  color_palette_widget_plugin.hpp
  color_2d_slider_plugin.hpp
  color_line_edit_plugin.hpp
  gradient_editor_plugin.hpp
  # add new headers above this line
  )

foreach (HEADER IN LISTS HEADERS)
  target_sources (${TARGET_NAME}
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/${HEADER}>)
endforeach (HEADER IN HEADERS)


# install
get_target_property (QT_QMAKE_EXECUTABLE
  Qt${QT_VERSION_MAJOR}::qmake LOCATION)

execute_process (
  COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
  OUTPUT_VARIABLE QT_INSTALL_PLUGINS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# install(TARGETS ${COLOR_WIDGETS_PLUGIN} DESTINATION ${QT_INSTALL_PLUGINS}/designer OPTIONAL)

execute_process (
  COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_LIBS
  OUTPUT_VARIABLE QT_INSTALL_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

# install(TARGETS ${COLOR_WIDGETS_PLUGIN} DESTINATION ${QT_INSTALL_LIBS}/qtcreator/plugins OPTIONAL)

add_custom_target(${TARGET_NAME}_install
    COMMAND cp $<TARGET_FILE:${TARGET_NAME}> ${QT_INSTALL_PLUGINS}/designer
    COMMAND cp $<TARGET_FILE:${TARGET_NAME}> ${QT_INSTALL_LIBS}/qtcreator/plugins
    DEPENDS ${TARGET_NAME}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

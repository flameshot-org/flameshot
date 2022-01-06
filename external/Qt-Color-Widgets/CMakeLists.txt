#
# Copyright (C) 2013-2020 Mattia Basaglia
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
cmake_minimum_required (VERSION 3.1 FATAL_ERROR)
if ( ${CMAKE_VERSION} VERSION_GREATER "3.12.0")
    cmake_policy(SET CMP0077 NEW)
endif()

set(COLORWIDGET_PROJECT_NAME QtColorWidgets)
project(${COLORWIDGET_PROJECT_NAME} CXX)


set (CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
  ${CMAKE_CURRENT_LIST_DIR})


include (CheckCXXCompilerFlag)

include (cmake/install.cmake)
include (cmake/versioning.cmake)


SET (${COLORWIDGET_PROJECT_NAME}_VERSION_MAJOR 2)
SET (${COLORWIDGET_PROJECT_NAME}_VERSION_MINOR 2)
SET (${COLORWIDGET_PROJECT_NAME}_VERSION_PATCH 0)
SET (${COLORWIDGET_PROJECT_NAME}_VERSION "${${COLORWIDGET_PROJECT_NAME}_VERSION_MAJOR}.${${COLORWIDGET_PROJECT_NAME}_VERSION_MINOR}.${${COLORWIDGET_PROJECT_NAME}_VERSION_PATCH}")




set(QT_VERSION_MAJOR 5)
find_package(QT NAMES Qt5 Qt4 COMPONENTS Core QUIET)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets REQUIRED)


# Auto generate moc files
set(CMAKE_AUTOMOC ON)
# Auto generate moc files
set(CMAKE_AUTOUIC ON)
# Auto generate moc files
set(CMAKE_AUTORCC ON)


set (TARGET_NAME ${COLORWIDGET_PROJECT_NAME})
option(COLOR_WIDGETS_QT_SUFFIX "Append qt version to the target output files" OFF)
if ( COLOR_WIDGETS_QT_SUFFIX )
    set (TARGET_OUTPUT_SUFFIX "-Qt${QT_VERSION}${${COLORWIDGET_PROJECT_NAME}_VERSION_MAJOR}")
else()
    set (TARGET_OUTPUT_SUFFIX "")
endif()

set (INCLUDE_PREFIX "QtColorWidgets")
set (COLOR_WIDGETS_LIBRARY "${TARGET_NAME}")

add_definitions(-DQTCOLORWIDGETS_STATICALLY_LINKED)


add_library (${TARGET_NAME} STATIC "")
set_target_properties(${TARGET_NAME}
  PROPERTIES
  EXPORT_NAME "${TARGET_NAME}${TARGET_OUTPUT_SUFFIX}")


set_target_properties (${TARGET_NAME}
  PROPERTIES
  CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON
  VERSION ${${COLORWIDGET_PROJECT_NAME}_VERSION}
  SOVERSION ${${COLORWIDGET_PROJECT_NAME}_VERSION_MAJOR}
  "INTERFACE_${COLORWIDGET_PROJECT_NAME}_MAJOR_VERSION" ${${COLORWIDGET_PROJECT_NAME}_VERSION_MAJOR}
  COMPATIBLE_INTERFACE_STRING "${COLORWIDGET_PROJECT_NAME}_MAJOR_VERSION"
  COMPILE_DEFINITIONS QTCOLORWIDGETS_LIBRARY
  OUTPUT_NAME "${TARGET_NAME}${TARGET_OUTPUT_SUFFIX}")

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


set (EXPORT_HEADER "${INCLUDE_PREFIX}/colorwidgets_global.hpp")


generate_versioning_information (
    TARGET_NAME ${TARGET_NAME}
    EXPORT_HEADER ${EXPORT_HEADER}
    EXPORT_MACRO QCP_EXPORT
    VERSIONED_ENTITY ${COLORWIDGET_PROJECT_NAME}
    INCLUDE_PREFIX ${INCLUDE_PREFIX}
    COMPANY_NAME "Mattia Basaglia"
    COMPANY_COPYRIGHT "Mattia Basaglia Copyright (C) 2013-2020"
    FILE_DESCRIPTION "Color wheel widget and dialog for Qt${QT_VERSION}"
)


target_include_directories(${TARGET_NAME}
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PUBLIC
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>
  $<INSTALL_INTERFACE:include>
  )


add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/include/${INCLUDE_PREFIX})
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/resources/${INCLUDE_PREFIX})
add_subdirectory (${CMAKE_CURRENT_SOURCE_DIR}/src/${INCLUDE_PREFIX})


target_link_libraries (${TARGET_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)


install_project (
  PROJECT_NAME ${COLORWIDGET_PROJECT_NAME}
  TARGET_NAME ${TARGET_NAME}
  TARGET_OUTPUT_SUFFIX "${TARGET_OUTPUT_SUFFIX}"
  EXPORT_HEADER ${EXPORT_HEADER}
  INCLUDE_PREFIX ${INCLUDE_PREFIX}
  HEADER_MATCHING_REGEX "^.*\.h$|^.*\.hpp$|^.*$"
  VERSION_HEADER ${CMAKE_CURRENT_BINARY_DIR}/${COLORWIDGET_PROJECT_NAME}_version.h
  NAMESPACE "")


add_subdirectory (gallery)

option(QTCOLORWIDGETS_DESIGNER_PLUGIN "Build QtDesigner plugin" ON)
if (${QTCOLORWIDGETS_DESIGNER_PLUGIN})
    find_package (Qt5Designer QUIET)
    if (Qt5Designer_FOUND)
        add_subdirectory (color_widgets_designer_plugin)
    endif(Qt5Designer_FOUND)
endif()

configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
add_custom_target(
    ${COLORWIDGET_PROJECT_NAME}_docs
    COMMAND doxygen Doxyfile
    COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/gallery/*.png ${CMAKE_CURRENT_BINARY_DIR}/doxygen/html
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

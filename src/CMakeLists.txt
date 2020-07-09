find_package(
  Qt5
  CONFIG
  REQUIRED
  Core
  Gui
  Widgets
  Network
  Svg
  DBus)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(flameshot)

add_subdirectory(cli)
add_subdirectory(config)
add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(widgets)
add_subdirectory(tools)

target_sources(
  flameshot
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../external/singleapplication/singleapplication.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/../external/Qt-Color-Widgets/src/color_utils.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/../external/Qt-Color-Widgets/src/color_wheel.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/../external/Qt-Color-Widgets/include/color_wheel.hpp
          ${CMAKE_CURRENT_SOURCE_DIR}/../data/graphics.qrc
          main.cpp)

target_include_directories(
  flameshot
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../external/singleapplication/>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../external/Qt-Color-Widgets/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../dbus/>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/cli>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/config>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/arrow>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/blur>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/circle>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/circlecount>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/copy>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/exit>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/imgur>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/launcher>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/line>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/marker>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/move>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/pencil>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/pin>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/rectangle>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/redo>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/save>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/selection>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/sizeindicator>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/text>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/undo>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/panel>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/widgets/capture>
         $<INSTALL_INTERFACE:include/mylib> # <prefix>/include/mylib
)

target_link_libraries(
  flameshot
  project_warnings
  project_options
  Qt5::Svg
  Qt5::DBus
  Qt5::Network
  Qt5::Widgets)

target_compile_definitions(flameshot PRIVATE APP_PREFIX="/usr")
target_compile_definitions(flameshot PRIVATE APP_VERSION="v0.8.0")
target_compile_definitions(flameshot PRIVATE IMGUR_CLIENT_ID="313baf0c7b4d3ff")
target_compile_definitions(flameshot PRIVATE QAPPLICATION_CLASS=QApplication)

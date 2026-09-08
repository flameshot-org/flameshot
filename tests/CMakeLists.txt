find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Gui)

add_executable(
  actionplugin-manifest-test
  actionplugin_manifest_test.cpp
  ${CMAKE_SOURCE_DIR}/src/plugins/actionplugin.cpp
  ${CMAKE_SOURCE_DIR}/src/plugins/actionplugin.h)

target_include_directories(
  actionplugin-manifest-test
  PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(
  actionplugin-manifest-test
  PRIVATE Qt${QT_VERSION_MAJOR}::Core
          Qt${QT_VERSION_MAJOR}::Gui)

add_test(
  NAME actionplugin-manifest
  COMMAND actionplugin-manifest-test ${CMAKE_COMMAND})

find_package(Python3 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
  add_test(
    NAME plugin-cli
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/plugin_cli_test.py
            $<TARGET_FILE:flameshot>
            ${CMAKE_SOURCE_DIR}/sdk/flameshot-plugin)
endif()

find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Core Gui Widgets Test)

set(CAPTURE_DIR ${CMAKE_SOURCE_DIR}/src/widgets/capture)

function(flameshot_add_test name)
  cmake_parse_arguments(TEST "" "" "SOURCES;LIBRARIES;ARGS" ${ARGN})
  add_executable(flameshot-${name}-tests ${TEST_SOURCES})
  target_include_directories(flameshot-${name}-tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
  target_link_libraries(
    flameshot-${name}-tests
    PRIVATE project_options Qt${QT_VERSION_MAJOR}::Test ${TEST_LIBRARIES})
  set_target_properties(flameshot-${name}-tests PROPERTIES AUTOMOC ON)
  add_test(NAME ${name} COMMAND flameshot-${name}-tests ${TEST_ARGS})
  set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

flameshot_add_test(
  smart-region
  SOURCES smartregiontest.cpp
          ${CAPTURE_DIR}/smartregioncontroller.cpp
          ${CAPTURE_DIR}/smartregiondetector.cpp
  LIBRARIES Qt${QT_VERSION_MAJOR}::Core)

flameshot_add_test(
  touchpad-scroll-classifier
  SOURCES touchpadscrollclassifiertest.cpp
  LIBRARIES Qt${QT_VERSION_MAJOR}::Core)

flameshot_add_test(
  smart-region-geometry
  SOURCES smartregiongeometrytest.cpp
  LIBRARIES Qt${QT_VERSION_MAJOR}::Core)

flameshot_add_test(
  selection-widget
  SOURCES selectionwidgettest.cpp
          ${CAPTURE_DIR}/selectionwidget.cpp
          ${CMAKE_SOURCE_DIR}/src/utils/globalvalues.cpp
  LIBRARIES Qt${QT_VERSION_MAJOR}::Widgets)
target_compile_definitions(
  flameshot-selection-widget-tests PRIVATE APP_VERSION="test" FLAMESHOT_GIT_HASH="test")

if(WIN32)
  # Drives a real Notepad window, so it needs the Windows platform plugin.
  flameshot_add_test(
    windows-smart-region
    SOURCES windowssmartregiontest.cpp
            ${CAPTURE_DIR}/smartregiondetector.cpp
            ${CAPTURE_DIR}/windowssmartregiondetector.cpp
            ${CMAKE_SOURCE_DIR}/src/utils/inputtrace.cpp
            ${CMAKE_SOURCE_DIR}/src/utils/windowsforeground.cpp
    LIBRARIES Qt${QT_VERSION_MAJOR}::Gui dwmapi ole32 oleacc uiautomationcore
    # Keep a log, so CI can show what was found in each application.
    ARGS -o windows-smart-region.txt,txt -o -,txt)
  target_compile_definitions(
    flameshot-windows-smart-region-tests PRIVATE FLAMESHOT_SMART_REGION_BACKEND)
  set_tests_properties(windows-smart-region PROPERTIES ENVIRONMENT "")
endif()

if(WIN32)
  # Drives the real flameshot.exe: needs a desktop, like the test above.
  flameshot_add_test(
    windows-flameshot
    SOURCES windowsflameshottest.cpp
    LIBRARIES Qt${QT_VERSION_MAJOR}::Gui
    ARGS -o windows-flameshot.txt,txt -o -,txt)
  add_dependencies(flameshot-windows-flameshot-tests flameshot)
  target_compile_definitions(
    flameshot-windows-flameshot-tests
    PRIVATE FLAMESHOT_EXE="$<TARGET_FILE:flameshot>")
  set_tests_properties(windows-flameshot PROPERTIES ENVIRONMENT "")
endif()

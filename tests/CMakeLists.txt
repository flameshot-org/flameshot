# Unit tests (QtTest). Wired into CTest via the top-level enable_testing().
find_package(
  Qt${QT_VERSION_MAJOR}
  CONFIG
  REQUIRED
  Core
  Gui
  Test
)

set(CMAKE_AUTOMOC ON)

# Tests compile the units under test directly so the test target stays
# independent of the full flameshot executable.
add_executable(test_colorprofile
  unit/test_colorprofile.cpp
  ${CMAKE_SOURCE_DIR}/src/utils/colorprofileprovider.cpp
)

target_include_directories(test_colorprofile PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(test_colorprofile
  PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Gui
  Qt${QT_VERSION_MAJOR}::Test
)

if(APPLE)
  target_link_libraries(test_colorprofile
    PRIVATE
    "-framework CoreGraphics"
    "-framework CoreFoundation"
  )
endif()

add_test(NAME test_colorprofile COMMAND test_colorprofile)

# Run GUI-less tests against the offscreen platform plugin in CI/headless.
set_tests_properties(test_colorprofile PROPERTIES
  ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
)

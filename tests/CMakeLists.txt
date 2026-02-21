find_package(Qt${QT_VERSION_MAJOR} CONFIG REQUIRED Test Widgets)

set(CMAKE_AUTOMOC ON)

# Shared source files used by tests (the extracted geometry utilities)
set(GEOMETRY_SOURCES
    ${CMAKE_SOURCE_DIR}/src/utils/screengrabber_geometry.cpp
)

# --- Unit test: screengrabber geometry calculations ---
add_executable(test_screengrabber_geometry
    test_screengrabber_geometry.cpp
    ${GEOMETRY_SOURCES}
)
target_include_directories(test_screengrabber_geometry PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(test_screengrabber_geometry PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
)
add_test(NAME test_screengrabber_geometry COMMAND test_screengrabber_geometry)

# --- Unit test: CaptureRequest ---
# Self-contained: reproduces the class to avoid the ConfigHandler dependency chain
add_executable(test_capturerequest
    test_capturerequest.cpp
)
target_link_libraries(test_capturerequest PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
)
add_test(NAME test_capturerequest COMMAND test_capturerequest)

# --- Unit test: DesktopInfo ---
add_executable(test_desktopinfo
    test_desktopinfo.cpp
    ${CMAKE_SOURCE_DIR}/src/utils/desktopinfo.cpp
)
target_include_directories(test_desktopinfo PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(test_desktopinfo PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
)
add_test(NAME test_desktopinfo COMMAND test_desktopinfo)

# --- Integration test: crop with real QPixmaps ---
add_executable(test_crop_integration
    test_crop_integration.cpp
    ${GEOMETRY_SOURCES}
)
target_include_directories(test_crop_integration PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}
)
target_link_libraries(test_crop_integration PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)
add_test(NAME test_crop_integration COMMAND test_crop_integration)
# QPixmap requires a platform plugin; use offscreen on Linux so the test
# works in headless environments (dh_auto_test, containerised CI, etc.)
if(UNIX AND NOT APPLE)
    set_tests_properties(test_crop_integration PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
    )
endif()

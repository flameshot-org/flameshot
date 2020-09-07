find_package(
  Qt5
  CONFIG
  REQUIRED
  Core
  Gui
  Widgets
  Network
  Svg
  DBus
  LinguistTools)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(flameshot)

add_executable(Flameshot::flameshot ALIAS flameshot)

IF(WIN32)
  set_property(TARGET flameshot PROPERTY WIN32_EXECUTABLE true)
ENDIF()

add_subdirectory(cli)
add_subdirectory(config)
add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(widgets)
add_subdirectory(tools)

set(FLAMESHOT_TS_FILES
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_ca.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_de_DE.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_fr.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_es.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_hu.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_ja.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_ka.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_nl.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_pl.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_pt_br.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_ru.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_sk.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_sr.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_tr.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_uk.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_zh_CN.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_zh_TW.ts
    ${CMAKE_SOURCE_DIR}/data/translations/Internationalization_ko.ts)

if(GENERATE_TS)
  qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${FLAMESHOT_TS_FILES})
else()
  qt5_add_translation(QM_FILES ${FLAMESHOT_TS_FILES})
endif()

target_sources(
  flameshot
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../external/singleapplication/singleapplication.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/../external/Qt-Color-Widgets/src/color_utils.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/../external/Qt-Color-Widgets/src/color_wheel.cpp
          ${CMAKE_CURRENT_SOURCE_DIR}/../external/Qt-Color-Widgets/include/color_wheel.hpp
          ${CMAKE_CURRENT_SOURCE_DIR}/../data/graphics.qrc
          ${QM_FILES}
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
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools/pixelate>
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
         $<INSTALL_INTERFACE:include/mylib>)

target_link_libraries(
  flameshot
  project_warnings
  project_options
  Qt5::Svg
  Qt5::DBus
  Qt5::Network
  Qt5::Widgets)

target_compile_definitions(flameshot PRIVATE APP_PREFIX="${CMAKE_INSTALL_PREFIX}")
target_compile_definitions(flameshot PRIVATE APP_VERSION="v${PROJECT_VERSION}")
target_compile_definitions(flameshot PRIVATE IMGUR_CLIENT_ID="313baf0c7b4d3ff")
target_compile_definitions(flameshot PRIVATE QAPPLICATION_CLASS=QApplication)

foreach(FILE ${QM_FILES})
  get_filename_component(F_NAME ${FILE} NAME)
  add_custom_command(
    TARGET flameshot
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${F_NAME}
            ${CMAKE_CURRENT_BINARY_DIR}/translations/${F_NAME})
endforeach()
# ######################################################################################################################
# Installation instructions

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Flameshot)

# Install binary
install(
  TARGETS flameshot
  EXPORT flameshot-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install desktop files, completion and dbus files
configure_file(${CMAKE_SOURCE_DIR}/docs/desktopEntry/package/flameshot.desktop
               ${CMAKE_CURRENT_BINARY_DIR}/share/applications/flameshot.desktop COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/docs/bash-completion/flameshot
               ${CMAKE_CURRENT_BINARY_DIR}/share/bash-completions/completions/flameshot COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/data/dbus/org.flameshot.Flameshot.xml
               ${CMAKE_CURRENT_BINARY_DIR}/share/dbus-1/interfaces/org.flameshot.Flameshot.xml COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/data/dbus/org.flameshot.Flameshot.service.in
               ${CMAKE_CURRENT_BINARY_DIR}/share/dbus-1/services/org.flameshot.Flameshot.service)

# Install Icons
configure_file(${CMAKE_SOURCE_DIR}/data/img/hicolor/48x48/apps/flameshot.png
               ${CMAKE_CURRENT_BINARY_DIR}/share/icons/hicolor/48x48/apps/flameshot.png COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/data/img/hicolor/128x128/apps/flameshot.png
               ${CMAKE_CURRENT_BINARY_DIR}/share/icons/hicolor/128x128/apps/flameshot.png COPYONLY)

configure_file(${CMAKE_SOURCE_DIR}/data/img/hicolor/scalable/apps/flameshot.svg
               ${CMAKE_CURRENT_BINARY_DIR}/share/icons/hicolor/scalable/apps/flameshot.svg COPYONLY)

# Install assets
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/share/ DESTINATION ${CMAKE_INSTALL_DATAROOTDIR})

# Install Translations
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/flameshot/translations)

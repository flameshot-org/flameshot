# Required to generate MOC
IF (APPLE)
    add_subdirectory(QHotkey)
ENDIF ()

target_sources(flameshot PRIVATE controller.h flameshotdbusadapter.h qguiappcurrentscreen.h)

target_sources(flameshot PRIVATE capturerequest.cpp controller.cpp flameshotdbusadapter.cpp qguiappcurrentscreen.cpp)

IF (WIN32)
    target_sources(flameshot PRIVATE globalshortcutfilter.h globalshortcutfilter.cpp)
ENDIF ()

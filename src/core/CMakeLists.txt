
# Required to generate MOC
target_sources(flameshot PRIVATE controller.h flameshotdbusadapter.h)

target_sources(flameshot PRIVATE capturerequest.cpp controller.cpp flameshotdbusadapter.cpp)

IF(WIN32)
    target_sources(flameshot PRIVATE globalshortcutfilter.h globalshortcutfilter.cpp)
ENDIF()

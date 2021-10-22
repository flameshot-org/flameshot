target_sources(flameshot PRIVATE
    controller.h
    flameshotdaemon.h
    flameshotdbusadapter.h
    qguiappcurrentscreen.h
)

target_sources(flameshot PRIVATE
    capturerequest.cpp
    controller.cpp
    flameshotdaemon.cpp
    flameshotdbusadapter.cpp
    qguiappcurrentscreen.cpp
)

IF (WIN32)
    target_sources(flameshot PRIVATE globalshortcutfilter.h globalshortcutfilter.cpp)
ENDIF ()

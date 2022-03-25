target_sources(flameshot PRIVATE
    flameshot.h
    flameshotdaemon.h
    flameshotdbusadapter.h
    qguiappcurrentscreen.h
)

target_sources(flameshot PRIVATE
    capturerequest.cpp
    flameshot.cpp
    flameshotdaemon.cpp
    flameshotdbusadapter.cpp
    qguiappcurrentscreen.cpp
)

IF (WIN32)
    target_sources(flameshot PRIVATE globalshortcutfilter.h globalshortcutfilter.cpp)
ENDIF ()

target_sources(flameshot PRIVATE
    flameshot.h
    flameshotdaemon.h
    flameshotdbusadapter.h
    qguiappcurrentscreen.h
    pluginmanager.h
    corepluginInterface.h
)

target_sources(flameshot PRIVATE
    capturerequest.cpp
    flameshot.cpp
    flameshotdaemon.cpp
    flameshotdbusadapter.cpp
    qguiappcurrentscreen.cpp
    pluginmanager.cpp
)

IF (WIN32)
    target_sources(flameshot PRIVATE globalshortcutfilter.h globalshortcutfilter.cpp)
ENDIF ()

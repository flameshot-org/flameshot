target_sources(flameshot PRIVATE
    flameshot.h
    flameshotdaemon.h
    qguiappcurrentscreen.h
)

target_sources(flameshot PRIVATE
    capturerequest.cpp
    flameshot.cpp
    flameshotdaemon.cpp
    qguiappcurrentscreen.cpp
)

if (UNIX AND NOT APPLE)
target_sources(flameshot PRIVATE
    flameshotdbusadapter.h
    flameshotdbusadapter.cpp
)
endif ()

if (USE_KDSINGLEAPPLICATION)
if (NOT WIN32)
target_sources(flameshot PRIVATE
    signaldaemon.h
    signaldaemon.cpp
)
endif ()
endif ()

if (WIN32)
    target_sources(flameshot PRIVATE globalshortcutfilter.h globalshortcutfilter.cpp)
endif ()

if (UNIX AND NOT APPLE AND XI_FOUND AND X11_FOUND)
    target_sources(flameshot PRIVATE x11shortcutfilter.h x11shortcutfilter.cpp)
endif ()

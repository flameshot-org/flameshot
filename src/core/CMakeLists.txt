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

if (APPLE)
    target_sources(flameshot PRIVATE
        macoscapturekeepalive.h
        macoscapturekeepalive.mm
    )
    # Weak-link: ScreenCaptureKit is missing below macOS 12.3
    target_link_libraries(flameshot "-weak_framework ScreenCaptureKit" "-framework Foundation")
endif ()

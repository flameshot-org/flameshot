# Required to generate MOC
target_sources(
        flameshot
        PRIVATE buttonhandler.h
        capturebutton.h
        capturetoolbutton.h
        capturewidget.h
        colorpicker.h
        hovereventfilter.h
        overlaymessage.h
        selectionwidget.h
        smartregioncontroller.h
        smartregiondetector.h
        smartregiongeometry.h
        touchpadscrollclassifier.h
        magnifierwidget.h
        notifierbox.h
        modificationcommand.h)

target_sources(
        flameshot
        PRIVATE buttonhandler.cpp
        capturebutton.cpp
        capturetoolbutton.cpp
        capturewidget.cpp
        colorpicker.cpp
        hovereventfilter.cpp
        overlaymessage.cpp
        notifierbox.cpp
        selectionwidget.cpp
        smartregioncontroller.cpp
        smartregiondetector.cpp
        magnifierwidget.cpp
        modificationcommand.cpp)

# Native smart region selection backends. Platforms without one keep the
# manual selection only.
if(APPLE)
    target_sources(flameshot PRIVATE macossmartregiondetector.cpp)
    target_compile_definitions(flameshot PRIVATE FLAMESHOT_SMART_REGION_BACKEND)
elseif(WIN32)
    target_sources(flameshot PRIVATE windowssmartregiondetector.cpp)
    target_compile_definitions(flameshot PRIVATE FLAMESHOT_SMART_REGION_BACKEND)
elseif(UNIX)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(XCB IMPORTED_TARGET GLOBAL xcb)
    endif()
    if(XCB_FOUND)
        target_sources(flameshot PRIVATE x11smartregiondetector.cpp)
        target_compile_definitions(flameshot PRIVATE FLAMESHOT_SMART_REGION_BACKEND)
        target_link_libraries(flameshot PkgConfig::XCB)
    else()
        message(STATUS "xcb not found: smart region selection is disabled")
    endif()
endif()

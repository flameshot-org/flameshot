# Required to generate MOC
target_sources(
        flameshot
        PRIVATE buttonhandler.h
        capturebutton.h
        capturetoolbutton.h
        capturewidget.h
        colorpicker.h
        hovereventfilter.h
        selectionwidget.h
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
        notifierbox.cpp
        selectionwidget.cpp
        modificationcommand.cpp)

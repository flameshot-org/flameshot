add_subdirectory(panel)
add_subdirectory(capture)

# Required to generate MOC
target_sources(
        flameshot
        PRIVATE
        infowindow.ui
        capturelauncher.ui

        capturelauncher.h
        draggablewidgetmaker.h
        imagelabel.h
        trayicon.h
        infowindow.h
        loadspinner.h
        notificationwidget.h
        orientablepushbutton.h

        colorpickerwidget.h
        capture/capturetoolobjects.h
)

if (ENABLE_IMGUR OR ENABLE_GDRIVE)
target_sources(
    flameshot
    PRIVATE
        uploadhistory.ui
        uploadlineitem.ui
        uploadhistory.h
        uploadlineitem.h
        imguploaddialog.h
        recipientchipedit.h
)
endif()

target_sources(
        flameshot
        PRIVATE
        capturelauncher.cpp
        draggablewidgetmaker.cpp
        imagelabel.cpp
        trayicon.cpp
        infowindow.cpp
        loadspinner.cpp
        notificationwidget.cpp
        orientablepushbutton.cpp
        colorpickerwidget.cpp
        capture/capturetoolobjects.cpp
)

if (ENABLE_IMGUR OR ENABLE_GDRIVE)
target_sources(
    flameshot
    PRIVATE
        uploadhistory.cpp
        uploadlineitem.cpp
        imguploaddialog.cpp
        recipientchipedit.cpp
)
endif()

if (NOT DISABLE_UPDATE_CHECKER)
    target_sources(
            flameshot
            PRIVATE
            updatenotificationwidget.h
            updatenotificationwidget.cpp
    )
endif()
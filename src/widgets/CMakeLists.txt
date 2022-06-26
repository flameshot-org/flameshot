add_subdirectory(panel)
add_subdirectory(capture)

# Required to generate MOC
target_sources(
        flameshot
        PRIVATE
        infowindow.ui
        capturelauncher.ui
        uploadhistory.ui
        uploadlineitem.ui

        capturelauncher.h
        draggablewidgetmaker.h
        imagelabel.h
        trayicon.h
        infowindow.h
        loadspinner.h
        notificationwidget.h
        orientablepushbutton.h
        uploadhistory.h
        uploadlineitem.h
        updatenotificationwidget.h
        colorpickerwidget.h
        imguploaddialog.h
        capture/capturetoolobjects.h
)

target_sources(
        flameshot
        PRIVATE capturelauncher.cpp
        draggablewidgetmaker.cpp
        imagelabel.cpp
        trayicon.cpp
        infowindow.cpp
        loadspinner.cpp
        notificationwidget.cpp
        orientablepushbutton.cpp
        uploadhistory.cpp
        uploadlineitem.cpp
        updatenotificationwidget.cpp
        colorpickerwidget.cpp
        imguploaddialog.cpp
        capture/capturetoolobjects.cpp
)

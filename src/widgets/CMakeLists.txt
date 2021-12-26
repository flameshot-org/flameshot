add_subdirectory(panel)
add_subdirectory(capture)

# Required to generate MOC
target_sources(
        flameshot
        PRIVATE capturelauncher.h
        draggablewidgetmaker.h
        imagelabel.h
        infowindow.h
        loadspinner.h
        notificationwidget.h
        orientablepushbutton.h
        historywidget.h
        updatenotificationwidget.h
        imguruploaddialog.h
        colorpickerwidget.h
        spinbox.h
        capture/capturetoolobjects.h
)

target_sources(
        flameshot
        PRIVATE capturelauncher.cpp
        draggablewidgetmaker.cpp
        imagelabel.cpp
        infowindow.cpp
        loadspinner.cpp
        notificationwidget.cpp
        orientablepushbutton.cpp
        historywidget.cpp
        updatenotificationwidget.cpp
        imguruploaddialog.cpp
        colorpickerwidget.cpp
        spinbox.cpp
        capture/capturetoolobjects.cpp
)

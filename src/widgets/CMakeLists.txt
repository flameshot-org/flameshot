add_subdirectory(panel)
add_subdirectory(capture)

# Required to generate MOC
target_sources(
  flameshot
  PRIVATE capturelauncher.h
          imagelabel.h
          infowindow.h
          loadspinner.h
          notificationwidget.h
          orientablepushbutton.h)

target_sources(
  flameshot
  PRIVATE capturelauncher.cpp
          imagelabel.cpp
          infowindow.cpp
          loadspinner.cpp
          notificationwidget.cpp
          orientablepushbutton.cpp)

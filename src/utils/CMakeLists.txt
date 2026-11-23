# Required to generate MOC
target_sources(
  flameshot
  PRIVATE abstractlogger.h
          filenamehandler.h
          screengrabber.h
          systemnotification.h
          valuehandler.h
          strfparse.h
          gifrecorder.h
          x11rootgrab.h
)

target_sources(
  flameshot
  PRIVATE abstractlogger.cpp
          filenamehandler.cpp
          screengrabber.cpp
          monitorpreview.cpp
          confighandler.cpp
          systemnotification.cpp
          valuehandler.cpp
          screenshotsaver.cpp
          globalvalues.cpp
          desktopfileparse.cpp
          desktopinfo.cpp
          pathinfo.cpp
          colorutils.cpp
          history.cpp
          strfparse.cpp
          gifencoder.cpp
          gifrecorder.cpp
          x11rootgrab.cpp
)

IF (UNIX AND NOT APPLE)
target_sources(
  flameshot
  PRIVATE request.h
          request.cpp
)
ENDIF()

IF (WIN32)
  target_sources(
    flameshot
    PRIVATE winlnkfileparse.cpp
  )
ENDIF()

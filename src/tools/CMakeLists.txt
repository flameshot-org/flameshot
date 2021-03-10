target_sources(flameshot PRIVATE arrow/arrowtool.h arrow/arrowtool.cpp)
target_sources(flameshot PRIVATE pixelate/pixelatetool.h pixelate/pixelatetool.cpp)
target_sources(flameshot PRIVATE circle/circletool.h circle/circletool.cpp)
target_sources(flameshot PRIVATE circlecount/circlecounttool.h circlecount/circlecounttool.cpp)
target_sources(flameshot PRIVATE copy/copytool.h copy/copytool.cpp)
target_sources(flameshot PRIVATE exit/exittool.h exit/exittool.cpp)
target_sources(flameshot PRIVATE sizeincrease/sizeincreasetool.h sizeincrease/sizeincreasetool.cpp)
target_sources(flameshot PRIVATE sizedecrease/sizedecreasetool.h sizedecrease/sizedecreasetool.cpp)
target_sources(
  flameshot
  PRIVATE storage/storagemanager.h
          storage/storagemanager.cpp
          storage/imguploader.h
          storage/imguploadertool.h
          storage/imguploader.cpp
          storage/imguploadertool.cpp
          storage/imgur/imguruploader.h
          storage/imgur/imguruploadertool.h
          storage/imgur/imguruploader.cpp
          storage/imgur/imguruploadertool.cpp
          storage/s3/imgs3settings.h
          storage/s3/imgs3settings.cpp
          storage/s3/imgs3uploader.h
          storage/s3/imgs3uploadertool.h
          storage/s3/imgs3uploader.cpp
          storage/s3/imgs3uploadertool.cpp
)
target_sources(
  flameshot
  PRIVATE launcher/applaunchertool.h
          launcher/applauncherwidget.h
          launcher/launcheritemdelegate.h
          launcher/terminallauncher.h
          launcher/applaunchertool.cpp
          launcher/applauncherwidget.cpp
          launcher/launcheritemdelegate.cpp
          launcher/openwithprogram.cpp
          launcher/terminallauncher.cpp)
target_sources(flameshot PRIVATE line/linetool.h line/linetool.cpp)
target_sources(flameshot PRIVATE marker/markertool.h marker/markertool.cpp)
target_sources(flameshot PRIVATE move/movetool.h move/movetool.cpp)
target_sources(flameshot PRIVATE pencil/penciltool.h pencil/penciltool.cpp)
target_sources(
  flameshot
  PRIVATE pin/pintool.h
          pin/pinwidget.h
          pin/pintool.cpp
          pin/pinwidget.cpp)
target_sources(flameshot PRIVATE rectangle/rectangletool.h rectangle/rectangletool.cpp)
target_sources(flameshot PRIVATE redo/redotool.h redo/redotool.cpp)
target_sources(flameshot PRIVATE save/savetool.h save/savetool.cpp)
target_sources(flameshot PRIVATE selection/selectiontool.h selection/selectiontool.cpp)
target_sources(flameshot PRIVATE sizeindicator/sizeindicatortool.h sizeindicator/sizeindicatortool.cpp)
target_sources(
  flameshot
  PRIVATE text/textconfig.h
          text/texttool.h
          text/textwidget.h
          text/textconfig.cpp
          text/texttool.cpp
          text/textwidget.cpp)
target_sources(flameshot PRIVATE undo/undotool.h undo/undotool.cpp)

target_sources(
  flameshot
  PRIVATE abstractactiontool.cpp
          abstractpathtool.cpp
          abstracttwopointtool.cpp
          capturecontext.cpp
          toolfactory.cpp
          abstractactiontool.h
          abstractpathtool.h
          abstracttwopointtool.h
          capturetool.h
          toolfactory.h)

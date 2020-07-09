
# Required to generate MOC
target_sources(flameshot PRIVATE controller.h flameshotdbusadapter.h)

target_sources(flameshot PRIVATE capturerequest.cpp controller.cpp flameshotdbusadapter.cpp)

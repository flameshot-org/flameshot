# Single Application implementation
include(../../singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QCoreApplication

SOURCES += main.cpp \
    messagereceiver.cpp

HEADERS += \
    messagereceiver.h

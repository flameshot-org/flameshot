QT += core network
CONFIG += c++11

HEADERS += $$PWD/singleapplication.h \
    $$PWD/singleapplication_p.h
SOURCES += $$PWD/singleapplication.cpp

INCLUDEPATH += $$PWD

win32 {
    msvc:LIBS += Advapi32.lib
    gcc:LIBS += -lAdvapi32
}

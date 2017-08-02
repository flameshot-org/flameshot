QT += widgets

HEADERS = button.h \
    calculator.h
SOURCES = button.cpp \
    calculator.cpp \
    main.cpp

# Single Application implementation
include(../../singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication

CONFIG += C++11

PUBLIC_HEADERS += $$PWD/QHotkey/qhotkey.h \
	$$PWD/QHotkey/QHotkey

HEADERS += $$PUBLIC_HEADERS \
	$$PWD/QHotkey/qhotkey_p.h

SOURCES += $$PWD/QHotkey/qhotkey.cpp

mac: SOURCES += $$PWD/QHotkey/qhotkey_mac.cpp
else:win32: SOURCES += $$PWD/QHotkey/qhotkey_win.cpp
else:unix: SOURCES += $$PWD/QHotkey/qhotkey_x11.cpp

INCLUDEPATH += $$PWD/QHotkey

include($$PWD/qhotkey.prc)

TEMPLATE = lib
win32: CONFIG += dll

TARGET = QHotkey
VERSION = 1.2.1

include(../qhotkey.pri)

DEFINES += QHOTKEY_LIB QHOTKEY_LIB_BUILD

# use INSTALL_ROOT to modify the install location
headers.files = $$PUBLIC_HEADERS
headers.path = $$[QT_INSTALL_HEADERS]
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target headers


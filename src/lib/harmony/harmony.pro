TEMPLATE = lib
TARGET = harmony

QT = core dbus

CONFIG += staticlib

include(../../config.pri)

INCLUDEPATH += ../../3rdparty/civetweb/include/

HEADERS += \
    iserver.h \
    jsonwebtoken.h \
    iauthentificationservice.h \
    harmonyextension.h \
    iextensionmanager.h

SOURCES += \
    server.cpp \
    jsonwebtoken.cpp \
    authentificationservice.cpp \
    harmonyextension.cpp \
    extensionmanager.cpp

RESOURCES += \
    harmony.qrc

system($$PWD/gencert.sh $$PWD/ssl)

TEMPLATE = lib
TARGET = harmonytest

QT = core dbus

CONFIG += staticlib

include(../../config.pri)

INCLUDEPATH += ../../lib/harmony
LIBS += -L../../lib/harmony -lharmony

HEADERS += \
    testserviceprovider.h \
    utils.h \
    dbusintrospect.h

SOURCES += \
    testserviceprovider.cpp \
    utils.cpp \
    dbusintrospect.cpp

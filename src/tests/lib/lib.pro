TEMPLATE = lib
TARGET = harmonytest

QT = core dbus

CONFIG += staticlib

include(../../config.pri)

INCLUDEPATH += ../../lib
LIBS += -L../../lib -lharmony

HEADERS += \
    testserviceprovider.h \
    utils.h \
    dbusintrospect.h

SOURCES += \
    testserviceprovider.cpp \
    utils.cpp \
    dbusintrospect.cpp

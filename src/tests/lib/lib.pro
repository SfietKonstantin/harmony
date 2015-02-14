TEMPLATE = lib
TARGET = harmonytest

QT = core dbus

CONFIG += staticlib

include(../../config.pri)

INCLUDEPATH += ../../lib
LIBS += -L../../lib -lharmony

HEADERS += \
    testserviceprovider.h \
    utils.h

SOURCES += \
    testserviceprovider.cpp \
    utils.cpp

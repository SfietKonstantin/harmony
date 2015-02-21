TEMPLATE = app
TARGET = tst_pluginservice

QT = core dbus testlib

include(../../../config.pri)

INCLUDEPATH += ../../../lib
LIBS += -L../../../plugins/test -lharmonytest
LIBS += -L../../../lib -lharmony

SOURCES += tst_pluginservice.cpp

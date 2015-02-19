TEMPLATE = app
TARGET = tst_harmonynodebridge

QT = core dbus testlib

include(../../../config.pri)

INCLUDEPATH += ../../../lib
LIBS += -L../../../lib -lharmony

SOURCES += tst_harmonynodebridge.cpp

TEMPLATE = app
TARGET = tst_identificationservice

QT = core dbus testlib

include(../../config.pri)

INCLUDEPATH += ../../lib
LIBS += -L../../lib -lharmony

SOURCES += tst_identificationservice.cpp

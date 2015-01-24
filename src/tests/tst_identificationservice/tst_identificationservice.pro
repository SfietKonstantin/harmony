TEMPLATE = app
TARGET = tst_identificationservice

QT = core dbus testlib

INCLUDEPATH += ../../lib
LIBS += -L../../lib -lharmony

SOURCES += tst_identificationservice.cpp

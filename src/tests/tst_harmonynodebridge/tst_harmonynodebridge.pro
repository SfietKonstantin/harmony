TEMPLATE = app
TARGET = tst_harmonynodebridge

QT = core dbus testlib
CONFIG += c++11

INCLUDEPATH += ../../lib
LIBS += -L../../lib -lharmony

SOURCES += tst_harmonynodebridge.cpp

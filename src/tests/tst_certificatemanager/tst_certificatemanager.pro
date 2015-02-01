TEMPLATE = app
TARGET = tst_certificatemanager

QT = core testlib
CONFIG += c++11

INCLUDEPATH += ../../lib
LIBS += -L../../lib -lharmony

SOURCES += tst_certificatemanager.cpp

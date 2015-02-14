TEMPLATE = app
TARGET = tst_certificatemanager

QT = core testlib

include(../../config.pri)

INCLUDEPATH += ../lib \
    ../../lib
LIBS += -L../lib -lharmonytest \
    -L../../lib -lharmony

SOURCES += tst_certificatemanager.cpp

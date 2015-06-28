TEMPLATE = app
TARGET = tst_authentificationservice

QT = core testlib

include(../../../config.pri)

INCLUDEPATH += ../../../lib/harmony
LIBS += -L../../../lib/harmony -lharmony

SOURCES += tst_authentificationservice.cpp

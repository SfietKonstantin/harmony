TEMPLATE = app
TARGET = tst_harmonyextension

QT = core testlib

include(../../../config.pri)

INCLUDEPATH += ../../../lib/harmony
LIBS += -L../../../plugins/test -lharmonytestextension \
    -L../../../lib/harmony -lharmony

SOURCES += tst_harmonyextension.cpp

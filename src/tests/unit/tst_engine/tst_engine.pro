TEMPLATE = app
TARGET = tst_engine

QT = core network dbus testlib

include(../../../config.pri)
include(../../../lib/civet/civet-deps.pri)

INCLUDEPATH += ../../../lib/harmony
LIBS += -L../../../plugins/test -lharmonytestextension \
    -L../../../lib/harmony -lharmony \
    -L../../../lib/civet -lcivet

SOURCES += tst_engine.cpp

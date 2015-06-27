TEMPLATE = app
TARGET = tst_server

QT = core network testlib

include(../../../config.pri)
include(../../../lib/civet/civet-deps.pri)

INCLUDEPATH += ../../lib \
    ../../../lib/harmony
LIBS += -L../../../lib/harmony -lharmony \
    -L../../../lib/civet -lcivet

SOURCES += tst_server.cpp

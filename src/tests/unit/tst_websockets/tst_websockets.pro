TEMPLATE = app
TARGET = tst_websockets

QT = core network testlib websockets

include(../../../config.pri)
include(../../../lib/civet/civet-deps.pri)

INCLUDEPATH += ../../../lib/harmony
LIBS += -L../../../plugins/test -lharmonytestextension \
    -L../../../lib/harmony -lharmony \
    -L../../../lib/civet -lcivet

SOURCES += tst_websockets.cpp

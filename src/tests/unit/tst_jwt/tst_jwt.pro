TEMPLATE = app
TARGET = tst_jwt

QT = core testlib

include(../../../config.pri)

INCLUDEPATH += ../../../lib/harmony
LIBS += -L../../../lib/harmony -lharmony

SOURCES += tst_jwt.cpp

TEMPLATE = app
TARGET = harmonyrunner

QT = core

include(../../config.pri)
include(../../lib/civet/civet-deps.pri)

INCLUDEPATH += ../../lib/harmony
LIBS += -L../../plugins/test -lharmonytestplugin \
    -L../../lib/harmony -lharmony \
    -L../../lib/civet -lcivet

SOURCES += main.cpp

target.path = /usr/bin
INSTALLS += target

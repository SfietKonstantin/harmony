TEMPLATE = app
TARGET = harmonyrunner

QT = core

include(../../config.pri)

INCLUDEPATH += ../../lib/harmony
LIBS += -L../../plugins/test -lharmonytestplugin
LIBS += -L../../lib/harmony -lharmony

SOURCES += main.cpp

target.path = /usr/bin
INSTALLS += target

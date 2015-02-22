TEMPLATE = app
TARGET = harmonyrunner

QT = core dbus

include(../../config.pri)

INCLUDEPATH += ../../lib
LIBS += -L../../plugins/test -lharmonytestplugin
LIBS += -L../../lib -lharmony

SOURCES += main.cpp

target.path = /usr/bin
INSTALLS += target

TEMPLATE = app
TARGET = harmonyrunner

QT = core dbus
CONFIG += c++11

INCLUDEPATH += ../../lib
LIBS += -L../../lib -lharmony

SOURCES += main.cpp

target.path = /usr/bin
INSTALLS += target

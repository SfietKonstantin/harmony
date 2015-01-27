TEMPLATE = app
TARGET = tst_nodemanager

QT = core dbus testlib
CONFIG += c++11

INCLUDEPATH += ../../lib
LIBS += -L../../lib -lharmony

RESOURCES += \
    res.qrc

SOURCES += tst_nodemanager.cpp

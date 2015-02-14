TEMPLATE = app
TARGET = tst_nodemanager

QT = core dbus testlib

include(../../config.pri)

INCLUDEPATH += ../../lib
LIBS += -L../../lib -lharmony

RESOURCES += \
    res.qrc

SOURCES += tst_nodemanager.cpp

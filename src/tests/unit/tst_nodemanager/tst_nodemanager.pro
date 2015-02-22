TEMPLATE = app
TARGET = tst_nodemanager

QT = core dbus testlib

include(../../../config.pri)

INCLUDEPATH += ../../lib \
    ../../../lib
LIBS += -L../../lib -lharmonytest \
    -L../../../lib -lharmony

RESOURCES += \
    res.qrc

HEADERS += testproxy.h

SOURCES += tst_nodemanager.cpp \
    testproxy.cpp

system(qdbusxml2cpp $$PWD/../../../lib/org.sfietkonstantin.harmony.nodemanager.xml -p testproxy -c TestProxy)

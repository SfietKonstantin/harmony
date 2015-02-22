TEMPLATE = app
TARGET = tst_base

QT = core dbus testlib

include(../../../config.pri)

INCLUDEPATH += ../../lib \
    ../../../lib
LIBS += -L../../lib -lharmonytest \
    -L../../../lib -lharmony

HEADERS += testproxy.h

SOURCES += tst_base.cpp \
    testproxy.cpp

OTHER_FILES += $$PWD/../../../lib/org.sfietkonstantin.harmony.nodeconfigurationservice.xml

system(qdbusxml2cpp $$PWD/../../../lib/org.sfietkonstantin.harmony.nodeconfigurationservice.xml -p testproxy -c TestProxy)

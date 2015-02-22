TEMPLATE = app
TARGET = tst_identificationservice

QT = core dbus testlib

include(../../../config.pri)

INCLUDEPATH += ../../lib \
    ../../../lib
LIBS += -L../../lib -lharmonytest \
    -L../../../lib -lharmony

HEADERS += testproxy.h

SOURCES += tst_identificationservice.cpp \
    testproxy.cpp

OTHER_FILES += $$PWD/../../../lib/org.sfietkonstantin.harmony.identificationservice.xml

system(qdbusxml2cpp $$PWD/../../../lib/org.sfietkonstantin.harmony.identificationservice.xml -p testproxy -c TestProxy)

TEMPLATE = app
TARGET = tst_pluginservice

QT = core dbus testlib

include(../../../config.pri)

INCLUDEPATH += ../../lib \
    ../../../lib
LIBS += -L../../../plugins/test -lharmonytestplugin \
    -L../../lib -lharmonytest \
    -L../../../lib -lharmony

HEADERS += testadaptor.h \
    testproxy.h

SOURCES += tst_pluginservice.cpp \
    testadaptor.cpp \
    testproxy.cpp

OTHER_FILES += \
    org.sfietkonstantin.harmony.test.xml

system(qdbusxml2cpp org.sfietkonstantin.harmony.test.xml -a testadaptor -c TestAdaptor -i pluginservice.h)
system(qdbusxml2cpp org.sfietkonstantin.harmony.test.xml -p testproxy -c TestProxy -i pluginservice.h)

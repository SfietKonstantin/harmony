TEMPLATE = app
TARGET = tst_harmonyextension

QT = core dbus testlib

include(../../config.pri)

INCLUDEPATH += ../lib \
    ../../lib
LIBS += -L../lib -lharmonytest \
    -L../../lib -lharmony

HEADERS += testadaptor.h \
    testproxy.h


SOURCES += tst_harmonyextension.cpp \
    testadaptor.cpp \
    testproxy.cpp

OTHER_FILES += \
    org.sfietkonstantin.harmony.test.xml

system(qdbusxml2cpp org.sfietkonstantin.harmony.test.xml -a testadaptor -c TestAdaptor -i harmonyextension.h)
system(qdbusxml2cpp org.sfietkonstantin.harmony.test.xml -p testproxy -c TestProxy -i harmonyextension.h)

TEMPLATE = lib
TARGET = harmony

QT = core dbus

CONFIG += c++11
CONFIG += staticlib
CONFIG(debug) {
    DEFINES += HARMONY_DEBUG
}

HEADERS += \
    serviceprovider.h \
    identificationservice.h \
    identificationserviceadaptor.h \
    nodemanager.h

SOURCES += \
    serviceprovider.cpp \
    identificationservice.cpp \
    identificationserviceadaptor.cpp \
    nodemanager.cpp

OTHER_FILES += \
    org.sfietkonstantin.harmony.identificationservice.xml

system(qdbusxml2cpp org.sfietkonstantin.harmony.identificationservice.xml -a identificationserviceadaptor -c IdentificationServiceAdaptor)

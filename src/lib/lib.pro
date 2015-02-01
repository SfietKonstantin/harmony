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
    nodemanager.h \
    nodemanageradaptor.h \
    certificatemanager.h

SOURCES += \
    serviceprovider.cpp \
    identificationservice.cpp \
    identificationserviceadaptor.cpp \
    nodemanager.cpp \
    nodemanageradaptor.cpp \
    certificatemanager.cpp

RESOURCES += \
    harmony.qrc

OTHER_FILES += \
    org.sfietkonstantin.harmony.identificationservice.xml \
    org.sfietkonstantin.harmony.nodemanager.xml \

system(qdbusxml2cpp org.sfietkonstantin.harmony.identificationservice.xml -a identificationserviceadaptor -c IdentificationServiceAdaptor)
system(qdbusxml2cpp org.sfietkonstantin.harmony.nodemanager.xml -a nodemanageradaptor -c NodeManagerAdaptor)

TEMPLATE = lib
TARGET = harmony

QT = core dbus

CONFIG += staticlib

include(../config.pri)

HEADERS += \
    serviceprovider.h \
    identificationservice.h \
    identificationserviceadaptor.h \
    nodemanager.h \
    nodemanageradaptor.h \
    certificatemanager.h \
    nodeconfigurationservice.h \
    nodeconfigurationserviceadaptor.h \
    harmonyextension.h

SOURCES += \
    serviceprovider.cpp \
    identificationservice.cpp \
    identificationserviceadaptor.cpp \
    nodemanager.cpp \
    nodemanageradaptor.cpp \
    certificatemanager.cpp \
    nodeconfigurationservice.cpp \
    nodeconfigurationserviceadaptor.cpp \
    harmonyextension.cpp

RESOURCES += \
    harmony.qrc

OTHER_FILES += \
    org.sfietkonstantin.harmony.identificationservice.xml \
    org.sfietkonstantin.harmony.nodemanager.xml \
    org.sfietkonstantin.harmony.nodeconfigurationservice.xml

system(qdbusxml2cpp org.sfietkonstantin.harmony.identificationservice.xml -a identificationserviceadaptor -c IdentificationServiceAdaptor)
system(qdbusxml2cpp org.sfietkonstantin.harmony.nodemanager.xml -a nodemanageradaptor -c NodeManagerAdaptor)
system(qdbusxml2cpp org.sfietkonstantin.harmony.nodeconfigurationservice.xml -a nodeconfigurationserviceadaptor -c NodeConfigurationServiceAdaptor)

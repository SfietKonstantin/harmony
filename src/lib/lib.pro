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
    harmonyextension.h \
    pluginmanager.h \
    pluginservice.h \
    pluginserviceadaptor.h \
    pluginadaptor.h

SOURCES += \
    serviceprovider.cpp \
    identificationservice.cpp \
    identificationserviceadaptor.cpp \
    nodemanager.cpp \
    nodemanageradaptor.cpp \
    certificatemanager.cpp \
    nodeconfigurationservice.cpp \
    nodeconfigurationserviceadaptor.cpp \
    harmonyextension.cpp \
    pluginmanager.cpp \
    pluginservice.cpp \
    pluginserviceadaptor.cpp \
    pluginadaptor.cpp

RESOURCES += \
    harmony.qrc

OTHER_FILES += \
    org.sfietkonstantin.harmony.identificationservice.xml \
    org.sfietkonstantin.harmony.nodemanager.xml \
    org.sfietkonstantin.harmony.nodeconfigurationservice.xml \
    org.sfietkonstantin.harmony.pluginservice.xml \
    org.sfietkonstantin.harmony.plugin.xml

system(qdbusxml2cpp org.sfietkonstantin.harmony.identificationservice.xml -a identificationserviceadaptor -c IdentificationServiceAdaptor)
system(qdbusxml2cpp org.sfietkonstantin.harmony.nodemanager.xml -a nodemanageradaptor -c NodeManagerAdaptor)
system(qdbusxml2cpp org.sfietkonstantin.harmony.nodeconfigurationservice.xml -a nodeconfigurationserviceadaptor -c NodeConfigurationServiceAdaptor)
system(qdbusxml2cpp org.sfietkonstantin.harmony.pluginservice.xml -a pluginserviceadaptor -c PluginServiceAdaptor -i pluginservice.h)
system(qdbusxml2cpp org.sfietkonstantin.harmony.plugin.xml -a pluginadaptor -c PluginAdaptor -i pluginservice.h)

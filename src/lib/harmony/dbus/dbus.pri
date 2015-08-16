system(qdbusxml2cpp harbour.harmony.xml -i private/dbusinterfaceimpl.h -a private/adaptor)

INCLUDEPATH += $$PWD

HEADERS += $$PWD/idbusinterface.h \
    $$PWD/private/adaptor.h \
    $$PWD/private/dbusinterfaceimpl.h

SOURCES += $$PWD/dbusinterface.cpp \
    $$PWD/private/adaptor.cpp

OTHER_FILES += $$PWD/harbour.harmony.xml


system(qdbusxml2cpp harbour.harmony.xml -i private/dbusengineimpl.h -a private/adaptor)

INCLUDEPATH += $$PWD

HEADERS += $$PWD/idbusengine.h \
    $$PWD/private/adaptor.h \
    $$PWD/private/dbusengineimpl.h

SOURCES += $$PWD/dbusengine.cpp \
    $$PWD/private/adaptor.cpp

OTHER_FILES += $$PWD/harbour.harmony.xml


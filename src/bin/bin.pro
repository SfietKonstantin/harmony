TEMPLATE = app
TARGET = harbour-harmony

QT = core gui dbus quick

include(../config.pri)

CONFIG += c++11
!CONFIG(desktop) {
    CONFIG += sailfishapp
} else {
    DEFINES += DESKTOP
    RESOURCES += res.qrc
}

INCLUDEPATH += ../lib/harmony
LIBS += -L../lib/harmony -lharmony

HEADERS += \

SOURCES += \
    main.cpp

OTHER_FILES += \
    qml/main.qml \
    qml/MainPage.qml

target.path = /usr/bin

desktop.path = /usr/share/applications
desktop.files = harbour-harmony.desktop

icon.path = /usr/share/icons/hicolor/86x86/apps/
icon.files = harmony.png

qml.path = /usr/share/harbour-harmony/qml
qml.files = $$OTHER_FILES

INSTALLS += target desktop icon qml

TEMPLATE = lib
TARGET = civet

CONFIG += c++11
CONFIG += link_pkgconfig
CONFIG -= qt
CONFIG += static

include(civet-deps.pri)

DEFINES += NDEBUG \
    NO_SSL_DL \
    USE_IPV6 \
    USE_WEBSOCKET

HEADERS += ../../3rdparty/civetweb/include/civetweb.h \
    ../../3rdparty/civetweb/include/CivetServer.h

SOURCES += ../../3rdparty/civetweb/src/civetweb.c \
    ../../3rdparty/civetweb/src/CivetServer.cpp

include(../../defaults.pri)

QT += core
QT -= gui
QT += dbus

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

HEADERS += dbdperftest.h \
    dbdperftestreader.h \
    dbdperftestwriter.h
SOURCES += main.cpp \
    dbdperftest.cpp \
    dbdperftestreader.cpp \
    dbdperftestwriter.cpp

LIBS += -L../../src -ldb -lqmjson -lxenstore

PRE_TARGETDEPS += ../../src/libdb.a

target.path = /usr/bin
INSTALLS += target

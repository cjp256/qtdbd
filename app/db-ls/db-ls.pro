include(../../defaults.pri)

QT += core
QT += gui
QT += dbus

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

LIBS += -L../../src -ldb

PRE_TARGETDEPS += ../../src/libdb.a

target.path = /usr/sbin
INSTALLS += target

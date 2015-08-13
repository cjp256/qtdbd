include(../../defaults.pri)

QT += core
QT += gui
QT += dbus

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app
LIBS += -lqmjson

SOURCES += main.cpp

LIBS += -L../../src -ldb -lqmjson

PRE_TARGETDEPS += ../../src/libdb.a

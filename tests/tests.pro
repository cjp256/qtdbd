include(../defaults.pri)

QT += core
QT += gui
QT += dbus
QT += testlib

CONFIG += c++11
CONFIG -= app_bundle

TEMPLATE = app
LIBS += -lqmjson

SOURCES += testdbd.cpp
HEADERS += testdbd.h

LIBS += -L../src -ldb -lqmjson

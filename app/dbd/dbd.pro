include(../../defaults.pri)

QMAKE_LFLAGS += -rdynamic

QT += core
QT -= gui
QT += dbus

CONFIG += c++11
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += backtrace.cpp main.cpp

LIBS += -L../../src -ldb -lqmjson -lxenstore

PRE_TARGETDEPS += ../../src/libdb.a

target.path = /usr/bin
INSTALLS += target

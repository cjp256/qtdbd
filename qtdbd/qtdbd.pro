#-------------------------------------------------
#
# Project created by QtCreator 2015-07-29T10:24:18
#
#-------------------------------------------------

QT       += core
QT += gui
QT += dbus

TARGET = qtdbd
CONFIG   += console
CONFIG   -= app_bundle
CONFIG += c++11

TEMPLATE = app

LIBS += -lqmjson

SOURCES += main.cpp \
    db_adaptor.cpp \
    db.cpp \
    simplejsondb.cpp \
    dbtree.cpp
#    tests/testdbtree.cpp

HEADERS += \
    db_adaptor.h \
    db.h \
    simplejsondb.h \
    dbtree.h
#    tests/testdbtree.h

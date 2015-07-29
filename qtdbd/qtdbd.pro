#-------------------------------------------------
#
# Project created by QtCreator 2015-07-29T10:24:18
#
#-------------------------------------------------

QT       += core
QT -= gui
QT += dbus

TARGET = qtdbd
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    db_adaptor.cpp \
    db.cpp

HEADERS += \
    db_adaptor.h \
    db.h

include(../defaults.pri)

QT += core
QT += gui
QT += dbus

TARGET = db
CONFIG += c++11
CONFIG += static

TEMPLATE = lib

LIBS += -lqmjson

SOURCES += \
    db_adaptor.cpp \
    db.cpp \
    simplejsondb.cpp \
    dbtree.cpp

HEADERS += \
    db_adaptor.h \
    db.h \
    simplejsondb.h \
    dbtree.h

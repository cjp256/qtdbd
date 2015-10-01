include(../defaults.pri)

QT += core
QT += gui
QT += dbus

TARGET = db
CONFIG += c++11
CONFIG += static

TEMPLATE = lib

LIBS += -lqmjson -lxenstore

SOURCES += \
    db.cpp \
    simplejsondb.cpp \
    dbtree.cpp \
    dbinterfaceadaptor.cpp \
    comcitrixxenclientdbinterface.cpp

HEADERS += \
    db.h \
    simplejsondb.h \
    dbtree.h \
    dbinterfaceadaptor.h \
    comcitrixxenclientdbinterface.h

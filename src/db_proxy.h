/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -p db_proxy ../db.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef DB_PROXY_H
#define DB_PROXY_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.citrix.xenclient.db
 */
class ComCitrixXenclientDbInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "com.citrix.xenclient.db"; }

public:
    ComCitrixXenclientDbInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~ComCitrixXenclientDbInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<QString> dump(const QString &path)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(path);
        return asyncCallWithArgumentList(QStringLiteral("dump"), argumentList);
    }

    inline QDBusPendingReply<bool> exists(const QString &path)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(path);
        return asyncCallWithArgumentList(QStringLiteral("exists"), argumentList);
    }

    inline QDBusPendingReply<> inject(const QString &path, const QString &value)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(path) << QVariant::fromValue(value);
        return asyncCallWithArgumentList(QStringLiteral("inject"), argumentList);
    }

    inline QDBusPendingReply<QStringList> list(const QString &path)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(path);
        return asyncCallWithArgumentList(QStringLiteral("list"), argumentList);
    }

    inline QDBusPendingReply<QString> read(const QString &path)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(path);
        return asyncCallWithArgumentList(QStringLiteral("read"), argumentList);
    }

    inline QDBusPendingReply<QByteArray> read_binary(const QString &path)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(path);
        return asyncCallWithArgumentList(QStringLiteral("read_binary"), argumentList);
    }

    inline QDBusPendingReply<> rm(const QString &path)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(path);
        return asyncCallWithArgumentList(QStringLiteral("rm"), argumentList);
    }

    inline QDBusPendingReply<> write(const QString &path, const QString &value)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(path) << QVariant::fromValue(value);
        return asyncCallWithArgumentList(QStringLiteral("write"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

namespace com {
  namespace citrix {
    namespace xenclient {
      typedef ::ComCitrixXenclientDbInterface db;
    }
  }
}
#endif
/**
 * Copyright (c) 2015 Assured Information Security, Inc. <pattersonc@ainfosec.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "db.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDebug>
#include <QtDBus/QtDBus>

Db::Db(DBTree *dbTree, bool lookupSenderId) : dbTree(dbTree), lookupSenderId(lookupSenderId)
{
    // try opening xenstore handle so it's available for later use
    xs = xs_daemon_open();
}

Db::~Db()
{
    // destructor
}

const QString Db::getSenderId()
{
    if (lookupSenderId)
    {
        return message().service();
    }
    else
    {
        return QString("");
    }
}

int Db::getSenderDomId()
{
    const QString senderId = message().service();

    // use only domid == 0 if lookup is disabled
    if (!lookupSenderId)
    {
        return 0;
    }

    QDBusInterface iface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", QDBusConnection::systemBus());
    if (iface.isValid())
    {
        QDBusPendingReply<int> reply = iface.asyncCall("GetConnectionDOMID", senderId);
        reply.waitForFinished();
        if (reply.isValid())
        {
            return reply.value();
        }

        qWarning() << "failed to read sender domid for sender id:" << senderId << "error:" << reply.error().name();
        return -1;
    }

    qWarning() << "failed to get valid dbus interface:" << iface.lastError().name();
    return -1;
}

QString Db::getUuidFromDomId(int domid)
{
    unsigned int len;

    if (xs == NULL)
    {
        xs = xs_daemon_open();
        if (xs == NULL)
        {
            qWarning() << "unable to open xenstore";
            return QString();
        }
    }

    QString vmPathQS = "/local/domain/" + QString::number(domid) + "/vm";
    QByteArray pba = vmPathQS.toLatin1();
    const char *cpa = pba.data();
    qDebug() << "performing xs_read for:" << cpa;

    char *uuidPath = (char *)xs_read(xs, 0, cpa, &len);

    if (uuidPath == NULL)
    {
        qWarning() << "unable to read vm path from xenstore for:" << vmPathQS;
        return QString();
    }

    qDebug() << "xs_read for:" << cpa << "returned:" << uuidPath;

    QString uuidPathQS = QString(uuidPath) + "/uuid";
    free(uuidPath);

    pba = uuidPathQS.toLatin1();
    cpa = pba.data();
    qDebug() << "performing xs_read for:" << cpa;

    char *uuid = (char *)xs_read(xs, 0, cpa, &len);

    if (uuid == NULL)
    {
        qWarning() << "unable to read uuid from xenstore for:" << uuidPathQS;
        return QString();
    }

    qDebug() << "xs_read for:" << cpa << "returned:" << uuid;

    QString uuidQS = QString(uuid);
    free(uuid);

    return uuidQS;
}

bool Db::senderPathSplit(QString path, QStringList &splitPath)
{
    // make sure input list is clear then populate path parts
    splitPath.clear();
    splitPath.append(path.split("/", QString::SplitBehavior::SkipEmptyParts));

    if (!lookupSenderId)
    {
        return true;
    }

    // no mods required if domid == 0
    int domid = getSenderDomId();

    // return invalid string on error
    if (domid < 0)
    {
        qWarning() << "unable to lookup sender domid";
        return false;
    }

    if (domid == 0)
    {
        return true;
    }

    // if domid != 0, then add domstore pathing /dom-store/<uuid>/
    QString uuid = getUuidFromDomId(domid);
    splitPath.insert(0, uuid);
    splitPath.insert(0, "dom-store");
    return true;
}

QString Db::dump(const QString &path)
{
    qDebug() << getSenderId() << " dump(" << path << ")";

    QStringList split;

    if (!senderPathSplit(path, split))
    {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return "";
    }

    QMPointer<QMJsonValue> value = dbTree->getValue(split);

    if (value.isNull())
    {
        qDebug() << "dump() no object found";
        return "null";
    }

    return value->toJson(QMJsonFormat_Optimized);
}

bool Db::exists(const QString &path)
{
    qDebug() << getSenderId() << " exists(" << path << ")";

    QStringList split;
    if (!senderPathSplit(path, split))
    {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return false;
    }

    QMPointer<QMJsonValue> value = dbTree->getValue(split);

    return !value.isNull();
}

void Db::inject(const QString &path, const QString &value)
{
    qDebug() << getSenderId() << " inject(" << path << ", " << value << ")";

    QStringList split;
    if (!senderPathSplit(path, split))
    {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return;
    }

    auto jsonValue = QMPointer<QMJsonValue>(QMJsonValue::fromJson(value));

    if (jsonValue.isNull())
    {
        sendErrorReply(QDBusError::InternalError, "invalid json value");
        return;
    }

    dbTree->mergeValue(split, jsonValue);
}

QStringList Db::list(const QString &path)
{
    qDebug() << getSenderId() << " list(" << path << ")";

    QStringList split;
    if (!senderPathSplit(path, split))
    {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return QStringList();
    }

    QMPointer<QMJsonValue> value = dbTree->getValue(split);

    if (value.isNull() || !value->isObject())
    {
        return QStringList();
    }

    QStringList retList = value->toObject()->keys();

    // XXX COMPATIBILITY HACK:
    // the old dbd returned sorted lists sorted numerically when a list of numbers
    // for lulz, we'll also sort non-integer lists as well, but do not rely on these
    // behaviors as the user should be expected to sort it themselves (however they see fit)
    bool listIsIntegers = true;
    foreach (const QString &str, retList)
    {
        str.toInt(&listIsIntegers);
        if (!listIsIntegers)
        {
            break;
        }
    }

    if (listIsIntegers)
    {
        struct CompareIntegerStrings
        {
            static bool lt(const QString & s0, const QString & s1) { return(s0.toInt() < s1.toInt()); }
        };

        qSort(retList.begin(), retList.end(), CompareIntegerStrings::lt);
    }
    else
    {
        retList.sort();
    }

    return retList;
}

QString Db::read(const QString &path)
{
    qDebug() << getSenderId() << " read(" << path << ")";

    QStringList split;
    if (!senderPathSplit(path, split))
    {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return "";
    }

    QMPointer<QMJsonValue> value = dbTree->getValue(split);

    if (value.isNull())
    {
        qDebug() << "read() no object found";
        return "";
    }

    if (value->isBool())
    {
        if (value->toBool())
        {
            return "true";
        }

        return "false";
    }

    if (value->isString())
    {
        return value->toString();
    }

    if (value->isDouble())
    {
        return QString::number(value->toDouble());
    }

    if (value->isObject() || value->isArray())
    {
        return "";
    }

    if (value->isNull())
    {
        return "null";
    }

    qWarning() << "read(): invalid value type!" << value;
    return "";
}


QByteArray Db::read_binary(const QString &path)
{
    qDebug() << getSenderId() << " read_binary(" << path << ")";

    return QByteArray();
}

void Db::rm(const QString &path)
{
    qDebug() << getSenderId() << " rm(" << path << ")";

    QStringList split;

    if (!senderPathSplit(path, split))
    {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return;
    }

    dbTree->rmValue(split);
}

void Db::write(const QString &path, const QString &value)
{
    qDebug() << getSenderId() << " write(" << path << ", " << value << ")";

    QStringList split;

    if (!senderPathSplit(path, split))
    {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return;
    }

    auto jsonValue = QMPointer<QMJsonValue>(new QMJsonValue(value));

    if (jsonValue.isNull())
    {
        sendErrorReply(QDBusError::InternalError, "invalid json value");
        return;
    }

    dbTree->setValue(split, jsonValue);
}

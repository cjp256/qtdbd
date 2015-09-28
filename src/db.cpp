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

}

Db::~Db()
{
    // destructor
}

QString Db::getSenderId()
{
    QString senderId = message().service();
    if (lookupSenderId) {
        QDBusInterface iface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", QDBusConnection::systemBus());
        if (iface.isValid()) {
            QDBusPendingReply<int> reply = iface.asyncCall("GetConnectionDOMID", senderId);
            reply.waitForFinished();
            if (reply.isValid()) {
                return QString(reply.value());
            }

            qWarning() << "failed to read sender domid for sender id:" << senderId << "error:" << reply.error().name();
            return QString();
        }
    }

    return senderId;
}

QString Db::dump(const QString &path)
{
    qDebug() << getSenderId() << " dump(" << path << ")";

    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);
    QMPointer<QMJsonValue> value = dbTree->getValue(split);

    if (value.isNull()) {
        qDebug() << "read() no object found";
        return "null";
    }

    return value->toJson(QMJsonFormat_Optimized);
}

bool Db::exists(const QString &path)
{
    qDebug() << getSenderId() << " exists(" << path << ")";

    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);
    QMPointer<QMJsonValue> value = dbTree->getValue(split);

    return !value.isNull();
}

void Db::inject(const QString &path, const QString &value)
{
    qDebug() << getSenderId() << " inject(" << path << ", " << value << ")";

    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);

    dbTree->mergeValue(split, QMPointer<QMJsonValue>(QMJsonValue::fromJson(value)));
}

QStringList Db::list(const QString &path)
{
    qDebug() << getSenderId() << " list(" << path << ")";

    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);
    QMPointer<QMJsonValue> value = dbTree->getValue(split);

    if (value.isNull() || !value->isObject()) {
        return QStringList();
    }

    return value->toObject()->keys();
}

QString Db::read(const QString &path)
{
    qDebug() << getSenderId() << " read(" << path << ")";

    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);
    QMPointer<QMJsonValue> value = dbTree->getValue(split);

    if (value.isNull()) {
        qDebug() << "read() no object found";
        return "";
    }

    if (value->isBool()) {
        if (value->toBool()) {
            return "true";
        }

        return "false";
    }

    if (value->isString()) {
        return value->toString();
    }

    if (value->isDouble()) {
        return QString::number(value->toDouble());
    }

    if (value->isObject() || value->isArray()) {
        return "";
    }

    if (value->isNull()) {
        return "null";
    }

    qWarning() << "read(): invalid value type!" << value;
    return "";
}


QByteArray Db::read_binary(const QString &path)
{
    qDebug() << getSenderId() << " read_binary(" << path << ")";

    QByteArray value;

    return value;
}

void Db::rm(const QString &path)
{
    qDebug() << getSenderId() << " rm(" << path << ")";

    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);

    dbTree->rmValue(split);
}

void Db::write(const QString &path, const QString &value)
{
    qDebug() << getSenderId() << " write(" << path << ", " << value << ")";

    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);

    dbTree->setValue(split, QMPointer<QMJsonValue>(new QMJsonValue(value)));
}

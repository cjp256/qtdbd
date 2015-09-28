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

const QString Db::getSenderId()
{
    if (lookupSenderId) {
        return message().service();
    } else {
        return QString("");
    }
}

int Db::getSenderDomId()
{
    const QString senderId = message().service();

    // use only domid == 0 if lookup is disabled
    if (!lookupSenderId) {
        return 0;
    }

    QDBusInterface iface("org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus", QDBusConnection::systemBus());
    if (iface.isValid()) {
        QDBusPendingReply<int> reply = iface.asyncCall("GetConnectionDOMID", senderId);
        reply.waitForFinished();
        if (reply.isValid()) {
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
    // vm_path = self.xenstore.read(0, '/local/domain/' + str(domid) + '/vm')
    // # The UUID is actually in the vm_path, but this is the old way...
    // uuid = self.xenstore.read(0, vm_path + '/uuid')

    return QString();
}

bool Db::senderPathSplit(QString path, QStringList &splitPath)
{
    // make sure input list is clear then populate path parts
    splitPath.clear();
    splitPath.append(path.split("/", QString::SplitBehavior::SkipEmptyParts));

    if (!lookupSenderId) {
        return true;
    }

    // no mods required if domid == 0
    int domid = getSenderDomId();

    // return invalid string on error
    if (domid < 0) {
        qWarning() << "unable to lookup sender domid";
        return false;
    }

    if (domid == 0) {
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
    if (!senderPathSplit(path, split)) {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return "";
    }

    QMPointer<QMJsonValue> value = dbTree->getValue(split);
    if (value.isNull()) {
        qDebug() << "dump() no object found";
        return "null";
    }

    return value->toJson(QMJsonFormat_Optimized);
}

bool Db::exists(const QString &path)
{
    qDebug() << getSenderId() << " exists(" << path << ")";

    QStringList split;
    if (!senderPathSplit(path, split)) {
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
    if (!senderPathSplit(path, split)) {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return;
    }

    dbTree->mergeValue(split, QMPointer<QMJsonValue>(QMJsonValue::fromJson(value)));
}

QStringList Db::list(const QString &path)
{
    qDebug() << getSenderId() << " list(" << path << ")";

    QStringList split;
    if (!senderPathSplit(path, split)) {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return QStringList();
    }

    QMPointer<QMJsonValue> value = dbTree->getValue(split);

    if (value.isNull() || !value->isObject()) {
        return QStringList();
    }

    return value->toObject()->keys();
}

QString Db::read(const QString &path)
{
    qDebug() << getSenderId() << " read(" << path << ")";

    QStringList split;
    if (!senderPathSplit(path, split)) {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return "";
    }

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

    QStringList split;
    if (!senderPathSplit(path, split)) {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return;
    }

    dbTree->rmValue(split);
}

void Db::write(const QString &path, const QString &value)
{
    qDebug() << getSenderId() << " write(" << path << ", " << value << ")";

    QStringList split;
    if (!senderPathSplit(path, split)) {
        sendErrorReply(QDBusError::InternalError, "unable to lookup sender domid");
        return;
    }

    dbTree->setValue(split, QMPointer<QMJsonValue>(new QMJsonValue(value)));
}

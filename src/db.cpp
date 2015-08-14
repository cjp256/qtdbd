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
    if (lookupSenderId) {
        return message().service();
    } else {
        return "";
    }
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

    return value->toJson(QMJSONVALUE_OPTIMIZED);
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
}

QStringList Db::list(const QString &path)
{
    qDebug() << getSenderId() << " list(" << path << ")";

    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);

    QMPointer<QMJsonValue> value = dbTree->getValue(split);

    if (!value->isObject()) {
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

    qDebug() << "value=" << value << " toJson=" << value->toJson() << "type=" << value->type() << "toString=" << value->toString();

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

    qWarning("read(): invalid value type!");
    return "";
}


QByteArray Db::read_binary(const QString &path)
{
    QByteArray value;
    qDebug() << getSenderId() << " read_binary(" << path << ")";
    return value;
}

void Db::rm(const QString &path)
{
    qDebug() << getSenderId() << " rm(" << path << ")";
}

void Db::write(const QString &path, const QString &value)
{
    qDebug() << getSenderId() << " write(" << path << ", " << value << ")";
    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);
    dbTree->setValue(split, value);
}

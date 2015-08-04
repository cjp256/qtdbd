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

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;

Db::Db(DBTree *dbTree) : dbTree(dbTree)
{

}

Db::~Db()
{
    // destructor
}

QString Db::dump(const QString &path)
{
    QString value("dump");
    qDebug() << message().service() << " dump(" << path << ")";
    return value;
}

bool Db::exists(const QString &path)
{
    bool ex = true;
    qDebug() << message().service() << " exists(" << path << ")";
    return ex;
}

void Db::inject(const QString &path, const QString &value)
{
    qDebug() << message().service() << " inject(" << path << ", " << value << ")";
}

QStringList Db::list(const QString &path)
{
    QStringList value;
    qDebug() << message().service() << " list(" << path << ")";
    return value;
}

QString Db::read(const QString &path)
{
    qDebug() << message().service() << " read(" << path << ")";
    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);

    Value *obj = dbTree->getObject(split);

    if (obj == nullptr) {
        qDebug() << "read() null object";
        return "";
    }

    QString retVal;

    switch (obj->GetType())
    {
    case kFalseType:
        retVal = "false";
        break;
    case kTrueType:
        retVal = "true";
        break;
    case kObjectType:
        retVal = "";
        break;
    case kArrayType:
        retVal = "";
        break;
    case kStringType:
        retVal = obj->GetString();
        break;
    case kNumberType:
        retVal = QString::number(obj->GetInt64());
        break;
    case kNullType:
        retVal = "null";
        break;
    default:
        qWarning("read(): invalid object type!");
        retVal = "";
        break;
    }

    qDebug() << "returning:" << retVal;

    return retVal;
}

QByteArray Db::read_binary(const QString &path)
{
    QByteArray value;
    qDebug() << message().service() << " read_binary(" << path << ")";
    return value;
}

void Db::rm(const QString &path)
{
    qDebug() << message().service() << " rm(" << path << ")";
}

void Db::write(const QString &path, const QString &value)
{
    qDebug() << message().service() << " write(" << path << ", " << value << ")";
    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);
    Value x;
    x.SetString(value.toLocal8Bit().data(), dbTree->dbRoot.GetAllocator());
    dbTree->setObject(split, x);
}


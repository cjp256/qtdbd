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
    QString value("");
    qDebug() << message().service() << " read(" << path << ")";
    QStringList split = path.split("/", QString::SplitBehavior::SkipEmptyParts);

    QVariant obj = dbTree->getObject(split, QString(""));
    qDebug() << "read() object:" << obj;

    value = obj.toString();

    qDebug() << "returning:" << value;
    return value;
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
    dbTree->setObject(split, value);
}


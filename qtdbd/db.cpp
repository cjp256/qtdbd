#include "db.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDebug>

Db::Db()
{
    // constructor
}

Db::~Db()
{
    // destructor
}

QString Db::dump(const QString &path)
{
    QString value("dump");
    qDebug() << "dump";
    return value;
}

bool Db::exists(const QString &path)
{
    bool ex = true;
    qDebug() << "exists";
    return ex;
}

void Db::inject(const QString &path, const QString &value)
{
    qDebug() << "inject";
}

QStringList Db::list(const QString &path)
{
    QStringList value;
    qDebug() << "list";
    return value;
}

QString Db::read(const QString &path)
{
    QString value;
    qDebug() << "read";
    return value;
}

QByteArray Db::read_binary(const QString &path)
{
    // handle method call com.citrix.xenclient.db.read_binary
    QByteArray value;
    qDebug() << "read_binary";
    return value;
}

void Db::rm(const QString &path)
{
    qDebug() << "rm";
}

void Db::write(const QString &path, const QString &value)
{
    qDebug() << "write";
}


#ifndef DB_H
#define DB_H

#include <QObject>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusMessage>
#include "dbtree.h"
#include <qmjson.h>

class Db: public QObject,
          protected QDBusContext
{
    Q_OBJECT
public:
    Db(DBTree *dbTree);
    ~Db();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QString dump(const QString &path);
    bool exists(const QString &path);
    void inject(const QString &path, const QString &value);
    QStringList list(const QString &path);
    QString read(const QString &path);
    QByteArray read_binary(const QString &path);
    void rm(const QString &path);
    void write(const QString &path, const QString &value);
Q_SIGNALS: // SIGNALS
private:
    DBTree *dbTree;
};

#endif // DB_H

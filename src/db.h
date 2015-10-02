#ifndef DB_H
#define DB_H

extern "C"
{
#include <stdlib.h>
#include <xs.h>
}

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
    Q_CLASSINFO("D-Bus Interface", "com.citrix.xenclient.db")
public:
    Db(DBTree *dbTree, bool lookupSenderId);
    ~Db();

    const QString getSenderId();
    int getSenderDomId();
    QString getUuidFromDomId(int domid);
    bool senderPathSplit(QString path, QStringList &splitPathOut);

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
    bool lookupSenderId;
    struct xs_handle *xs;
};

#endif // DB_H

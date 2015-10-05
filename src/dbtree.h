#ifndef DBTREE_H
#define DBTREE_H

#include <QObject>
#include <QThread>
#include <QHash>
#include <QSharedPointer>
#include <qmjson.h>
#include "simplejsondb.h"

class DBTree : public QObject
{
    Q_OBJECT
public:
    DBTree(const QString dbPath = ":memory:", int maxFlushDelayMillis = 3000);
    ~DBTree();
    QSharedPointer<SimpleJsonDB> createChildDb(const QString parentPath, const QString topLevel, const QString secondLevel, QHash<QString, QSharedPointer<SimpleJsonDB>> &dbs);
    void loadChildren(const QString path, const QString key, QHash<QString, QSharedPointer<SimpleJsonDB>> &dbs);
    void loadTree();
    QSharedPointer<SimpleJsonDB> lookupDb(const QStringList &splitPath, bool createEmpty);
    QMPointer<QMJsonValue> getValue(const QStringList &splitPath);
    void setValue(const QStringList &splitPath, QMPointer<QMJsonValue> value, bool skipFlush=false);
    void mergeValue(const QStringList &splitPath, QMPointer<QMJsonValue> value);
    void rmValue(const QStringList &splitPath);

    QMPointer<QMJsonValue> dbRoot;

private:
    QString dbPath;
    int maxFlushDelay;
    QSharedPointer<SimpleJsonDB> mainDb;
    QThread dbWriterThread;
    QHash<QString, QSharedPointer<SimpleJsonDB>> vmsDbs;
    QHash<QString, QSharedPointer<SimpleJsonDB>> domstoreDbs;

signals:
public slots:
    void exitCleanup();

};

#endif // DBTREE_H

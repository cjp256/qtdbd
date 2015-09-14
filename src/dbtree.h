#ifndef DBTREE_H
#define DBTREE_H

#include <QObject>
#include <QThread>
#include <qmjson.h>
#include "simplejsondb.h"

class DBTree : public QObject
{
    Q_OBJECT
public:
    DBTree(const QString dbPath = ":memory:", int maxFlushDelayMillis = 3000);
    ~DBTree();
    QMPointer<QMJsonValue> getValue(const QStringList &splitPath);
    void setValue(const QStringList &splitPath, const QString &value);
    void mergeValue(const QStringList &splitPath, const QString &value);
    void rmValue(const QStringList &splitPath);
private:
    QMPointer<QMJsonValue> dbRoot;
    QString dbPath;
    int maxFlushDelay;
    QSharedPointer<SimpleJsonDB> mainDb;
    QThread dbWriterThread;
signals:
public slots:
};

#endif // DBTREE_H

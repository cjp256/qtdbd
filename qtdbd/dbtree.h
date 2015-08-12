#ifndef DBTREE_H
#define DBTREE_H

#include <QObject>
#include <qmjson.h>
#include "simplejsondb.h"

class DBTree : public QObject
{
    Q_OBJECT
public:
    DBTree();
    DBTree(const QString dbPath, int maxFlushDelayMillis);
    ~DBTree();
    QMPointer<QMJsonValue> getObject(const QStringList &splitPath, const QMPointer<QMJsonValue> defaultValue);
    void setObject(const QStringList &splitPath, const QString &value);
private:
    QMPointer<QMJsonValue> dbRoot;
    QString dbPath;
    int maxFlushDelay;
    SimpleJsonDB mainDb;
signals:
public slots:
};

#endif // DBTREE_H

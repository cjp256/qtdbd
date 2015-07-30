#ifndef DBTREE_H
#define DBTREE_H

#include <QObject>
#include "simplejsondb.h"

class DBTree : public QObject
{
    Q_OBJECT
public:
    DBTree(QString dbPath, int maxFlushDelayMillis);
    ~DBTree();
private:
    QString dbPath;
    int maxFlushDelay;
    SimpleJsonDB *mainDB;
signals:

public slots:
};

#endif // DBTREE_H

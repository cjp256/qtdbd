#include "dbtree.h"
#include "simplejsondb.h"
#include <QDir>
#include <QDebug>

DBTree::DBTree(QString dbPath, int maxFlushDelayMillis) : dbPath(dbPath), maxFlushDelay(maxFlushDelayMillis)
{
    qDebug() << "DBTree init(" << dbPath << "," << maxFlushDelay << ")";

    mainDB = new SimpleJsonDB(QDir(dbPath).filePath("db"), "", maxFlushDelay);
}

DBTree::~DBTree()
{

}

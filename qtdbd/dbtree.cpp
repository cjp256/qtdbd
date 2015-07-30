#include "dbtree.h"
#include "simplejsondb.h"
#include <QDir>
#include <QDebug>
#include <QVariant>

DBTree::DBTree(QString dbPath, int maxFlushDelayMillis) : dbPath(dbPath), maxFlushDelay(maxFlushDelayMillis)
{
    qDebug() << "DBTree init(" << dbPath << "," << maxFlushDelay << ")";

    mainDB = new SimpleJsonDB(QDir(dbPath).filePath("db"), "", maxFlushDelay);
    mainDB->filterVmAndDomstoreKeys = true;
    mainDB->dbHashTable.insert("me", "iwuzhere");
    mainDB->writeToDisk();
}

DBTree::~DBTree()
{

}

QVariant DBTree::getObject(QStringList &splitPath, const QVariant &defaultValue)
{
    QVariant obj(mainDB->dbHashTable);

    // if it is top of tree, return the whole tree
    if (splitPath.length() == 0) {
        return obj;
    }

    // traverse tree parts
    foreach (const QString &part, splitPath) {
        qDebug() << "part:" << part;

        // make sure next level is a hashtable before entering
        if (obj.type() == QVariant::Hash) {
            QVariantHash nextHash(obj.toHash());
            obj = nextHash.value(part, QVariant());
        } else if (obj.type() == QVariant::Map) {
            QVariantMap nextMap(obj.toMap());
            obj = nextMap.value(part, QVariant());
        } else {
            qDebug() << "getObject() failed to traverse path:" << obj;
            return defaultValue;
        }

        qDebug() << "getObject() next object:" << obj;

        // if it doesn't exist, return default value
        if (!obj.isValid()) {
            return defaultValue;
        }
    }

    qDebug() << "getObject() returning object:" << obj;
    return obj;
}

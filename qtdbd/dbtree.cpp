#include "dbtree.h"
#include "simplejsondb.h"
#include <QDir>
#include <QDebug>
#include <QVariant>
#include <QHash>

DBTree::DBTree(QString dbPath, int maxFlushDelayMillis) : dbPath(dbPath), maxFlushDelay(maxFlushDelayMillis)
{
    qDebug() << "DBTree init(" << dbPath << "," << maxFlushDelay << ")";

    mainDB = new SimpleJsonDB(QDir(dbPath).filePath("db"), "", maxFlushDelay);
    mainDB->filterVmAndDomstoreKeys = true;
    mainDB->debugJsonObject();
    mainDB->dbMap.insert("me", "iwuzhere");
    mainDB->debugJsonObject();
    mainDB->dbMap.find("owner").value().toMap().insert("me", "iwuzhere2");
    mainDB->debugJsonObject();
    mainDB->writeToDisk();
}

DBTree::~DBTree()
{

}

QVariant DBTree::getObject(const QStringList &splitPath, const QVariant &defaultValue)
{
    QVariant obj(mainDB->dbMap);

    // if it is top of tree, return the whole tree
    if (splitPath.length() == 0) {
        return obj;
    }

    // traverse tree parts
    foreach (const QString &part, splitPath) {
        qDebug() << "getObject: part:" << part << "obj:" << obj << "type:" << obj.type();

        // make sure next level is a map
        if (obj.type() == QVariant::Map) {
            qDebug() << "getObject() is variant map";
            QVariantMap nextMap = obj.toMap();
            obj = nextMap.value(part, QVariant());
        } else {
            qDebug() << "getObject() failed to traverse path:" << obj;
            return defaultValue;
        }

        qDebug() << "getObject: next object:" << obj;

        // if it doesn't exist, return default value
        if (!obj.isValid()) {
            return defaultValue;
        }
    }

    qDebug() << "getObject: returning object:" << obj;
    return obj;
}

void DBTree::setObject(QStringList splitPath, const QVariant &value)
{
    QVariantMap obj = mainDB->dbMap;

    // if it is top of tree, ignore
    if (splitPath.length() == 0) {
        qWarning("setObject: ignoring attempt to write to root");
        return;
    }

    // split key from parent path values
    QString key = splitPath.takeLast();

    // make tree as required
    foreach (const QString &part, splitPath) {
        qDebug() << "setObject: part:" << part;

        if (!obj.contains(part)) {
            qDebug() << "setObject: creating empty map for key part:" << part;
            QVariantMap next = QVariantMap();
            obj.insert(part, next);
        }
        obj = obj.value(part, QVariantMap()).toMap();
        qDebug() << "setObject: next object:" << obj;
    }
    obj.insert(key, value.toString());
    qDebug() << "setObject: set" << key << "to:" << value;
}

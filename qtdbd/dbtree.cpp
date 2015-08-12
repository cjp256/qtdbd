#include "dbtree.h"
#include "simplejsondb.h"
#include <QDir>
#include <QDebug>
#include <QVariant>
#include <QHash>
#include <qmjson.h>

DBTree::DBTree(QString dbPath, int maxFlushDelayMillis) : dbRoot(), maxFlushDelay(maxFlushDelayMillis), mainDb(nullptr), dbWriterThread(this)
{
    qDebug() << "DBTree init(" << dbPath << "," << maxFlushDelay << ")";

    if (dbPath == ":memory:") {
        mainDb = QSharedPointer<SimpleJsonDB>(new SimpleJsonDB(QString(""), QString(":memory:"), maxFlushDelayMillis));
    } else {
        mainDb = QSharedPointer<SimpleJsonDB>(new SimpleJsonDB(QString(""), QDir(dbPath).filePath("db"), maxFlushDelayMillis));
    }
    mainDb->setFilterVmAndDomstoreKeys(true);
    dbRoot = mainDb->readFromDisk();
}

DBTree::~DBTree()
{
}

QMPointer<QMJsonValue> DBTree::getObject(const QStringList &splitPath, const QMPointer<QMJsonValue> defaultValue)
{
    QMPointer<QMJsonValue> obj = dbRoot;

    qDebug() << "getObject(): db = " << obj;

    // if it is top of tree, return the whole tree
    if (splitPath.length() == 0) {
        return obj;
    }

    // traverse tree parts
    foreach (const QString &part, splitPath) {
        qDebug() << "getObject: part:" << part << "obj:" << obj << "type:" << obj->type();

        // make sure next level is an object
        if (!obj->isObject()) {
            qDebug() << "getObject() failed to traverse path:" << obj;
            return defaultValue;
        }

        obj = obj->toObject()->value(part);

        qDebug() << "getObject: next object:" << obj;
    }

    qDebug() << "getObject: returning object:" << obj;
    return obj;
}

void DBTree::setObject(const QStringList &splitPath, const QString &value)
{
    QMPointer<QMJsonValue> obj = dbRoot;

    // if it is top of tree, ignore
    if (splitPath.length() == 0) {
        qWarning("setObject: ignoring attempt to write to root");
        return;
    }

    // split key from parent path values
    QStringList parentList(splitPath);
    QString key = parentList.takeLast();

    // make tree as required
    foreach (const QString &part, parentList) {
        qDebug() << "setObject: part:" << part;

        if (!obj->toObject()->contains(part)) {
            obj->toObject()->insert(part, QMPointer<QMJsonObject>(new QMJsonObject()));
        }

        obj = obj->toObject()->value(part);

        // make sure next level is an object
        if (!obj->isObject()) {
            qDebug() << "setObject() failed to traverse path:" << obj;
            return;
        }
    }

    obj->toObject()->insert(key, QMPointer<QMJsonValue>(new QMJsonValue(value)));
    return;
}

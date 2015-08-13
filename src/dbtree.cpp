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

QMPointer<QMJsonValue> DBTree::getValue(const QStringList &splitPath)
{
    QMPointer<QMJsonValue> obj = dbRoot;

    qDebug() << "getValue(): db = " << obj->toJson();

    // if it is top of tree, return the whole tree
    if (splitPath.length() == 0) {
        return obj;
    }

    // traverse tree parts
    foreach (const QString &part, splitPath) {
        qDebug() << "getValue: part:" << part << "obj:" << obj << "type:" << obj->type();

        // make sure next level is an object
        if (!obj->isObject()) {
            qDebug() << "getValue() failed to traverse path:" << obj;
            return QMPointer<QMJsonValue>();
        }

        if (!obj->toObject()->contains(part)) {
            qDebug() << "getValue() failed to traverse next path:" << obj;
            return QMPointer<QMJsonValue>();
        }

        obj = obj->toObject()->value(part);

        qDebug() << "getValue: next object:" << obj;
    }

    qDebug() << "getValue: returning object:" << obj;
    return obj;
}

void DBTree::setValue(const QStringList &splitPath, const QString &value)
{
    QMPointer<QMJsonValue> obj = dbRoot;

    // if it is top of tree, ignore
    if (splitPath.length() == 0) {
        qWarning("setValue: ignoring attempt to write to root");
        return;
    }

    // split key from parent path values
    QStringList parentList(splitPath);
    QString key = parentList.takeLast();

    // make tree as required
    foreach (const QString &part, parentList) {
        qDebug() << "setValue: part:" << part;

        if (!obj->toObject()->contains(part)) {
            obj->toObject()->insert(part, QMPointer<QMJsonObject>(new QMJsonObject()));
        }

        obj = obj->toObject()->value(part);

        // make sure next level is an object
        if (!obj->isObject()) {
            qDebug() << "setValue() failed to traverse path:" << obj;
            return;
        }
    }

    obj->toObject()->insert(key, QMPointer<QMJsonValue>(new QMJsonValue(value)));
    return;
}

#include "dbtree.h"
#include "simplejsondb.h"
#include <QDir>
#include <QDebug>
#include <QVariant>
#include <QHash>
#include <QRegExp>
#include <qmjson.h>

DBTree::DBTree(QString dbPath, int maxFlushDelayMillis) : dbRoot(), maxFlushDelay(maxFlushDelayMillis), mainDb(nullptr), dbWriterThread(this)
{
    qDebug() << "DBTree init(" << dbPath << "," << maxFlushDelay << ")";

    if (dbPath == ":memory:") {
        mainDb = QSharedPointer<SimpleJsonDB>(new SimpleJsonDB(QString(""), QString(":memory:"), maxFlushDelayMillis));
        mainDb->setFilterVmAndDomstoreKeys(true);
        dbRoot = mainDb->getValue();
    } else {
        QDir topDir = QDir(dbPath);
        QDir domstoreDir = QDir(dbPath + QDir::separator() + "dom-store");
        QDir vmsDir = QDir(dbPath + QDir::separator() + "vms");
        QStringList nameFilter;

        nameFilter << "*.db";

        domstoreDir.setNameFilters(nameFilter);
        domstoreDir.setFilter(QDir::Files);
        vmsDir.setNameFilters(nameFilter);
        vmsDir.setFilter(QDir::Files);

        mainDb = QSharedPointer<SimpleJsonDB>(new SimpleJsonDB(QString(""), topDir.filePath("db"), maxFlushDelayMillis));
        mainDb->setFilterVmAndDomstoreKeys(true);
        dbRoot = mainDb->getValue();

        // load children domstore nodes
        QStringList entries = domstoreDir.entryList();
        for( QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry )
        {
            QString fileName = *entry;
            QString filePath = dbPath + QDir::separator() + "vms" + QDir::separator() + fileName;
            qDebug() << "dom-store node: " << filePath;

            QRegExp regex = QRegExp("[0-9a-f]{8}-([0-9a-f]{4}-){3}[0-9a-f]{12}.db");

            if (!regex.exactMatch(fileName)) {
                qDebug() << "ignoring invalid format dom-store db:" << filePath;
                continue;
            }

            QString uuid = fileName.remove(36,3);
            QString vPath = QString("/dom-store/") + uuid;
            QStringList splitPath = vPath.split("/", QString::SplitBehavior::SkipEmptyParts);

            qDebug() << "creating dom-store for uuid:" << uuid;

            auto db = QSharedPointer<SimpleJsonDB>(new SimpleJsonDB(vPath, filePath, maxFlushDelayMillis));
            domstoreDbs.insert(uuid, db);

            setValue(splitPath, db->getValue());

        }

        entries = vmsDir.entryList();
        for( QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry )
        {
            QString fileName = *entry;
            QString filePath = dbPath + QDir::separator() + "vms" + QDir::separator() + fileName;
            qDebug() << "vms node: " << filePath;

            QRegExp regex = QRegExp("[0-9a-f]{8}-([0-9a-f]{4}-){3}[0-9a-f]{12}.db");

            if (!regex.exactMatch(fileName)) {
                qDebug() << "ignoring invalid format vms db:" << filePath;
                continue;
            }

            QString uuid = fileName.remove(36,3);
            QString vPath = QString("/vm/") + uuid;
            QStringList splitPath = vPath.split("/", QString::SplitBehavior::SkipEmptyParts);

            qDebug() << "creating vms for uuid:" << uuid;

            auto db = QSharedPointer<SimpleJsonDB>(new SimpleJsonDB(vPath, filePath, maxFlushDelayMillis));
            vmsDbs.insert(uuid, db);

            setValue(splitPath, db->getValue());
        }
    }
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

void DBTree::setValue(const QStringList &splitPath, QMPointer<QMJsonValue> value)
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
        qDebug() << "setValue: part:" << part << "obj:" << obj;

        if (!obj->toObject()->contains(part)) {
            qDebug() << "setValue() inserting empty object";
            obj->toObject()->insert(part, QMPointer<QMJsonObject>(new QMJsonObject()));
        }

        qDebug() << "setValue() getting value";
        obj = obj->toObject()->value(part);

        // make sure next level is an object
        if (!obj->isObject()) {
            qDebug() << "setValue() failed to traverse path:" << obj;
            return;
        }
    }

    qDebug() << "setValue() inserting value:" << value;
    obj->toObject()->insert(key, value);
    return;
}

void DBTree::rmValue(const QStringList &splitPath)
{
    QMPointer<QMJsonValue> obj = dbRoot;

    // if it is top of tree, ignore
    if (splitPath.length() == 0) {
        qWarning("rmValue: ignoring attempt to write to root");
        return;
    }

    // split key from parent path values
    QStringList parentList(splitPath);
    QString key = parentList.takeLast();

    // iterate through parent objects
    foreach (const QString &part, parentList) {
        qDebug() << "rmValue: part:" << part;

        // bail if parent doesn't exist
        if (!obj->toObject()->contains(part)) {
            return;
        }

        obj = obj->toObject()->value(part);

        // bail if next level is not an object
        if (!obj->isObject()) {
            qDebug() << "rmValue() failed to traverse path:" << obj;
            return;
        }
    }

    obj->toObject()->remove(key);
}

void DBTree::mergeValue(const QStringList &splitPath, QMPointer<QMJsonValue> value)
{
    QMPointer<QMJsonValue> obj = dbRoot;

    // if it is top of tree, ignore
    if (splitPath.length() == 0) {
        qWarning("setValue: ignoring attempt to write to root");
        return;
    }

    QStringList parentList(splitPath);

    // iterate through parent objects
    foreach (const QString &part, parentList) {
        qDebug() << "mergeValue: part:" << part;

        // create missing children nodes
        if (!obj->toObject()->contains(part)) {
            obj->toObject()->insert(part, QMPointer<QMJsonObject>(new QMJsonObject()));
        }

        obj = obj->toObject()->value(part);

        // bail if next level is not an object
        if (!obj->isObject()) {
            qDebug() << "mergeValue() failed to traverse path:" << obj;
            return;
        }
    }

    qDebug() << "mergeValue(): attempting merge obj:" << obj << "value:" << value->toObject();

    obj->toObject()->unite(value->toObject(), QMJsonReplacementPolicy_Replace, QMJsonArrayUnitePolicy_Append);
}

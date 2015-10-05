#include "dbtree.h"
#include "simplejsondb.h"
#include <QDir>
#include <QDebug>
#include <QVariant>
#include <QHash>
#include <QRegExp>
#include <qmjson.h>

DBTree::DBTree(QString dbPath, int maxFlushDelayMillis) : dbRoot(), dbPath(dbPath), maxFlushDelay(maxFlushDelayMillis), mainDb(nullptr), dbWriterThread(this)
{
    qDebug() << "DBTree init(" << dbPath << "," << maxFlushDelay << ")";
    loadTree();
}

DBTree::~DBTree()
{
}

QSharedPointer<SimpleJsonDB> DBTree::createChildDb(const QString parentPath, const QString topLevel, const QString secondLevel, QHash<QString, QSharedPointer<SimpleJsonDB>> &dbs)
{
    QString path = parentPath + QDir::separator() + secondLevel + ".db";
    QString vPath = topLevel + "/" + secondLevel;
    QStringList baseSplitPath;
    baseSplitPath << topLevel << secondLevel;

    qDebug() << "creating db in:" << topLevel << " for uuid:" << secondLevel << "at path:" << path;

    auto db = QSharedPointer<SimpleJsonDB>(new SimpleJsonDB(vPath, path, maxFlushDelay));
    dbs.insert(secondLevel, db);
    setValue(baseSplitPath, db->getValue(), true);
    return db;
}

void DBTree::loadChildren(const QString path, const QString key, QHash<QString, QSharedPointer<SimpleJsonDB>> &dbs)
{
    QDir childrenDir = QDir(path);
    QStringList nameFilter;

    // make sure child directory exists, if not - create it
    if (!childrenDir.exists()) {
        childrenDir.mkpath(".");
    }

    nameFilter << "*.db";

    childrenDir.setNameFilters(nameFilter);
    childrenDir.setFilter(QDir::Files);

    // load children db nodes
    QStringList entries = childrenDir.entryList();
    for( QStringList::ConstIterator entry=entries.begin(); entry!=entries.end(); ++entry )
    {
        QString fileName = *entry;
        QString filePath = path + QDir::separator() + fileName;
        qDebug() << "path:" << path << "node: " << filePath;

        QRegExp regex = QRegExp("[0-9a-f]{8}-([0-9a-f]{4}-){3}[0-9a-f]{12}.db");

        if (!regex.exactMatch(fileName)) {
            qDebug() << "ignoring invalid format db:" << filePath;
            continue;
        }

        QString uuid = fileName.remove(36,3);
        createChildDb(path, key, uuid, dbs);
    }
}

void DBTree::loadTree()
{
    if (dbPath == ":memory:") {
        mainDb = QSharedPointer<SimpleJsonDB>(new SimpleJsonDB(QString(""), QString(":memory:"), maxFlushDelay));
        mainDb->setFilterVmAndDomstoreKeys(true);
        dbRoot = mainDb->getValue();
    } else {
        QString topDbPath = dbPath + QDir::separator() + "db";
        mainDb = QSharedPointer<SimpleJsonDB>(new SimpleJsonDB(QString(""), topDbPath, maxFlushDelay));
        mainDb->setFilterVmAndDomstoreKeys(true);
        dbRoot = mainDb->getValue();

        QString domstorePath = dbPath + QDir::separator() + "dom-store";
        loadChildren(domstorePath, "dom-store", domstoreDbs);

        QString vmsPath = dbPath + QDir::separator() + "vms";
        loadChildren(vmsPath, "vm", vmsDbs);
    }

}

// find existing db, creating one if it does not exist
QSharedPointer<SimpleJsonDB> DBTree::lookupDb(const QStringList &splitPath)
{
    if (splitPath.length() < 2) {
        return mainDb;
    }

    QString topLevel = splitPath[0];
    QString secondLevel = splitPath[1];

    if (topLevel == "dom-store") {
        QHash<QString, QSharedPointer<SimpleJsonDB>>::iterator i = domstoreDbs.find(secondLevel);

        if (i != domstoreDbs.end() && i.key() == secondLevel) {
            return i.value();
        }

        // not found, create one
        return createChildDb(dbPath + QDir::separator() + "dom-store", topLevel, secondLevel, domstoreDbs);
    }

    if (topLevel == "vm") {
        QHash<QString, QSharedPointer<SimpleJsonDB>>::iterator i = vmsDbs.find(secondLevel);

        if (i != vmsDbs.end() && i.key() == secondLevel) {
            return i.value();
        }

        // not found, create one
        return createChildDb(dbPath + QDir::separator() + "vms", topLevel, secondLevel, vmsDbs);
    }

    // if not in domstore or vms db, must be in main db
    return mainDb;
}

QMPointer<QMJsonValue> DBTree::getValue(const QStringList &splitPath)
{
    QMPointer<QMJsonValue> obj = dbRoot;

    qDebug() << "getValue(): splitPath:" << splitPath;

    // if it is top of tree, return the whole tree
    if (splitPath.length() == 0) {
        return obj;
    }

    // traverse tree parts
    QStringList currentSplitPath;
    for (const QString &part : splitPath) {
        currentSplitPath << part;
        qDebug() << "getValue: currentSplitPath:" << currentSplitPath << "type:" << obj->type();

        // make sure next level is an object
        if (!obj->isObject()) {
            qDebug() << "getValue() failed to traverse path:" << currentSplitPath;
            return QMPointer<QMJsonValue>();
        }

        if (!obj->toObject()->contains(part)) {
            qDebug() << "getValue() failed to traverse next path:" << currentSplitPath;
            return QMPointer<QMJsonValue>();
        }

        obj = obj->toObject()->value(part);
    }

    qDebug() << "getValue: returning object";
    return obj;
}

void DBTree::setValue(const QStringList &splitPath, QMPointer<QMJsonValue> value, bool skipFlush)
{
    QMPointer<QMJsonValue> obj = dbRoot;
    auto db = lookupDb(splitPath);

    // if it is top of tree, ignore
    if (splitPath.length() == 0) {
        qWarning("setValue: ignoring attempt to write to root");
        return;
    }

    // split key from parent path values
    QStringList parentList(splitPath);
    QString key = parentList.takeLast();

    // make tree as required
    QStringList currentSplitPath;
    auto currentDb = mainDb;
    for (const QString &part : parentList) {
        currentSplitPath << part;
        qDebug() << "setValue: currentSplitPath:" << currentSplitPath << "type:" << obj->type();

        if (!obj->toObject()->contains(part)) {
            qDebug() << "setValue() inserting empty object";
            currentDb->acquireWriteLock();
            obj->toObject()->insert(part, QMPointer<QMJsonObject>(new QMJsonObject()));
            currentDb->releaseWriteLock();
        }

        qDebug() << "setValue() getting value";
        obj = obj->toObject()->value(part);

        // make sure next level is an object
        if (!obj->isObject()) {
            qDebug() << "setValue() failed to traverse path:" << currentSplitPath << "type:" << obj->type();
            return;
        }

        currentDb = lookupDb(currentSplitPath);
    }

    qDebug() << "setValue() inserting value:" << value;

    currentDb->acquireWriteLock();
    obj->toObject()->insert(key, value);
    currentDb->releaseWriteLock();

    // notify db to flush
    if (!skipFlush) {
        db->queueFlush();
    }
    return;
}

void DBTree::rmValue(const QStringList &splitPath)
{
    QMPointer<QMJsonValue> obj = dbRoot;
    auto db = lookupDb(splitPath);

    // if it is top of tree, ignore
    if (splitPath.length() == 0) {
        qWarning("rmValue: ignoring attempt to write to root");
        return;
    }

    // split key from parent path values
    QStringList parentList(splitPath);
    QString key = parentList.takeLast();

    // iterate through parent objects
    QStringList currentSplitPath;
    for (const QString &part : parentList) {
        currentSplitPath << part;
        qDebug() << "rmValue: currentSplitPath:" << currentSplitPath << "type:" << obj->type();

        // bail if parent doesn't exist
        if (!obj->toObject()->contains(part)) {
            return;
        }

        obj = obj->toObject()->value(part);

        // bail if next level is not an object
        if (!obj->isObject()) {
            qDebug() << "rmValue() failed to traverse path:" << currentSplitPath << "type:" << obj->type();
            return;
        }
    }

    db->acquireWriteLock();
    obj->toObject()->remove(key);
    db->releaseWriteLock();

    // notify db to flush
    db->queueFlush();
}

void DBTree::mergeValue(const QStringList &splitPath, QMPointer<QMJsonValue> value)
{
    QMPointer<QMJsonValue> obj = dbRoot;
    auto db = lookupDb(splitPath);

    // if it is top of tree, ignore
    if (splitPath.length() == 0) {
        qWarning("setValue: ignoring attempt to write to root");
        return;
    }

    QStringList parentList(splitPath);

    // iterate through parent objects
    QStringList currentSplitPath;
    for (const QString &part : parentList) {
        currentSplitPath << part;
        qDebug() << "mergeValue: currentSplitPath:" << currentSplitPath << "type:" << obj->type();

        // create missing children nodes
        if (!obj->toObject()->contains(part)) {
            obj->toObject()->insert(part, QMPointer<QMJsonObject>(new QMJsonObject()));
        }

        obj = obj->toObject()->value(part);

        // bail if next level is not an object
        if (!obj->isObject()) {
            qDebug() << "mergeValue() failed to traverse path:" << currentSplitPath << "type:" << obj->type();
            return;
        }
    }

    qDebug() << "mergeValue(): attempting merge obj:" << obj << "value:" << value->toObject();

    db->acquireWriteLock();
    obj->toObject()->unite(value->toObject(), QMJsonReplacementPolicy_Replace, QMJsonArrayUnitePolicy_Append);
    db->releaseWriteLock();

    // notify db to flush
    db->queueFlush();
}

void DBTree::exitCleanup()
{
    qDebug() << "exiting...";

    mainDb->forcePendingFlush();

    for (auto db : vmsDbs) {
        db->forcePendingFlush();
    }

    for (auto db : domstoreDbs) {
        db->forcePendingFlush();
    }

    qDebug() << "buh bye";
}

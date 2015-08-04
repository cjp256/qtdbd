#include "dbtree.h"
#include "simplejsondb.h"
#include <QDir>
#include <QDebug>
#include <QVariant>
#include <QHash>

DBTree::DBTree() : dbRoot(), dbPath(""), maxFlushDelay(0), mainDb(&dbRoot, "")
{
    qDebug() << "DBTree init()";
    mainDb.filterVmAndDomstoreKeys = true;
}

DBTree::DBTree(QString dbPath, int maxFlushDelayMillis) : dbRoot(), dbPath(dbPath), maxFlushDelay(maxFlushDelayMillis), mainDb(&dbRoot, QDir(dbPath).filePath("db"), "", maxFlushDelay)
{
    qDebug() << "DBTree init(" << dbPath << "," << maxFlushDelay << ")";
    mainDb.filterVmAndDomstoreKeys = false;
    qDebug() <<  valueToJsonString(&dbRoot);
}

DBTree::~DBTree()
{

}

QString DBTree::valueToJsonString(Value *d)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    Document e;
    e.Parse("{}");
    e.CopyFrom(*d, e.GetAllocator());
    e.Accept(writer);
    return buffer.GetString();
}

Value* DBTree::getObject(const QStringList &splitPath)
{
    Value *obj = &dbRoot;

    qDebug() << "getObject(): db = " << valueToJsonString(obj);

    // if it is top of tree, return the whole tree
    if (splitPath.length() == 0) {
        return obj;
    }

    // traverse tree parts
    foreach (const QString &part, splitPath) {
        qDebug() << "getObject: part:" << part << "obj:" << obj << "type:" << obj->GetType();

        // make sure next level is an object
        if (obj->IsObject()) {
            qDebug() << "getObject() is object";

            QByteArray array = part.toLocal8Bit();
            Value::MemberIterator itr = obj->FindMember(array.data());

            // if it doesn't exist, return default value
            if (itr == obj->MemberEnd()) {
                return nullptr;
            }

            obj = &itr->value;
        } else {
            qDebug() << "getObject() failed to traverse path:" << obj;
            return nullptr;
        }

        qDebug() << "getObject: next object:" << obj;
    }

    qDebug() << "getObject: returning object:" << obj;
    return obj;
}

void DBTree::setObject(QStringList splitPath, Value &value)
{
    Value *obj = &dbRoot;

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

        Value::MemberIterator itr = obj->FindMember(StringRef(part.toLocal8Bit().data()));

        // if it doesn't exist, create
        if (itr == obj->MemberEnd()) {
            qDebug() << "setObject: creating empty map for key part:" << part;
            Value v(kObjectType);
            auto p = part.toLocal8Bit();
            const char *pp = p.data();
            obj->AddMember(StringRef(pp), v, dbRoot.GetAllocator());
        }

        // make sure next level is an object
        auto p = key.toLocal8Bit();
        const char *pp = p.data();
        itr = obj->FindMember(StringRef(pp));
        if (obj->IsObject()) {
            qDebug() << "setObject: traversing to next level";
            obj = &itr->value;
        } else {
            qFatal("setObject: failed to traverse tree");
        }
    }

    auto k = key.toLocal8Bit();
    const char *kk = k.data();
    obj->AddMember(StringRef(kk), value, dbRoot.GetAllocator());
    qDebug() << "setObject: set" << key << "to:" << valueToJsonString(&value);
    return;
}

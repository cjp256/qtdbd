#include "simplejsondb.h"
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QSaveFile>
#include <QtDebug>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;

SimpleJsonDB::SimpleJsonDB(Value *v, QString vpath) : db(v), vpath(vpath), fileLock()
{
    // testing constructor, no backing file
    filterVmAndDomstoreKeys = false;
    skipDisk = true;
    flushTimer = new QTimer(this);
    flushTimer->setSingleShot(true);
    path = QString(":memory");

    Document newDoc;
    newDoc.Parse("{}");
    db->Swap(newDoc);
}

SimpleJsonDB::SimpleJsonDB(Value *v, QString path, QString vpath, int maxFlushDelayMillis) : db(v), path(path), vpath(vpath), maxFlushDelay(maxFlushDelayMillis), fileLock()
{
    filterVmAndDomstoreKeys = false;
    flushTimer = new QTimer(this);
    flushTimer->setSingleShot(true);
    //connect(flushTimer, SIGNAL(dbChanged()), this, SLOT(writeToDisk()));
    readFromDisk();
    qDebug() << jsonString();
}

SimpleJsonDB::~SimpleJsonDB()
{
}

QString SimpleJsonDB::jsonString()
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    // build new json object and strip vm/dom-store keys as required
    Document d;
    d.Parse("{}");
    d.CopyFrom(*db, d.GetAllocator());
    if (filterVmAndDomstoreKeys) {
        while (d.RemoveMember("vm")) {}
        while (d.RemoveMember("dom-store")) {}
    }
    d.Accept(writer);
    return buffer.GetString();
}

void SimpleJsonDB::readFromDisk()
{
    fileLock.lock();

    // read file into string
    qDebug() << "opening file: " << path;
    QString val;
    QFile file(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    qDebug() << "closed file: " << path;

    QByteArray array = val.toLocal8Bit();
    Document newDoc;
    newDoc.Parse(array.data());
    db->Swap(newDoc);

    fileLock.unlock();
}

void SimpleJsonDB::writeToDisk()
{
    fileLock.lock();

    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);

    Document e;
    e.Parse("{}");
    e.CopyFrom(*db, e.GetAllocator());
    e.Accept(writer);

    if (db->Size() <= 0) {
        // db is empty, remove old db file (if it exists)
        QFile file(path);
        file.remove();
    } else {
        // save json file with atomic QSaveFile
        QSaveFile file(path);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream outStream(&file);
        outStream << buffer.GetString();
        file.commit();
    }

    fileLock.unlock();
}

void SimpleJsonDB::dbChanged()
{
    if (!flushTimer->isActive()) {
        flushTimer->start(maxFlushDelay);
    }
}

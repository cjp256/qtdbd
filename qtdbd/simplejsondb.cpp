#include "simplejsondb.h"
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QSaveFile>
#include <QtDebug>
#include <qmjson.h>

SimpleJsonDB::SimpleJsonDB(const QString vpath) : db(), vpath(vpath), fileLock()
{
    // testing constructor, no backing file
    filterVmAndDomstoreKeys = false;
    skipDisk = true;
    flushTimer = new QTimer(this);
    flushTimer->setSingleShot(true);
    path = QString(":memory");
    qDebug() << db;
}

SimpleJsonDB::SimpleJsonDB(const QString path, QString vpath, int maxFlushDelayMillis) : db(), path(path), vpath(vpath), maxFlushDelay(maxFlushDelayMillis), fileLock()
{
    filterVmAndDomstoreKeys = false;
    flushTimer = new QTimer(this);
    flushTimer->setSingleShot(true);
    //connect(flushTimer, SIGNAL(dbChanged()), this, SLOT(writeToDisk()));
    readFromDisk();
    qDebug() << db;
}

SimpleJsonDB::~SimpleJsonDB()
{
}

QString SimpleJsonDB::jsonString()
{
    return db->toJson();
}

QMPointer<QMJsonValue> SimpleJsonDB::readFromDisk()
{
    fileLock.lock();
    db = QMJsonValue::fromJsonFile(path);
    fileLock.unlock();

    return db;
}

void SimpleJsonDB::writeToDisk(const QString &jsonString)
{
    fileLock.lock();

    if (jsonString.size() <= 0) {
        // db is empty, remove old db file (if it exists)
        QFile file(path);
        file.remove();
    } else {
        // save json file with atomic QSaveFile
        QSaveFile file(path);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream outStream(&file);
        outStream << jsonString;
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

#include "simplejsondb.h"
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QSaveFile>
#include <QtDebug>
#include <qmjson.h>

SimpleJsonDB::SimpleJsonDB(const QString vpath, const QString path, int maxFlushDelayMillis) : db(), vpath(vpath), path(path), fileLock(), flushTimer(0), maxFlushDelay(maxFlushDelayMillis), skipDisk(false), filterVmAndDomstoreKeys(false)
{
    flushTimer.setSingleShot(true);
    qDebug() << db;
}

SimpleJsonDB::~SimpleJsonDB()
{
}

QString SimpleJsonDB::jsonString()
{
    return db->toJson();
}

void SimpleJsonDB::setFilterVmAndDomstoreKeys(bool filter)
{
    filterVmAndDomstoreKeys = filter;
}

void SimpleJsonDB::setMaxFlushDelay(int maxFlushDelayMillis)
{
    maxFlushDelay = maxFlushDelayMillis;
}

void SimpleJsonDB::setWorkerThread(QThread *workerThread)
{
    flushTimer.moveToThread(workerThread);
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
    if (!flushTimer.isActive()) {
        flushTimer.start(maxFlushDelay);
    }
}

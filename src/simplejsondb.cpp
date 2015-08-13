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
    QObject::connect(&flushTimer, &QTimer::timeout, this, &SimpleJsonDB::flush);
    db = readFromDisk();
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

    if (path != ":memory:") {
        db = QMJsonValue::fromJsonFile(path);
    }

    // if no json file exists, create empty object
    if (db.isNull()) {
        auto obj = QMPointer<QMJsonObject>(new QMJsonObject());
        db = QMPointer<QMJsonValue>(new QMJsonValue(obj));
    }

    fileLock.unlock();

    return db;
}

void SimpleJsonDB::flush()
{
    QString jsonString = db->toJson();

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

void SimpleJsonDB::queueFlush(bool delayed)
{
    if (!delayed) {
        return flush();
    }

    if (!flushTimer.isActive()) {
        flushTimer.start(maxFlushDelay);
    }
}

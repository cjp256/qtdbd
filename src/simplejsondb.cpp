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

QMPointer<QMJsonValue> SimpleJsonDB::getValue()
{
    return db;
}

void SimpleJsonDB::readFromDisk()
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
}

void SimpleJsonDB::flush()
{
    QString jsonString = db->toJson();

    // skip flush if delay is -1
    if (maxFlushDelay == -1) {
        qDebug() << "skipping flush for:" << jsonString;
        return;
    }

    fileLock.lock();

    if (jsonString.size() <= 0) {
        // db is empty, remove old db file (if it exists)
        QFile file(path);
        file.remove();
    } else {
        // save json file with atomic QSaveFile
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "unable to write to db=" << path;
        } else {
            QTextStream outStream(&file);
            outStream << jsonString;
            file.commit();
        }
    }

    fileLock.unlock();
}

void SimpleJsonDB::queueFlush()
{
    if (maxFlushDelay == 0) {
        return flush();
    }

    if (maxFlushDelay > 0 && !flushTimer.isActive()) {
        flushTimer.start(maxFlushDelay);
    }
}

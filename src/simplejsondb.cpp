 #include "simplejsondb.h"
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QSaveFile>
#include <QtDebug>
#include <qmjson.h>

SimpleJsonDB::SimpleJsonDB(const QString vpath, const QString path, int maxFlushDelayMillis) : db(), vpath(vpath), path(path), writeLock(), flushTimer(0), maxFlushDelay(maxFlushDelayMillis), skipDisk(false), filterVmAndDomstoreKeys(false)
{
    flushTimer.setSingleShot(true);
    QObject::connect(&flushTimer, &QTimer::timeout, this, &SimpleJsonDB::flush);
    readFromDisk();
    qDebug() << db;
}

SimpleJsonDB::~SimpleJsonDB()
{
}

void SimpleJsonDB::acquireWriteLock()
{
    writeLock.lock();
}

void SimpleJsonDB::releaseWriteLock()
{
    writeLock.unlock();
}

QString SimpleJsonDB::jsonString()
{
    acquireWriteLock();
    QString dbString = db->toJson();
    releaseWriteLock();

    return dbString;
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

QString SimpleJsonDB::getPath()
{
    return path;
}

void SimpleJsonDB::readFromDisk()
{
    acquireWriteLock();

    if (path != ":memory:") {
        db = QMJsonValue::fromJsonFile(path);
        qDebug() << "db from json file:" << db;
    }

    // if no json file exists, create empty object
    // XXX: fix desired https://github.com/QtMark/qmjson/issues/9
    if (db->isNull()) {
        qInfo() << "failed to read db:" << path << "malformed?";
        auto obj = QMPointer<QMJsonObject>(new QMJsonObject());
        db = QMPointer<QMJsonValue>(new QMJsonValue(obj));
    }

    qInfo() << "read db:" << path << "value:" << db;
    releaseWriteLock();
}

void SimpleJsonDB::flush()
{
    qDebug() << "flush for db:" << path;

    // skip flush if delay is -1
    if (maxFlushDelay == -1 || path == ":memory:") {
        qDebug() << "skipping flush for:" << jsonString();
        return;
    }

    QString dbString = jsonString();

    if (dbString.size() <= 0) {
        // db is empty, remove old db file (if it exists)
        QFile file(path);
        file.remove();
    } else {
        // save json file with atomic QSaveFile
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "unable to write to db:" << path;
        } else {
            QTextStream outStream(&file);
            outStream << dbString;
            file.commit();
        }
        qInfo() << "flushed db:" << path;
    }
}

void SimpleJsonDB::queueFlush()
{
    if (maxFlushDelay == 0) {
        qDebug() << "immediate flush for db:" << path;
        return flush();
    }

    if (maxFlushDelay > 0 && !flushTimer.isActive()) {
        qDebug() << "queue flush for db:" << path;
        flushTimer.start(maxFlushDelay);
    }
}

void SimpleJsonDB::forcePendingFlush()
{
    if (flushTimer.isActive()) {
        qDebug() << "force active pending flush for db:" << path;
        flushTimer.stop();
        flush();
    }
}

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

    if (filterVmAndDomstoreKeys) {
        auto filteredValue = QMJsonValue::fromJson(dbString);

        if (filteredValue.isNull()) {
            qFatal("unable to convert db string to qmjsonvalue!");
            exit(1);
        }

        if (!filteredValue->isObject()) {
            qFatal("db qmjsonvalue is not an object!");
            exit(1);
        }

        filteredValue->toObject()->remove("vm");
        filteredValue->toObject()->remove("dom-store");
        dbString = filteredValue->toJson();
    }

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
        QFile dbFile(path);

        if (dbFile.exists()) {
            db = QMJsonValue::fromJsonFile(path);
            qDebug() << "db from json file:" << db;

            // XXX: fix desired https://github.com/QtMark/qmjson/issues/9
            if (db.isNull() || !db->isObject()) {
                // save off bad db file for later review, but do not abort
                // this is preferable than failing to boot if dbd doesn't come up
                qCritical() << "failed to read db:" << path << "malformed?" << dbFile.readAll();
                qCritical() << "moving bad db from:" << path << "to:" << path + ".bad";
                dbFile.rename(path + ".bad");
            }
        }
    }

    // if no json file exists, create empty object
    if (db.isNull() || !db->isObject()) {
        qDebug() << "creating default empty object for db:" << path;
        auto obj = QMPointer<QMJsonObject>(new QMJsonObject());
        db = QMPointer<QMJsonValue>(new QMJsonValue(obj));
    }

#if QT_VERSION >= 0x050500
    qInfo() << "read db:" << path << "value:" << db;
#endif
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
#if QT_VERSION >= 0x050500
        qInfo() << "flushed db:" << path;
#endif
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

/**
 * Copyright (c) 2015 Assured Information Security, Inc. <pattersonc@ainfosec.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "simplejsondb.h"
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QSaveFile>
#include <QtDebug>
#include <qmjson.h>

SimpleJsonDB::SimpleJsonDB(const QString vpath, const QString path, int maxFlushDelayMillis) : db(), vpath(vpath), path(path), writeLock(), flushTimer(this), maxFlushDelay(maxFlushDelayMillis), skipDisk(false), filterVmAndDomstoreKeys(false)
{
    flushTimer.setSingleShot(true);

    // connect timer up to flush function
    if (!QObject::connect(&flushTimer, &QTimer::timeout, this, &SimpleJsonDB::flush))
    {
        qFatal("failed to connect timeout() to flush()");
    }

    // inter-thread signal so main thread can trigger the timer start
    if (!QObject::connect(this, &SimpleJsonDB::signalFlushTimer, this, &SimpleJsonDB::startFlushTimer))
    {
        qFatal("failed to connect signalFlushTimer() to startFlushTimer()");
    }

    // read in db from disk
    readFromDisk();

    qDebug() << db;
}

SimpleJsonDB::~SimpleJsonDB()
{
}

void SimpleJsonDB::startFlushTimer()
{
    qDebug() << "start flush timer thread:" << QThread::currentThread();
    flushTimer.start(maxFlushDelay);
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
    QString dbString = db->toJson(QMJsonFormat_Pretty, QMJsonSort_CaseSensitive);

    if (filterVmAndDomstoreKeys)
    {
        auto filteredValue = QMJsonValue::fromJson(dbString);

        if (filteredValue.isNull())
        {
            qFatal("unable to convert db string to qmjsonvalue!");
            exit(1);
        }

        if (!filteredValue->isObject())
        {
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

    if (path != ":memory:")
    {
        QFile dbFile(path);

        if (dbFile.exists())
        {
            db = QMJsonValue::fromJsonFile(path);
            qDebug() << "db from json file:" << db;

            // XXX: fix desired https://github.com/QtMark/qmjson/issues/9
            if (db.isNull() || !db->isObject())
            {
                // save off bad db file for later review, but do not abort
                // this is preferable than failing to boot if dbd doesn't come up
                qCritical() << "failed to read db:" << path << "malformed?" << dbFile.readAll();
                qCritical() << "moving bad db from:" << path << "to:" << path + ".bad";
                dbFile.rename(path + ".bad");
            }
        }
    }

    // if no json file exists, create empty object
    if (db.isNull() || !db->isObject())
    {
        qDebug() << "creating default empty object for db:" << path;
        auto obj = QMPointer<QMJsonObject>(new QMJsonObject());
        db = QMPointer<QMJsonValue>(new QMJsonValue(obj));
    }

    releaseWriteLock();
}

void SimpleJsonDB::flush()
{
    qDebug() << "flush for db:" << path << "thread:" << QThread::currentThread();

    // skip flush if delay is -1
    if (maxFlushDelay == -1 || path == ":memory:")
    {
        qDebug() << "skipping flush for:" << jsonString();
        return;
    }

    QString dbString = jsonString();

    if (dbString == "{}")
    {
        // db is empty, remove old db file (if it exists)
        QFile file(path);
        file.remove();
    }
    else
    {
        dbString += '\n';

        // save json file with atomic QSaveFile
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qWarning() << "unable to write to db:" << path;
        }
        else
        {
            QTextStream outStream(&file);
            outStream << dbString;
            file.commit();
        }
#if QT_VERSION >= 0x050500
        qInfo() << "flushed db:" << path;
#endif
    }

    emit flushed();
}

void SimpleJsonDB::queueFlush()
{
    if (maxFlushDelay == 0)
    {
        qDebug() << "immediate flush for db:" << path;
        return flush();
    }

    if (maxFlushDelay > 0 && !flushTimer.isActive())
    {
        qDebug() << "queue flush for db:" << path << "millis:" << maxFlushDelay;

        if (this->thread()->isFinished()) {
            qFatal("flush thread is finished??");
        }

        if (!this->thread()->isRunning()) {
            qFatal("flush thread is not running??");
        }

        qDebug() << "queue flush signal thread:" << QThread::currentThread();
        qDebug() << "queue flush slot thread:" << this->thread();

        emit signalFlushTimer();
    }
}

void SimpleJsonDB::forcePendingFlush()
{
    if (flushTimer.isActive())
    {
        qDebug() << "force active pending flush for db:" << path;
        flushTimer.stop();
        flush();
    }
}

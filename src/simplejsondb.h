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

#ifndef SIMPLEJSONDB_H
#define SIMPLEJSONDB_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QVariantMap>

#include <qmjson.h>

class SimpleJsonDB: public QObject
{
    Q_OBJECT

public:
    SimpleJsonDB(const QString vpath, const QString path, bool createEmpty, int maxFlushDelayMillis);
    ~SimpleJsonDB();

    void readFromDisk();
    void acquireWriteLock();
    void releaseWriteLock();
    void setFilterVmAndDomstoreKeys(bool filter);
    void setMaxFlushDelay(int maxFlushDelayMillis);

    QString jsonString();
    QMPointer<QMJsonValue> getValue();
    QString getPath();

public slots:
    void startFlushTimer();
    void queueFlush();
    void flush();
    void forcePendingFlush();

Q_SIGNALS:
    void signalFlushTimer();
    void flushed();

private:
    QMPointer<QMJsonValue> db;
    QString vpath;
    QString path;
    QMutex writeLock;
    QTimer flushTimer;
    int maxFlushDelay;
    bool skipDisk;
    bool filterVmAndDomstoreKeys;
};

#endif // SIMPLEJSONDB_H

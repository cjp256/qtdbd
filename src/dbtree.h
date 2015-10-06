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

#ifndef DBTREE_H
#define DBTREE_H

#include <QObject>
#include <QThread>
#include <QHash>
#include <QSharedPointer>
#include <qmjson.h>
#include "simplejsondb.h"

class DBTree : public QObject
{
    Q_OBJECT

public:
    DBTree(const QString dbPath = ":memory:", int maxFlushDelayMillis = 3000);
    ~DBTree();

    QSharedPointer<SimpleJsonDB> createChildDb(const QString parentPath, const QString topLevel, const QString secondLevel, QHash<QString, QSharedPointer<SimpleJsonDB>> &dbs);
    void loadChildren(const QString path, const QString key, QHash<QString, QSharedPointer<SimpleJsonDB>> &dbs);
    void loadTree();
    QSharedPointer<SimpleJsonDB> lookupDb(const QStringList &splitPath, bool createEmpty);
    QMPointer<QMJsonValue> getValue(const QStringList &splitPath);
    void setValue(const QStringList &splitPath, QMPointer<QMJsonValue> value, bool skipFlush = false);
    void mergeValue(const QStringList &splitPath, QMPointer<QMJsonValue> value);
    void rmValue(const QStringList &splitPath);

    QMPointer<QMJsonValue> dbRoot;

public slots:
    void exitCleanup();

private:
    QString dbPath;
    int maxFlushDelay;
    QSharedPointer<SimpleJsonDB> mainDb;
    QThread dbWriterThread;
    QHash<QString, QSharedPointer<SimpleJsonDB>> vmsDbs;
    QHash<QString, QSharedPointer<SimpleJsonDB>> domstoreDbs;
};

#endif // DBTREE_H

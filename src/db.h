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

#ifndef DB_H
#define DB_H

extern "C"
{
#include <stdlib.h>
#include <xs.h>
}

#include <QObject>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusMessage>
#include "dbtree.h"
#include <qmjson.h>

class Db: public QObject,
    protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.citrix.xenclient.db")

public:
    Db(DBTree *dbTree, bool lookupSenderId);
    ~Db();

    const QString getSenderId();
    int getSenderDomId();
    QString getUuidFromDomId(int domid);
    bool senderPathSplit(QString path, QStringList &splitPathOut);

public slots:
    QString dump(const QString &path);
    bool exists(const QString &path);
    void inject(const QString &path, const QString &value);
    QStringList list(const QString &path);
    QString read(const QString &path);
    QByteArray read_binary(const QString &path);
    void rm(const QString &path);
    void write(const QString &path, const QString &value);

private:
    DBTree *dbTree;
    bool lookupSenderId;
    struct xs_handle *xs;
};

#endif // DB_H

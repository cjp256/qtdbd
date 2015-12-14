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

#ifndef TESTDBD_H
#define TESTDBD_H

#include <QObject>

class TestDBD: public QObject
{
    Q_OBJECT

public:
    TestDBD();

    QStringList splitPath(QString path);
    bool copyDirectory(const QString &srcPath, const QString &dstPath);
    QString prepTestDB(const QString &dbPath);

private slots:
    void testDbTreeBasicGetSet();
    void testDbBasicReadWrite();
    void testDb1BasicReadWrite();
    void testDb1VariousTypesRead();
    void testDbBasicDump();
    void testDbBasicExists();
    void testDbBasicList();
    void testDbBasicRm();
    void testDbBasicInject();
    void testDb2Inject();
    void testDb2WriteVm();
    void testDb2WriteDomstore();
    void testDb2WriteDomstoreQueuedFlush();
    void testDb2WriteRmDomstore();
    void testDb3WriteBaseDb();
    void testDb3WriteDbQueuedFlush();
    void testDb2WriteRmThenInjectDomstore();
    void testDb3WriteDbWriteThenRm();
    void testDb3WriteDbWriteThenRmRepeatedly();
};

#endif // TESTDBD_H

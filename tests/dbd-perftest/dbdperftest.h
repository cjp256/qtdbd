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

#ifndef DBDPERFTEST_H
#define DBDPERFTEST_H

#include <QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QTimer>
#include <QThread>
#include <QElapsedTimer>
#include <comcitrixxenclientdbinterface.h>
#include <dbdperftestreader.h>
#include <dbdperftestwriter.h>

class DbdPerfTest: public QObject
{
    Q_OBJECT

public:
    DbdPerfTest(QCoreApplication *app);
    ~DbdPerfTest();

    void parseCommandLine();
    void startTests();

public slots:
    void printSummary();
    void exitCleanup();

private:
    QCoreApplication *app;
    double readInterval;
    double writeInterval;
    qulonglong readIterations;
    qulonglong numberVms;
    QTimer printTimer;
    QElapsedTimer elapsedTimer;
    QThread *readerThread;
    DbdPerfTestReader *reader;
    QThread *writerThread;
    DbdPerfTestWriter *writer;

signals:
};
#endif // DBDPERFTEST_H

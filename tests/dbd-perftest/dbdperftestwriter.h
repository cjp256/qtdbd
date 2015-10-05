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

#ifndef DBDPERFTESTWRITER_H
#define DBDPERFTESTWRITER_H

#include <QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QTimer>
#include <QThread>
#include <comcitrixxenclientdbinterface.h>

class DbdPerfTestWriter: public QObject
{
    Q_OBJECT

public:
    DbdPerfTestWriter();
    ~DbdPerfTestWriter();

    bool running;
    double writeInterval;
    QTimer writeTimer;
    QMutex updateLock;
    qulonglong writeSuccessCount;
    qulonglong writeErrorCount;
    ComCitrixXenclientDbInterface *dbClient;

public slots:
    void performWrite();
    void setup();
    void stop();

signals:
    void finished();
};
#endif // DBDPERFTESTWRITER_H

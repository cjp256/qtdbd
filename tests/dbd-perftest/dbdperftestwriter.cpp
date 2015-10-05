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

#include "dbdperftestwriter.h"

DbdPerfTestWriter::DbdPerfTestWriter() : running(false), writeInterval(1.0), writeTimer(this), updateLock(), writeSuccessCount(0), writeErrorCount(0), dbClient(NULL)
{
}

DbdPerfTestWriter::~DbdPerfTestWriter()
{
}

void DbdPerfTestWriter::setup()
{
    if (!QDBusConnection::systemBus().isConnected()) {
        qFatal("failed to connect to dbus");
        exit(1);
    }

    dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", QDBusConnection::systemBus(), this);

    writeTimer.setInterval(writeInterval);
    QObject::connect(&writeTimer, &QTimer::timeout, this, &DbdPerfTestWriter::performWrite, Qt::DirectConnection);
    writeTimer.start();
}

void DbdPerfTestWriter::stop()
{
    running = false;
    writeTimer.stop();
    emit finished();
}

void DbdPerfTestWriter::performWrite()
{
    auto reply = dbClient->write(QString("test"), QString("write"));

    // wait until the dbus reply is ready
    reply.waitForFinished();

    // if it's valid, print it
    if (!reply.isValid()) {
        updateLock.lock();
        writeErrorCount += 1;
        updateLock.unlock();
    } else {
        updateLock.lock();
        writeSuccessCount += 1;
        updateLock.unlock();
    }
}

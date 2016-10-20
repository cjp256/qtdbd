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

#include "dbdperftestreader.h"

DbdPerfTestReader::DbdPerfTestReader() : running(false), readInterval(1.0), readIterations(1000), readTimer(this), updateLock(), readSuccessCount(0), readErrorCount(0), dbClient(NULL)
{
}

DbdPerfTestReader::~DbdPerfTestReader()
{
}

void DbdPerfTestReader::setup()
{
    if (!QDBusConnection::systemBus().isConnected())
    {
        qCritical("failed to connect to dbus");
        exit(1);
    }

    dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", QDBusConnection::systemBus(), this);

    // write known test key to be read in later
    dbClient->write(QString("key"), QString("value"));

    readTimer.setInterval(readInterval);
    QObject::connect(&readTimer, &QTimer::timeout, this, &DbdPerfTestReader::performRead, Qt::DirectConnection);
    readTimer.start();
}

void DbdPerfTestReader::stop()
{
    running = false;
    readTimer.stop();
    emit finished();
}

void DbdPerfTestReader::performRead()
{
    auto reply = dbClient->read(QString("key"));

    // wait until the dbus reply is ready
    reply.waitForFinished();

    // if it's valid, print it
    if (!reply.isValid())
    {
        updateLock.lock();
        readErrorCount += 1;
        updateLock.unlock();
        return;
    }

    if (reply.value() == QString("value"))
    {
        updateLock.lock();
        readSuccessCount += 1;
        updateLock.unlock();
    }
    else
    {
        qDebug() << "reading bad (valid) value??" << reply.value();
        updateLock.lock();
        readErrorCount += 1;
        updateLock.unlock();
    }

    if (readIterations <= (readSuccessCount + readErrorCount))
    {
        // time to exit
        emit finished();
        QCoreApplication::quit();
    }
}

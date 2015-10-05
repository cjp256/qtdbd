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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QMessageLogger>
#include <QtGlobal>
#include <QTimer>
#include <QElapsedTimer>
#include "db.h"
#include "dbinterfaceadaptor.h"
#include "dbtree.h"
#include "dbdperftest.h"

void exitHandler(int signal)
{
    qDebug() << "signal to quit received:" << signal;
    QCoreApplication::quit();
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("dbd-perftest");
    QCoreApplication::setApplicationVersion("1.0");

    DbdPerfTest perfTest(&app);

    perfTest.parseCommandLine();

    // setup exit handler
    signal(SIGQUIT, exitHandler);
    signal(SIGINT, exitHandler);
    signal(SIGTERM, exitHandler);
    signal(SIGHUP, exitHandler);

    QObject::connect(&app, SIGNAL(aboutToQuit()), &perfTest, SLOT(exitCleanup()));
    perfTest.startTests();
    app.exec();
}

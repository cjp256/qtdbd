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
#include <QtCore/QCoreApplication>
#include <QtDBus/QtDBus>
#include <QtGlobal>
#include "db.h"
#include "dbinterfaceadaptor.h"
#include "dbtree.h"
#include "comcitrixxenclientdbinterface.h"
#include "dbdlogging.h"

typedef struct
{
    bool debuggingEnabled;
    QString key;
} CmdLineOptions;

static CmdLineOptions g_cmdLineOptions;

void parseCommandLine(QCoreApplication &app, CmdLineOptions *opts)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("remove db key at specified path");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("key", QCoreApplication::translate("main", "key"));

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                   QCoreApplication::translate("main", "enable debug/verbose logging"));
    parser.addOption(debugOption);

    parser.process(app);

    opts->debuggingEnabled = parser.isSet(debugOption);

    const QStringList posArgs = parser.positionalArguments();
    if (posArgs.size() < 1)
    {
        qCritical("invalid arguments");
        exit(1);
    }

    opts->key = posArgs.at(0);

    DbdLogging::logger()->debugMode =  opts->debuggingEnabled;

    qDebug() << "debugging enabled:" << opts->debuggingEnabled;
    qDebug() << "key:" << opts->key;
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(DbdLogging::logOutput);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("db-rm");
    QCoreApplication::setApplicationVersion("3.0");

    parseCommandLine(app, &g_cmdLineOptions);

    if (!QDBusConnection::systemBus().isConnected())
    {
        qCritical("failed to connect to dbus");
        exit(1);
    }

    auto dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", QDBusConnection::systemBus(), &app);
    auto reply = dbClient->rm(g_cmdLineOptions.key);

    // wait until the dbus reply is ready
    reply.waitForFinished();

    // if it's valid, print it
    if (!reply.isValid())
    {
        qCritical("dbus not responding!");
        exit(1);
    }

    return 0;
}

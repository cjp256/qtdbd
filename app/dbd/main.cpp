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
    bool backgroundEnabled;
    bool skipDomidLookupEnabled;
    bool consoleLoggingEnabled;
    bool sessionBusEnabled;
    int dbMaxDelayMillis;
    QString dbBaseDirectoryPath;
} CmdLineOptions;

static CmdLineOptions g_cmdLineOptions;

void exitHandler(int signal)
{
    qDebug() << "signal to quit received:" << signal;
    QCoreApplication::quit();
}

void parseCommandLine(QCommandLineParser &parser, QCoreApplication &app, CmdLineOptions *opts)
{
    parser.setApplicationDescription("openxt simple db storage daemon");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                   QCoreApplication::translate("main", "enable debug/verbose logging"));
    parser.addOption(debugOption);

    QCommandLineOption backgroundOption(QStringList() << "b" << "background",
                                        QCoreApplication::translate("main", "run in background - fork"));
    parser.addOption(backgroundOption);

    QCommandLineOption skipLookupDomIDOption(QStringList() << "s" << "skip-domid-lookup",
            QCoreApplication::translate("main", "skip looking up sender domid using openxt specific call"));
    parser.addOption(skipLookupDomIDOption);

    QCommandLineOption consoleLogOption(QStringList() << "c" << "console-logging",
                                        QCoreApplication::translate("main", "use console logging instead of syslog"));
    parser.addOption(consoleLogOption);

    QCommandLineOption sessionBusOption(QStringList() << "x" << "use-session-bus",
                                        QCoreApplication::translate("main", "use session bus instead of system bus (useful for testing)"));
    parser.addOption(sessionBusOption);

    QCommandLineOption maxDbFlushTimeOption(QStringList() << "t" << "db-max-flush-delay",
                                            QCoreApplication::translate("main", "set maximum db flush delay to <milliseconds>."),
                                            QCoreApplication::translate("main", "milliseconds"));
    parser.addOption(maxDbFlushTimeOption);

    QCommandLineOption dbDirectoryOption(QStringList() << "w" << "db-base-directory",
                                         QCoreApplication::translate("main", "set base db path to <directory>"),
                                         QCoreApplication::translate("main", "directory"));
    parser.addOption(dbDirectoryOption);

    parser.process(app);

    opts->debuggingEnabled = parser.isSet(debugOption);
    opts->backgroundEnabled = parser.isSet(backgroundOption);
    opts->skipDomidLookupEnabled = parser.isSet(skipLookupDomIDOption);
    opts->consoleLoggingEnabled = parser.isSet(consoleLogOption);
    opts->sessionBusEnabled = parser.isSet(sessionBusOption);

    if (parser.isSet(maxDbFlushTimeOption))
    {
        opts->dbMaxDelayMillis = parser.value(maxDbFlushTimeOption).toDouble();
    }
    else
    {
        opts->dbMaxDelayMillis = 3000;
    }

    if (parser.isSet(dbDirectoryOption))
    {
        opts->dbBaseDirectoryPath = parser.value(dbDirectoryOption);
    }
    else
    {
        opts->dbBaseDirectoryPath = QString("/config");
    }

    DbdLogging::logger()->syslogMode =  !opts->consoleLoggingEnabled;
    DbdLogging::logger()->debugMode =  opts->debuggingEnabled;

    qDebug() << "debugging enabled:" << opts->debuggingEnabled;
    qDebug() << "background enabled:" << opts->backgroundEnabled;
    qDebug() << "skip domid lookup enabled:" << opts->skipDomidLookupEnabled;
    qDebug() << "console logging enabled:" << opts->consoleLoggingEnabled;
    qDebug() << "session bus enabled:" << opts->sessionBusEnabled;
    qDebug() << "max delay millis:" << opts->dbMaxDelayMillis;
    qDebug() << "db base directory path:" << opts->dbBaseDirectoryPath;
}

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    qInstallMessageHandler(DbdLogging::logOutput);

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("dbd");
    QCoreApplication::setApplicationVersion("3.0");
    QCommandLineParser parser;
    QDBusConnection bus = QDBusConnection::sessionBus();

    // setup exit handler
    signal(SIGQUIT, exitHandler);
    signal(SIGINT, exitHandler);
    signal(SIGTERM, exitHandler);
    signal(SIGHUP, exitHandler);

    parseCommandLine(parser, app, &g_cmdLineOptions);

    if (g_cmdLineOptions.backgroundEnabled)
    {
        daemon(1, 0);
    }

    if (!g_cmdLineOptions.sessionBusEnabled)
    {
        bus = QDBusConnection::systemBus();
    }

    if (!bus.isConnected())
    {
        qCritical("failed to connect to dbus");
        exit(1);
    }

    if (!bus.registerService("com.citrix.xenclient.db"))
    {
        qCritical("failed to register service");
        exit(2);
    }

    DBTree *dbTree = new DBTree(g_cmdLineOptions.dbBaseDirectoryPath, g_cmdLineOptions.dbMaxDelayMillis);
    Db *db = new Db(dbTree, !g_cmdLineOptions.skipDomidLookupEnabled);
    new DbInterfaceAdaptor(db);

    bus.registerObject("/", db, QDBusConnection::ExportAllSlots);

    qDebug() << "registered and listening on dbus...";

    qDebug() << dbTree->dbRoot->toJson();

    QObject::connect(&app, SIGNAL(aboutToQuit()), dbTree, SLOT(exitCleanup()));
    app.exec();
}

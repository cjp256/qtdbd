#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QMessageLogger>
#include <QtGlobal>
#include <stdio.h>
#include <stdlib.h>
#include "db.h"
#include "db_adaptor.h"
#include "dbtree.h"

typedef struct
{
    bool debuggingEnabled;
    bool domidLookupEnabled;
    bool syslogEnabled;
    bool sessionBusEnabled;
    int dbMaxDelayMillis;
    QString dbBaseDirectoryPath;
} CmdLineOptions;

static CmdLineOptions g_cmdLineOptions;
static  DBTree *dbTree = nullptr;

void logOutput(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    /* TODO: honor syslog option */
    switch (type) {
    case QtDebugMsg:
        if (g_cmdLineOptions.debuggingEnabled) {
            fprintf(stderr, "Debug: %s\n", qPrintable(msg));
        }
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s\n", qPrintable(msg));
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s\n", qPrintable(msg));
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s\n", qPrintable(msg));
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s\n", qPrintable(msg));
        abort();
    }
}

void parseCommandLine(QCommandLineParser &parser, QCoreApplication &app, CmdLineOptions *opts)
{
    // set defaults
    opts->debuggingEnabled = false;
    opts->domidLookupEnabled = false;
    opts->syslogEnabled = false;
    opts->sessionBusEnabled = false;
    opts->dbMaxDelayMillis = 3000;
    opts->dbBaseDirectoryPath = QString("/config");

    parser.setApplicationDescription("openxt simple db storage daemon");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
            QCoreApplication::translate("main", "enable debug/verbose logging."));
    parser.addOption(debugOption);

    QCommandLineOption lookupDomIDOption(QStringList() << "l" << "lookup-domid",
            QCoreApplication::translate("main", "enable looking up sender domid using openxt specific call"));
    parser.addOption(lookupDomIDOption);

    QCommandLineOption syslogOption(QStringList() << "s" << "syslog",
            QCoreApplication::translate("main", "enable logging via syslog"));
    parser.addOption(syslogOption);

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
    opts->domidLookupEnabled = parser.isSet(lookupDomIDOption);
    opts->syslogEnabled = parser.isSet(syslogOption);
    opts->sessionBusEnabled = parser.isSet(sessionBusOption);

    if (parser.isSet(maxDbFlushTimeOption)) {
        opts->dbMaxDelayMillis = parser.value(maxDbFlushTimeOption).toDouble();
    }

    if (parser.isSet(dbDirectoryOption)) {
        opts->dbBaseDirectoryPath = parser.value(dbDirectoryOption);
    }

    qDebug("debugging enabled: %d", opts->debuggingEnabled);
    qDebug("domid lookup enabled: %d", opts->domidLookupEnabled);
    qDebug("syslog enabled: %d", opts->syslogEnabled);
    qDebug("session bus enabled: %d", opts->sessionBusEnabled);
    qDebug("max delay millis: %d", opts->dbMaxDelayMillis);
    qDebug("db base directory path: %s", qPrintable(opts->dbBaseDirectoryPath));
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logOutput);
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("dbd");
    QCoreApplication::setApplicationVersion("3.0");
    QCommandLineParser parser;

    parseCommandLine(parser, app, &g_cmdLineOptions);

    if (!QDBusConnection::sessionBus().isConnected()) {
        qFatal("failed to connect to dbus");
        exit(1);
    }

    if (!QDBusConnection::sessionBus().registerService("com.citrix.xenclient.db")) {
        qFatal("failed to register service");
        exit(2);
    }

    dbTree = new DBTree(g_cmdLineOptions.dbBaseDirectoryPath, g_cmdLineOptions.dbMaxDelayMillis);
    Db *db = new Db(dbTree, g_cmdLineOptions.domidLookupEnabled);
    new DbInterfaceAdaptor(db);

    QDBusConnection::sessionBus().registerObject("/", "com.citrix.xenclient.db", db, QDBusConnection::ExportAllSlots);

    qDebug("registered and listening on dbus...");
    app.exec();
}

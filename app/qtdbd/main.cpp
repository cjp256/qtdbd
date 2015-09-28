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
#include <unistd.h>
#include <syslog.h>

typedef struct
{
    bool debuggingEnabled;
    bool foregroundEnabled;
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
    switch (type) {
    case QtDebugMsg:
        if (g_cmdLineOptions.debuggingEnabled) {
            if (g_cmdLineOptions.syslogEnabled) {
                syslog(LOG_DEBUG, "[DEBUG] %s\n", qPrintable(msg));
            } else {
                fprintf(stderr, "[DEBUG] %s\n", qPrintable(msg));
            }
        }
        break;
#if QT_VERSION >= 0x050500
    case QtInfoMsg:
        if (g_cmdLineOptions.syslogEnabled) {
            syslog(LOG_INFO, "[INFO] %s\n", qPrintable(msg));
        } else {
            fprintf(stderr, "[INFO] %s\n", qPrintable(msg));
        }
        break;
#endif
    case QtWarningMsg:
        if (g_cmdLineOptions.syslogEnabled) {
            syslog(LOG_WARNING, "[WARNING] %s\n", qPrintable(msg));
        } else {
            fprintf(stderr, "[WARNING] %s\n", qPrintable(msg));
        }
        break;
    case QtCriticalMsg:
        if (g_cmdLineOptions.syslogEnabled) {
            syslog(LOG_CRIT, "[CRITICAL] %s\n", qPrintable(msg));
        } else {
            fprintf(stderr, "[CRITICAL] %s\n", qPrintable(msg));
        }
        break;
    case QtFatalMsg:
        if (g_cmdLineOptions.syslogEnabled) {
            syslog(LOG_ALERT, "[FATAL] %s\n", qPrintable(msg));
        } else {
            fprintf(stderr, "[FATAL] %s\n", qPrintable(msg));
        }
        abort();
    }
}

void parseCommandLine(QCommandLineParser &parser, QCoreApplication &app, CmdLineOptions *opts)
{
    // set defaults
    opts->debuggingEnabled = false;
    opts->foregroundEnabled = false;
    opts->domidLookupEnabled = false;
    opts->syslogEnabled = false;
    opts->sessionBusEnabled = false;
    opts->dbMaxDelayMillis = 3000;
    opts->dbBaseDirectoryPath = QString("/config");

    parser.setApplicationDescription("openxt simple db storage daemon");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
            QCoreApplication::translate("main", "enable debug/verbose logging"));
    parser.addOption(debugOption);

    QCommandLineOption foregroundOption(QStringList() << "f" << "foreground",
            QCoreApplication::translate("main", "run in foreground - do not fork"));
    parser.addOption(foregroundOption);

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
    opts->foregroundEnabled = parser.isSet(foregroundOption);
    opts->domidLookupEnabled = parser.isSet(lookupDomIDOption);
    opts->syslogEnabled = parser.isSet(syslogOption);
    opts->sessionBusEnabled = parser.isSet(sessionBusOption);

    if (parser.isSet(maxDbFlushTimeOption)) {
        opts->dbMaxDelayMillis = parser.value(maxDbFlushTimeOption).toDouble();
    }

    if (parser.isSet(dbDirectoryOption)) {
        opts->dbBaseDirectoryPath = parser.value(dbDirectoryOption);
    }

    qDebug() << "debugging enabled:" << opts->debuggingEnabled;
    qDebug() << "foreground enabled:" << opts->foregroundEnabled;
    qDebug() << "domid lookup enabled:" << opts->domidLookupEnabled;
    qDebug() << "syslog enabled:" << opts->syslogEnabled;
    qDebug() << "session bus enabled:" << opts->sessionBusEnabled;
    qDebug() << "max delay millis:" << opts->dbMaxDelayMillis;
    qDebug() << "db base directory path:" << opts->dbBaseDirectoryPath;
}

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logOutput);
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("dbd");
    QCoreApplication::setApplicationVersion("3.0");
    QCommandLineParser parser;
    QDBusConnection bus = QDBusConnection::sessionBus();

    parseCommandLine(parser, app, &g_cmdLineOptions);

    if (g_cmdLineOptions.syslogEnabled) {
        openlog("dbd", LOG_PID, LOG_DAEMON);
    }

    if (!g_cmdLineOptions.sessionBusEnabled) {
        bus = QDBusConnection::systemBus();
    }

    if (!bus.isConnected()) {
        qFatal("failed to connect to dbus");
        exit(1);
    }

    if (!bus.registerService("com.citrix.xenclient.db")) {
        qFatal("failed to register service");
        exit(2);
    }

    dbTree = new DBTree(g_cmdLineOptions.dbBaseDirectoryPath, g_cmdLineOptions.dbMaxDelayMillis);
    Db *db = new Db(dbTree, g_cmdLineOptions.domidLookupEnabled);
    new DbInterfaceAdaptor(db);

    bus.registerObject("/", db, QDBusConnection::ExportAllSlots);

    qDebug() << "registered and listening on dbus...";

    if (!g_cmdLineOptions.foregroundEnabled) {
        daemon(1, 0);
    }

    qDebug() << dbTree->dbRoot->toJson();
    app.exec();
}
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QtGlobal>
#include <stdio.h>
#include <stdlib.h>
#include "db.h"
#include "db_adaptor.h"

static bool debuggingEnabled = false;
static bool domidLookupEnabled = false;
static bool syslogEnabled = false;

void logOutput(QtMsgType type, const QMessageLogContext&, const QString& msg)
 {
    /* TODO: honor syslog option */
    switch (type) {
    case QtDebugMsg:
        if (debuggingEnabled) {
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

int main(int argc, char *argv[])
{
    qInstallMessageHandler(logOutput);
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("dbd");
    QCoreApplication::setApplicationVersion("3.0");

    QCommandLineParser parser;
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

    QCommandLineOption maxDbFlushTimeOption(QStringList() << "t" << "max-db-flush-delay",
            QCoreApplication::translate("main", "set maximum db flush delay to <milliseconds>."),
            QCoreApplication::translate("main", "milliseconds"));
    parser.addOption(maxDbFlushTimeOption);

    parser.process(app);

    debuggingEnabled = parser.isSet(debugOption);
    domidLookupEnabled = parser.isSet(lookupDomIDOption);
    syslogEnabled = parser.isSet(syslogOption);

    if (!QDBusConnection::sessionBus().isConnected()) {
        fprintf(stderr, "Cannot connect to the D-Bus system bus.\n");
        return 1;
    }

    if (!QDBusConnection::sessionBus().registerService("com.citrix.xenclient.db")) {
        fprintf(stderr, "%s\n",
                qPrintable(QDBusConnection::sessionBus().lastError().message()));
        exit(1);
    }

    Db *db = new Db();
    new DbInterfaceAdaptor(db);

    QDBusConnection::sessionBus().registerObject("/", "com.citrix.xenclient.db", db, QDBusConnection::ExportAllSlots);

    qDebug("registered...\n");
    app.exec();
}

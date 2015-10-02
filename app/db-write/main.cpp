#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QMessageLogger>
#include <QtGlobal>
#include <stdio.h>
#include <stdlib.h>
#include "db.h"
#include "dbinterfaceadaptor.h"
#include "dbtree.h"
#include <unistd.h>
#include <syslog.h>
#include <comcitrixxenclientdbinterface.h>

typedef struct
{
    bool debuggingEnabled;
    QString key;
    QString value;
} CmdLineOptions;

static CmdLineOptions g_cmdLineOptions;

void parseCommandLine(QCoreApplication &app, CmdLineOptions *opts)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("write simple string value at specified key path");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("key", QCoreApplication::translate("main", "key"));
    parser.addPositionalArgument("value", QCoreApplication::translate("main", "value"));

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                   QCoreApplication::translate("main", "enable debug/verbose logging"));
    parser.addOption(debugOption);

    parser.process(app);

    opts->debuggingEnabled = parser.isSet(debugOption);

    const QStringList posArgs = parser.positionalArguments();

    if (posArgs.size() < 2) {
        qWarning() << "invalid arguments";
        exit(1);
    }

    opts->key = posArgs.at(0);
    opts->value = posArgs.at(1);

    qDebug() << "debugging enabled:" << opts->debuggingEnabled;
    qDebug() << "key:" << opts->key;
    qDebug() << "value:" << opts->value;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("db-write");
    QCoreApplication::setApplicationVersion("3.0");

    parseCommandLine(app, &g_cmdLineOptions);

    if (!QDBusConnection::systemBus().isConnected()) {
        qFatal("failed to connect to dbus");
        exit(1);
    }

    auto dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", QDBusConnection::systemBus(), &app);
    auto reply = dbClient->write(g_cmdLineOptions.key, g_cmdLineOptions.value);

    // wait until the dbus reply is ready
    reply.waitForFinished();

    // if it's valid, print it
    if (!reply.isValid()) {
        qWarning() << "dbus not responding!";
        exit(1);
    }

    return 0;
}

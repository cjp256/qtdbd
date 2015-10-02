#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <QtCore/QCoreApplication>
#include <QtDBus/QtDBus>
#include <QtGlobal>
#include "db.h"
#include "dbinterfaceadaptor.h"
#include "dbtree.h"
#include <comcitrixxenclientdbinterface.h>

typedef struct
{
    bool debuggingEnabled;
    QString key;
} CmdLineOptions;

static CmdLineOptions g_cmdLineOptions;

void parseCommandLine(QCoreApplication &app, CmdLineOptions *opts)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("check if db key exists");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("key", QCoreApplication::translate("main", "key"));

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                   QCoreApplication::translate("main", "enable debug/verbose logging"));
    parser.addOption(debugOption);

    parser.process(app);

    opts->debuggingEnabled = parser.isSet(debugOption);

    const QStringList posArgs = parser.positionalArguments();
    if (posArgs.size() < 1) {
        qWarning() << "invalid arguments";
        exit(1);
    }

    opts->key = posArgs.at(0);

    qDebug() << "debugging enabled:" << opts->debuggingEnabled;
    qDebug() << "key:" << opts->key;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("db-exists");
    QCoreApplication::setApplicationVersion("3.0");

    parseCommandLine(app, &g_cmdLineOptions);

    if (!QDBusConnection::systemBus().isConnected()) {
        qFatal("failed to connect to dbus");
        exit(1);
    }

    auto dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", QDBusConnection::systemBus(), &app);
    auto reply = dbClient->exists(g_cmdLineOptions.key);

    // wait until the dbus reply is ready
    reply.waitForFinished();

    // if it's valid, print it
    if (!reply.isValid()) {
        qWarning() << "dbus not responding!";
        exit(1);
    }

    if (reply.value()) {
        QTextStream(stdout) << "true" << endl;
    } else {
        QTextStream(stdout) << "false" << endl;
    }

    return 0;
}

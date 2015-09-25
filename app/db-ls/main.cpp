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
#include <db_proxy.h>

typedef struct
{
    bool debuggingEnabled;
    bool syslogEnabled;
    bool sessionBusEnabled;
    QString key;
} CmdLineOptions;

static CmdLineOptions g_cmdLineOptions;

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
    opts->syslogEnabled = false;
    opts->sessionBusEnabled = false;

    parser.setApplicationDescription("openxt simple db storage daemon");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("key", QCoreApplication::translate("main", "key"));

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
            QCoreApplication::translate("main", "enable debug/verbose logging"));
    parser.addOption(debugOption);

    QCommandLineOption syslogOption(QStringList() << "s" << "syslog",
            QCoreApplication::translate("main", "enable logging via syslog"));
    parser.addOption(syslogOption);

    QCommandLineOption sessionBusOption(QStringList() << "x" << "use-session-bus",
            QCoreApplication::translate("main", "use session bus instead of system bus (useful for testing)"));
    parser.addOption(sessionBusOption);

    parser.process(app);

    opts->debuggingEnabled = parser.isSet(debugOption);
    opts->syslogEnabled = parser.isSet(syslogOption);
    opts->sessionBusEnabled = parser.isSet(sessionBusOption);

    const QStringList posArgs = parser.positionalArguments();
    if (posArgs.size() < 1) {
        qWarning() << "invalid arguments";
        exit(1);
    }

    opts->key = posArgs.at(0);

    qDebug() << "debugging enabled:" << opts->debuggingEnabled;
    qDebug() << "syslog enabled:" << opts->syslogEnabled;
    qDebug() << "session bus enabled:" << opts->sessionBusEnabled;
    qDebug() << "key:" << opts->key;
}

// Compatibility glue for insane dump (ala db-ls)
//void lsObject(QMPointer<QMJsonValue)
//def dump(obj, path):
//    key_name = ""
//    for elem in path.split("/"):
//        if len(elem) > 0:
//            key_name = elem
//            break

//    outstr = []
//    def __path_dump_level(outstr, key_name, obj, level):
//        # handle the invalid key case
//        if obj == 'null':
//            outstr.append("%s%s = \"\"" % (" " * level, key_name))
//            return

//        # simple key value
//        if isinstance(obj, basestring) or isinstance(obj, bool):
//            outstr.append("%s%s = \"%s\"" % (" " * level, key_name, str(obj)))
//            return

//        # iterate over dicts
//        if isinstance(obj, dict):
//            outstr.append("%s%s =" % (" " * level, key_name))
//            for k, v in obj.iteritems():
//                __path_dump_level(outstr, k, v, level + 1)

//    __path_dump_level(outstr, key_name, obj, 0)
//    return "\n".join(outstr)

//def main():
//    if len(sys.argv) < 2:
//        key = "/"
//    else:
//        key = sys.argv[1]

//    bus = dbus.SystemBus()
//    dbd = bus.get_object('com.citrix.xenclient.db', '/')
//    iface = dbus.Interface(dbd, 'com.citrix.xenclient.db')
//    data = iface.dump(key)
//    obj = json.loads(data)
//    out = dump(obj, key)
//    print("%s" % (out))



int main(int argc, char *argv[])
{
    qInstallMessageHandler(logOutput);
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("db-ls");
    QCoreApplication::setApplicationVersion("3.0");
    QCommandLineParser parser;
    QDBusConnection bus = QDBusConnection::sessionBus();

    parseCommandLine(parser, app, &g_cmdLineOptions);

    if (g_cmdLineOptions.syslogEnabled) {
        openlog("db-ls", LOG_PID, LOG_DAEMON);
    }

    if (!g_cmdLineOptions.sessionBusEnabled) {
        bus = QDBusConnection::systemBus();
    }

    if (!bus.isConnected()) {
        qFatal("failed to connect to dbus");
        exit(1);
    }

    auto dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", bus, &app);
    auto reply = dbClient->read(g_cmdLineOptions.key);

    // wait until the dbus reply is ready
    reply.waitForFinished();

    // if it's valid, print it
    if (!reply.isValid()) {
        qWarning() << "dbus not responding!";
        exit(1);
    }

    QTextStream(stdout) << reply.value() << endl;
    return 0;
}

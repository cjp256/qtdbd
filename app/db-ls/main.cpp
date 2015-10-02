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
#include <qmjson.h>

typedef struct
{
    bool debuggingEnabled;
    QString key;
} CmdLineOptions;

static CmdLineOptions g_cmdLineOptions;

void parseCommandLine(QCoreApplication &app, CmdLineOptions *opts)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("recursively list db tree at specified key value");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument("key", QCoreApplication::translate("main", "key"));

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                   QCoreApplication::translate("main", "enable debug/verbose logging"));
    parser.addOption(debugOption);

    parser.process(app);

    opts->debuggingEnabled = parser.isSet(debugOption);

    const QStringList posArgs = parser.positionalArguments();
    if (posArgs.size() >= 1) {
        opts->key = posArgs.at(0);
    } else {
        opts->key = QString("/");
    }

    qDebug() << "debugging enabled:" << opts->debuggingEnabled;
    qDebug() << "key:" << opts->key;
}


// Insane stringify (aka db-ls)
void lsObject(QMPointer<QMJsonValue> value, QStringList &outStringList, QString key, int level)
{
    QString out;

    if (value->isNull()) {
        out = QString(level, QChar(' ')) + key + " = null";
        outStringList.append(out);
        return;
    }

    if (value->isBool()) {
        //outstr.append("%s%s = \"%s\"" % (" " * level, key_name, str(obj)))
        if (value->toBool()) {
            out = QString(level, QChar(' ')) + key + " = \"true\"";
        } else {
            out = QString(level, QChar(' ')) + key + " = \"false\"";
        }
        outStringList.append(out);
        return;
    }

    if (value->isString()) {
        out = QString(level, QChar(' ')) + key + " = \"" + value->toString() + "\"";
        outStringList.append(out);
        return;
    }

    if (value->isDouble()) {
        out = QString(level, QChar(' ')) + key + " = \"" + QString::number(value->toDouble()) + "\"";
        outStringList.append(out);
        return;
    }

    if (value->isObject()) {
        out = QString(level, QChar(' ')) + key + " =";
        outStringList.append(out);
for (const auto &subKey : value->toObject()->keys()) {
            lsObject(value->toObject()->value(subKey), outStringList, subKey, level + 1);
        }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("db-ls");
    QCoreApplication::setApplicationVersion("3.0");

    parseCommandLine(app, &g_cmdLineOptions);

    if (!QDBusConnection::systemBus().isConnected()) {
        qFatal("failed to connect to dbus");
        exit(1);
    }

    auto dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", QDBusConnection::systemBus(), &app);
    auto reply = dbClient->dump(g_cmdLineOptions.key);

    // wait until the dbus reply is ready
    reply.waitForFinished();

    // if it's valid, print it
    if (!reply.isValid()) {
        qWarning() << "dbus not responding!";
        exit(1);
    }

    QMPointer<QMJsonValue> value = QMJsonValue::fromJson(reply.value());

    QStringList outStringList;
    QString key = QString("");
    QStringList splitPath = g_cmdLineOptions.key.split("/", QString::SplitBehavior::SkipEmptyParts);

    if (splitPath.length() > 0) {
        key = splitPath.last();
    }

    lsObject(value, outStringList, key, 0);

    QTextStream(stdout) << outStringList.join("\n") << endl;
    return 0;
}

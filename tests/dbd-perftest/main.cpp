#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QMessageLogger>
#include <QtGlobal>
#include <QTimer>
#include <QElapsedTimer>
#include <stdio.h>
#include <stdlib.h>
#include "db.h"
#include "dbinterfaceadaptor.h"
#include "dbtree.h"
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
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

    qDebug() << "parsing command line";
    perfTest.parseCommandLine();

    qDebug() << "parsed command line";

    // setup exit handler
    signal(SIGQUIT, exitHandler);
    signal(SIGINT, exitHandler);
    signal(SIGTERM, exitHandler);
    signal(SIGHUP, exitHandler);

    QObject::connect(&app, SIGNAL(aboutToQuit()), &perfTest, SLOT(exitCleanup()));
    perfTest.startTests();
    app.exec();
}

#include "dbdperftest.h"

DbdPerfTest::DbdPerfTest(QCoreApplication *app) : app(app), parser(), readInterval(10), writeInterval(1000), readIterations(100000), numberVms(25), printTimer(), elapsedTimer(), reader(), writer()
{
}

DbdPerfTest::~DbdPerfTest()
{

}

void DbdPerfTest::exitCleanup()
{
    qDebug() << "exiting...";
    printSummary();
}

void DbdPerfTest::printSummary()
{
    reader.updateLock->lock();
    auto readSuccess = reader.readSuccessCount;
    auto readError = reader.readErrorCount;
    reader.updateLock->unlock();

    writer.updateLock->lock();
    auto writeSuccess = writer.writeSuccessCount;
    auto writeError = writer.writeErrorCount;
    writer.updateLock->unlock();

    auto elapsed = elapsedTimer.elapsed();

    qDebug() << "read success count:" << readSuccess;
    qDebug() << "read error count:" << readError;
    qDebug() << "read total count:" << readSuccess + readError;
    qDebug() << "write success count:" << writeSuccess;
    qDebug() << "write error count:" << writeError;
    qDebug() << "write total count:" << writeSuccess + writeError;
    qDebug() << "elapsed time:" << elapsed << "ms";
    qDebug() << "read rate:" << (readSuccess) / (elapsedTimer.elapsed() / 1000.0) << "per second";
}

void DbdPerfTest::parseCommandLine()
{
    parser.setApplicationDescription("openxt simple db storage daemon");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption readIntervalOption(QStringList() << "r" << "read-interval",
            QCoreApplication::translate("main", "interval to set read timer to read"),
            QCoreApplication::translate("main", "milliseconds"));
    parser.addOption(readIntervalOption);

    QCommandLineOption writeIntervalOption(QStringList() << "w" << "write-interval",
            QCoreApplication::translate("main", "interval to set write timer to write"),
            QCoreApplication::translate("main", "milliseconds"));
    parser.addOption(writeIntervalOption);

    QCommandLineOption readIterationsOption(QStringList() << "i" << "read-iterations",
            QCoreApplication::translate("main", "number of read iterations to attempt"),
            QCoreApplication::translate("main", "iterations"));
    parser.addOption(readIterationsOption);

    QCommandLineOption numberVmsOption(QStringList() << "n" << "number-vms",
            QCoreApplication::translate("main", "number of vms to simulate"),
            QCoreApplication::translate("main", "vms"));
    parser.addOption(numberVmsOption);

    QCommandLineOption sessionBusOption(QStringList() << "x" << "use-session-bus",
            QCoreApplication::translate("main", "use session bus instead of system bus (useful for testing)"));
    parser.addOption(sessionBusOption);

    parser.process(*app);

    if (parser.isSet(readIntervalOption)) {
        readInterval = parser.value(readIntervalOption).toDouble();
    }

    if (parser.isSet(writeIntervalOption)) {
        writeInterval = parser.value(writeIntervalOption).toDouble();
    }

    if (parser.isSet(readIterationsOption)) {
        readIterations = parser.value(readIterationsOption).toULongLong();
    }

    if (parser.isSet(numberVmsOption)) {
        numberVms = parser.value(numberVmsOption).toULongLong();
    }

    qDebug() << "write interval:" << writeInterval;
    qDebug() << "read iterations:" << readIterations;
    qDebug() << "number vms:" << numberVms;
}

void DbdPerfTest::startTests()
{
    qDebug() << "starting tests...";

    QObject::connect(&printTimer, &QTimer::timeout, this, &DbdPerfTest::printSummary);
    printTimer.setInterval(1000);
    elapsedTimer.start();
    printTimer.start();

    reader.readInterval = readInterval;
    reader.readIterations = readIterations;
    writer.writeInterval = writeInterval;

    reader.moveToThread(&reader);
    reader.start();
    writer.moveToThread(&writer);
    writer.start();

    qDebug() << "tests started...";
}


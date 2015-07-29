#include "dbdperftestreader.h"

DbdPerfTestReader::DbdPerfTestReader() : running(false), readInterval(1.0), readIterations(1000), readTimer(this), updateLock(), readSuccessCount(0), readErrorCount(0), dbClient(NULL)
{
}

DbdPerfTestReader::~DbdPerfTestReader()
{
}

void DbdPerfTestReader::setup()
{
    if (!QDBusConnection::systemBus().isConnected()) {
        qFatal("failed to connect to dbus");
        exit(1);
    }

    dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", QDBusConnection::systemBus(), this);

    // write known test key to be read in later
    dbClient->write(QString("key"), QString("value"));

    readTimer.setInterval(readInterval);
    QObject::connect(&readTimer, &QTimer::timeout, this, &DbdPerfTestReader::performRead, Qt::DirectConnection);
    readTimer.start();
}

void DbdPerfTestReader::stop()
{
    running = false;
    readTimer.stop();
    emit finished();
}

void DbdPerfTestReader::performRead()
{
    auto reply = dbClient->read(QString("key"));

    // wait until the dbus reply is ready
    reply.waitForFinished();

    // if it's valid, print it
    if (!reply.isValid()) {
        updateLock.lock();
        readErrorCount += 1;
        updateLock.unlock();
        return;
    }

    if (reply.value() == QString("value")) {
        updateLock.lock();
        readSuccessCount += 1;
        updateLock.unlock();
    } else {
        qDebug() << "reading bad (valid) value??" << reply.value();
        updateLock.lock();
        readErrorCount += 1;
        updateLock.unlock();
    }

    if (readIterations <= (readSuccessCount + readErrorCount)) {
        // time to exit
        emit finished();
        QCoreApplication::quit();
    }
}

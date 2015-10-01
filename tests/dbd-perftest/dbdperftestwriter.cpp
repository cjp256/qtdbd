#include "dbdperftestwriter.h"

DbdPerfTestWriter::DbdPerfTestWriter() : running(false), writeInterval(1.0), writeTimer(this), updateLock(), writeSuccessCount(0), writeErrorCount(0), dbClient(NULL)
{
}

DbdPerfTestWriter::~DbdPerfTestWriter()
{
}

void DbdPerfTestWriter::setup()
{
    if (!QDBusConnection::systemBus().isConnected()) {
        qFatal("failed to connect to dbus");
        exit(1);
    }

    dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", QDBusConnection::systemBus(), this);

    writeTimer.setInterval(writeInterval);
    QObject::connect(&writeTimer, &QTimer::timeout, this, &DbdPerfTestWriter::performWrite, Qt::DirectConnection);
    writeTimer.start();
}

void DbdPerfTestWriter::stop()
{
    running = false;
    writeTimer.stop();
    emit finished();
}

void DbdPerfTestWriter::performWrite()
{
    auto reply = dbClient->write(QString("test"), QString("write"));

    // wait until the dbus reply is ready
    reply.waitForFinished();

    // if it's valid, print it
    if (!reply.isValid()) {
        updateLock.lock();
        writeErrorCount += 1;
        updateLock.unlock();
    } else {
        updateLock.lock();
        writeSuccessCount += 1;
        updateLock.unlock();
    }
}

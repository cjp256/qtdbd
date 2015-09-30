#ifndef DBDPERFTEST_H
#define DBDPERFTEST_H

#include <QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QTimer>
#include <QThread>
#include <QElapsedTimer>
#include <db_proxy.h>



class DbdPerfTestReader: public QThread
{
    Q_OBJECT
public:
    DbdPerfTestReader() : readInterval(1.0), readIterations(1000), readTimer(NULL), updateLock(NULL), readSuccessCount(0), readErrorCount(0), dbClient(NULL)
    {
    }

    ~DbdPerfTestReader()
    {
    }

public slots:

    void performRead()
    {
        auto reply = dbClient->read(QString("key"));

        // wait until the dbus reply is ready
        reply.waitForFinished();

        // if it's valid, print it
        if (!reply.isValid()) {
            updateLock->lock();
            readErrorCount += 1;
            updateLock->unlock();
            return;
        }

        if (reply.value() == QString("value")) {
            updateLock->lock();
            readSuccessCount += 1;
            updateLock->unlock();
        } else {
            qDebug() << "reading bad (valid) value??" << reply.value();
            updateLock->lock();
            readErrorCount += 1;
            updateLock->unlock();
        }

        if (readIterations <= (readSuccessCount + readErrorCount)) {
            // time to exit
            QCoreApplication::quit();
        }
    }

    void run()
    {
        if (!QDBusConnection::systemBus().isConnected()) {
            qFatal("failed to connect to dbus");
            exit(1);
        }

        updateLock = new QMutex();
        dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", QDBusConnection::systemBus(), this);

        // write known test key to be read in later
        dbClient->write(QString("key"), QString("value"));

        readTimer = new QTimer(this);
        readTimer->setInterval(readInterval);
        QObject::connect(readTimer, &QTimer::timeout, this, &DbdPerfTestReader::performRead, Qt::DirectConnection);
        readTimer->moveToThread(this);
        readTimer->start();
        exec();
    }

public:
    double readInterval;
    qulonglong readIterations;
    QTimer *readTimer;
    QMutex *updateLock;
    qulonglong readSuccessCount;
    qulonglong readErrorCount;
    ComCitrixXenclientDbInterface *dbClient;
Q_SIGNALS:
};

class DbdPerfTestWriter: public QThread
{
    Q_OBJECT
public:
    DbdPerfTestWriter() : writeInterval(1.0), writeTimer(NULL), updateLock(NULL), writeSuccessCount(0), writeErrorCount(0), dbClient(NULL)
    {
    }

    ~DbdPerfTestWriter()
    {
    }

public slots:

    void performWrite()
    {
        auto reply = dbClient->write(QString("test"), QString("write"));

        // wait until the dbus reply is ready
        reply.waitForFinished();

        // if it's valid, print it
        if (!reply.isValid()) {
            updateLock->lock();
            writeErrorCount += 1;
            updateLock->unlock();
        } else {
            updateLock->lock();
            writeSuccessCount += 1;
            updateLock->unlock();
        }
    }

    void run()
    {
        if (!QDBusConnection::systemBus().isConnected()) {
            qFatal("failed to connect to dbus");
            exit(1);
        }

        dbClient = new ComCitrixXenclientDbInterface("com.citrix.xenclient.db", "/", QDBusConnection::systemBus(), this);

        updateLock = new QMutex();
        writeTimer = new QTimer(this);
        writeTimer->setInterval(writeInterval);
        QObject::connect(writeTimer, &QTimer::timeout, this, &DbdPerfTestWriter::performWrite, Qt::DirectConnection);
        writeTimer->moveToThread(this);
        writeTimer->start();
        exec();
    }

public:
    double writeInterval;
    QTimer *writeTimer;
    QMutex *updateLock;
    qulonglong writeSuccessCount;
    qulonglong writeErrorCount;
    ComCitrixXenclientDbInterface *dbClient;
Q_SIGNALS:
};

class DbdPerfTest: public QObject
{
    Q_OBJECT
public:
    DbdPerfTest(QCoreApplication *app);
    ~DbdPerfTest();
public slots:
    void parseCommandLine();
    void exitCleanup();
    void printSummary();
    void startTests();
private:
    QCoreApplication *app;
    QCommandLineParser parser;
    double readInterval;
    double writeInterval;
    qulonglong readIterations;
    qulonglong numberVms;
    QTimer printTimer;
    QElapsedTimer elapsedTimer;
    DbdPerfTestReader reader;
    DbdPerfTestWriter writer;
Q_SIGNALS:
};
#endif // DBDPERFTEST_H

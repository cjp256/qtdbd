#ifndef DBDPERFTESTREADER_H
#define DBDPERFTESTREADER_H

#include <QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QTimer>
#include <QThread>
#include <db_proxy.h>

class DbdPerfTestReader: public QObject
{
    Q_OBJECT
public:
    DbdPerfTestReader();
    ~DbdPerfTestReader();

public slots:
    void setup();
    void performRead();
    void stop();

public:
    bool running;
    double readInterval;
    qulonglong readIterations;
    QTimer readTimer;
    QMutex updateLock;
    qulonglong readSuccessCount;
    qulonglong readErrorCount;
    ComCitrixXenclientDbInterface *dbClient;

signals:
    void finished();
};

#endif // DBDPERFTESTREADER_H

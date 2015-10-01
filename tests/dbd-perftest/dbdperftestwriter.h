#ifndef DBDPERFTESTWRITER_H
#define DBDPERFTESTWRITER_H

#include <QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QTimer>
#include <QThread>
#include <comcitrixxenclientdbinterface.h>

class DbdPerfTestWriter: public QObject
{
    Q_OBJECT
public:
    DbdPerfTestWriter();
    ~DbdPerfTestWriter();

public slots:
    void setup();
    void stop();
    void performWrite();

public:
    bool running;
    double writeInterval;
    QTimer writeTimer;
    QMutex updateLock;
    qulonglong writeSuccessCount;
    qulonglong writeErrorCount;
    ComCitrixXenclientDbInterface *dbClient;

signals:
    void finished();
};
#endif // DBDPERFTESTWRITER_H

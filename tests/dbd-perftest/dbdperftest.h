#ifndef DBDPERFTEST_H
#define DBDPERFTEST_H

#include <QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include <QTimer>
#include <QThread>
#include <QElapsedTimer>
#include <comcitrixxenclientdbinterface.h>
#include <dbdperftestreader.h>
#include <dbdperftestwriter.h>

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
    double readInterval;
    double writeInterval;
    qulonglong readIterations;
    qulonglong numberVms;
    QTimer printTimer;
    QElapsedTimer elapsedTimer;
    QThread *readerThread;
    DbdPerfTestReader *reader;
    QThread *writerThread;
    DbdPerfTestWriter *writer;

signals:
};
#endif // DBDPERFTEST_H

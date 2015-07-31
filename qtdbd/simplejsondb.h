#ifndef SIMPLEJSONDB_H
#define SIMPLEJSONDB_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QVariantMap>

class SimpleJsonDB: public QObject
{
    Q_OBJECT
public:
    SimpleJsonDB(QString path, QString vpath, int maxFlushDelayMillis);
    ~SimpleJsonDB();
    QVariantMap dbMap;
    bool filterVmAndDomstoreKeys;
public Q_SLOTS:
    void readFromDisk();
    void writeToDisk();
    void debugJsonObject();
private:
    QString path;
    QString vpath;
    int maxFlushDelay;
    QMutex fileLock;
    QTimer *flushTimer;
    void dbChanged();
Q_SIGNALS: // SIGNALS
};

#endif // SIMPLEJSONDB_H

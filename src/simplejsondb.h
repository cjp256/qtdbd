#ifndef SIMPLEJSONDB_H
#define SIMPLEJSONDB_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QVariantMap>

#include <qmjson.h>

class SimpleJsonDB: public QObject
{
    Q_OBJECT
public:
    SimpleJsonDB(const QString vpath, const QString path=":memory:", int maxFlushDelayMillis = 3000);
    ~SimpleJsonDB();
public Q_SLOTS:
    QString jsonString();
    QMPointer<QMJsonValue> getValue();
    void readFromDisk();
    void setFilterVmAndDomstoreKeys(bool filter);
    void setMaxFlushDelay(int maxFlushDelayMillis);
    void setWorkerThread(QThread *workerThread);
    void flush();
    void queueFlush(bool delayed);
private:
    QMPointer<QMJsonValue> db;
    QString vpath;
    QString path;
    QMutex fileLock;
    QTimer flushTimer;
    int maxFlushDelay;
    bool skipDisk;
    bool filterVmAndDomstoreKeys;
Q_SIGNALS:
};

#endif // SIMPLEJSONDB_H

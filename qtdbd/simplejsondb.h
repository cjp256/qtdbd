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
    SimpleJsonDB(const QString vpath);
    SimpleJsonDB(const QString path, const QString vpath, int maxFlushDelayMillis);
    ~SimpleJsonDB();
    bool filterVmAndDomstoreKeys;
public Q_SLOTS:
    QString jsonString();
    QMPointer<QMJsonValue> readFromDisk();
    void writeToDisk(const QString &jsonString);
private:
    QMPointer<QMJsonValue> db;
    QString path;
    QString vpath;
    int maxFlushDelay;
    QMutex fileLock;
    QTimer *flushTimer;
    bool skipDisk;
    void dbChanged();
Q_SIGNALS: // SIGNALS
};

#endif // SIMPLEJSONDB_H

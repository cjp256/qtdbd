#ifndef SIMPLEJSONDB_H
#define SIMPLEJSONDB_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>
#include <QVariantMap>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;

class SimpleJsonDB: public QObject
{
    Q_OBJECT
public:
    SimpleJsonDB(Value *v, QString vpath);
    SimpleJsonDB(Value *v, QString path, QString vpath, int maxFlushDelayMillis);
    ~SimpleJsonDB();
    bool filterVmAndDomstoreKeys;
public Q_SLOTS:
    QString jsonString();
    void readFromDisk();
    void writeToDisk();
private:
    Value *db;
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

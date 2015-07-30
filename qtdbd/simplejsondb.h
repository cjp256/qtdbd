#ifndef SIMPLEJSONDB_H
#define SIMPLEJSONDB_H

#include <QObject>
#include <QMutex>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHash>

class SimpleJsonDB: public QObject
{
    Q_OBJECT
public:
    SimpleJsonDB(QString path, QString vpath, int maxFlushDelayMillis);
    ~SimpleJsonDB();
    QVariantHash dbHashTable;
    bool filterVmAndDomstoreKeys;
public Q_SLOTS:
    void readFromDisk();
    void writeToDisk();
private:
    QString path;
    QString vpath;
    int maxFlushDelay;
    QMutex fileLock;
    QTimer *flushTimer;
    QJsonDocument jsonDocument;
    QJsonObject jsonObject;
    void dbChanged();
Q_SIGNALS: // SIGNALS
};

#endif // SIMPLEJSONDB_H

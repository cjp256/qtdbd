#ifndef DBTREE_H
#define DBTREE_H

#include <QObject>
#include "simplejsondb.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;

class DBTree : public QObject
{
    Q_OBJECT
public:
    DBTree();
    DBTree(QString dbPath, int maxFlushDelayMillis);
    ~DBTree();
    Value *getObject(const QStringList &splitPath);
    void setObject(QStringList splitPath, Value &value);
    QString valueToJsonString(Value *d);
    Document dbRoot;
private:
    QString dbPath;
    int maxFlushDelay;
    SimpleJsonDB mainDb;

signals:

public slots:
};

#endif // DBTREE_H

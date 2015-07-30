#include "simplejsondb.h"
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QSaveFile>
#include <QtDebug>

SimpleJsonDB::SimpleJsonDB(QString path, QString vpath, int maxFlushDelayMillis) : path(path), vpath(vpath), maxFlushDelay(maxFlushDelayMillis), fileLock()
{
    filterVmAndDomstoreKeys = false;
    flushTimer = new QTimer(this);
    flushTimer->setSingleShot(true);
    //connect(flushTimer, SIGNAL(dbChanged()), this, SLOT(writeToDisk()));
    readFromDisk();
}

SimpleJsonDB::~SimpleJsonDB()
{
}

void SimpleJsonDB::readFromDisk()
{
    fileLock.lock();

    // read file into string
    qDebug() << "opening file: " << path;
    QString val;
    QFile file(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    qDebug() << "closed file: " << path;

    // convert to json document
    jsonDocument = QJsonDocument::fromJson(val.toUtf8());
    qDebug() << "json read OK from file: " << path;

    jsonObject = jsonDocument.object();
    qDebug() << "qjsonobject read OK from file: " << path << "dump:" << QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);

    dbHashTable = jsonObject.toVariantHash();

    fileLock.unlock();
}

void SimpleJsonDB::writeToDisk()
{
    fileLock.lock();

    // build new json object and strip vm/dom-store keys as required
    jsonObject = QJsonObject::fromVariantHash(dbHashTable);
    if (filterVmAndDomstoreKeys) {
        jsonObject.remove("vm");
        jsonObject.remove("dom-store");
    }

    if (jsonObject.isEmpty()) {
        // db is empty, remove old db file (if it exists)
        QFile file(path);
        file.remove();
    } else {
        // save json file with atomic QSaveFile
        QJsonDocument doc(jsonObject);
        QString strJson(doc.toJson(QJsonDocument::Indented));
        QSaveFile file(path);
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream outStream(&file);
        outStream << strJson;
        file.commit();
    }

    fileLock.unlock();
}

void SimpleJsonDB::dbChanged()
{
    if (!flushTimer->isActive()) {
        flushTimer->start(maxFlushDelay);
    }
}

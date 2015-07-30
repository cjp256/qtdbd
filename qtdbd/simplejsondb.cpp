#include "simplejsondb.h"
#include <QTimer>
#include <QMutex>
#include <QFile>
#include <QtDebug>

SimpleJsonDB::SimpleJsonDB(QString path, QString vpath, int maxFlushDelayMillis) : path(path), vpath(vpath), maxFlushDelay(maxFlushDelayMillis), fileLock()
{
    flushTimer = new QTimer(this);
    flushTimer->setSingleShot(true);
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
    QFile file;
    file.setFileName(path);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    val = file.readAll();
    file.close();
    qDebug() << "closed file: " << path;

    // convert to json document
    jsonDocument = QJsonDocument::fromJson(val.toUtf8());
    qDebug() << "json read OK from file: " << path;

    jsonObject = jsonDocument.object();
    qDebug() << "qjsonobject read OK from file: " << path << "dump:" << QJsonDocument(jsonObject).toJson(QJsonDocument::Compact);

    fileLock.unlock();
}

void SimpleJsonDB::writeToDisk()
{
    fileLock.lock();

    if (jsonObject.isEmpty()) {
        QFile file;
        file.setFileName(path);
        file.remove();
    } else {
        //if (path == "/config/db") then filter 'vm' and 'dom-store'
        //and write data to file
        QJsonDocument doc(jsonObject);
        QString strJson(doc.toJson(QJsonDocument::Indented));
        QFile file;
        file.setFileName(path + ".tmp");
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream outStream(&file);
        outStream << strJson;
        file.flush();
        file.rename(path);
        file.close();
    }

    fileLock.unlock();
}

void SimpleJsonDB::dbChanged()
{
    if (!flushTimer->isActive()) {
        flushTimer->start(maxFlushDelay);
    }
}

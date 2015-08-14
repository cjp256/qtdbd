#ifndef TESTDBD_H
#define TESTDBD_H

#include <QObject>

class TestDBD: public QObject
{
Q_OBJECT
public:
    TestDBD();
    QStringList splitPath(QString path);
private slots:
    void testDbTreeBasicGetSet();
    void testDbBasicReadWrite();
    void testDb1BasicReadWrite();
    void testDb1VariousTypesRead();
    void testDbBasicDump();
    void testDbBasicExists();
};

#endif // TESTDBD_H

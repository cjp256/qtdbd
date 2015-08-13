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
};

#endif // TESTDBD_H

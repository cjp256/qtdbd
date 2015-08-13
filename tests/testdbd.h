#ifndef TESTDBD_H
#define TESTDBD_H

#include <QObject>

class TestDBD: public QObject
{
Q_OBJECT
public:
    TestDBD();
private slots:
    void test1();
};

#endif // TESTDBD_H

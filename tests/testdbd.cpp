
#include "testdbd.h"

#include <QtTest/QtTest>

TestDBD::TestDBD()
{

}

void TestDBD::test1()
{
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HELLO"));
}

QTEST_MAIN(TestDBD)

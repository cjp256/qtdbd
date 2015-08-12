
#include "testdbtree.h"

#include <QtTest/QtTest>

void TestDBTree::toUpper()
{
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HELLO"));
}

QTEST_MAIN(TestDBTree)
#include "testqstring.moc"


#include "testdbd.h"

#include <QtTest/QtTest>
#include <dbtree.h>
#include <qmjson.h>

TestDBD::TestDBD()
{

}

QStringList TestDBD::splitPath(QString path)
{
    return QString(path).split("/", QString::SplitBehavior::SkipEmptyParts);
}

void TestDBD::testDbTreeBasicGetSet()
{
    DBTree dbTree;
    QStringList path;
    //QMPointer<QMJsonValue> null = QMPointer<QMJsonValue>(new QMJsonValue());

    path = splitPath("/");
    QCOMPARE(dbTree.getObject(splitPath("/"))->toJson(), QString("{}"));

    path = splitPath("");
    QCOMPARE(dbTree.getObject(splitPath(""))->toJson(), QString("{}"));

    path = splitPath("/somekey");
    dbTree.setObject(path, "somevalue");
    QCOMPARE(dbTree.getObject(path)->toJson(), QString("\"somevalue\""));
    QCOMPARE(dbTree.getObject(path)->isString(), true);
    QCOMPARE(dbTree.getObject(path)->toString(), QString("somevalue"));

    path = splitPath("/some/other/key");
    dbTree.setObject(path, "someothervalue");
    QCOMPARE(dbTree.getObject(path)->toJson(), QString("\"someothervalue\""));
    QCOMPARE(dbTree.getObject(path)->isString(), true);
    QCOMPARE(dbTree.getObject(path)->toString(), QString("someothervalue"));

    QCOMPARE(dbTree.getObject(splitPath(""))->toJson(QMJSONVALUE_OPTIMIZED), QString("{\"somekey\":\"somevalue\",\"some\":{\"other\":{\"key\":\"someothervalue\"}}}"));
}

QTEST_MAIN(TestDBD)

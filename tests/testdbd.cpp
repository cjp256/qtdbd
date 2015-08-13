
#include "testdbd.h"
#include "db.h"
#include "db_adaptor.h"

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
    QCOMPARE(dbTree.getObject(splitPath(""))->toJson(QMJSONVALUE_OPTIMIZED), QString("{\"somekey\":\"somevalue\"}"));

    path = splitPath("/some/other/key");
    dbTree.setObject(path, "someothervalue");
    QCOMPARE(dbTree.getObject(path)->toJson(), QString("\"someothervalue\""));
    QCOMPARE(dbTree.getObject(path)->isString(), true);
    QCOMPARE(dbTree.getObject(path)->toString(), QString("someothervalue"));
    //QCOMPARE(dbTree.getObject(splitPath(""))->toJson(QMJSONVALUE_OPTIMIZED), QString("{\"somekey\":\"somevalue\",\"some\":{\"other\":{\"key\":\"someothervalue\"}}}"));

    path = splitPath("/some/other/key");
    dbTree.setObject(path, "someothervalue2");
    QCOMPARE(dbTree.getObject(path)->toJson(), QString("\"someothervalue2\""));
    QCOMPARE(dbTree.getObject(path)->isString(), true);
    QCOMPARE(dbTree.getObject(path)->toString(), QString("someothervalue2"));
    //QCOMPARE(dbTree.getObject(splitPath(""))->toJson(QMJSONVALUE_OPTIMIZED), QString("{\"somekey\":\"somevalue\",\"some\":{\"other\":{\"key\":\"someothervalue2\"}}}"));
}


void TestDBD::testDbBasicReadWrite()
{
    DBTree *dbTree = new DBTree(":memory:", 1000);
    Db *db = new Db(dbTree, false);
    new DbInterfaceAdaptor(db);

    QCOMPARE(db->read("/"), QString(""));
    QCOMPARE(db->read(""), QString(""));

    QCOMPARE(db->read("/notexist"), QString(""));
    QCOMPARE(db->read("notexist"), QString(""));

    QCOMPARE(db->read("/does/notexist"), QString(""));
    QCOMPARE(db->read("does/notexist"), QString(""));

    QCOMPARE(db->read("/does/not/exist"), QString(""));
    QCOMPARE(db->read("does/not/exist"), QString(""));

    db->write("/somekey", "somevalue");
    QCOMPARE(db->read("/somekey"), QString("somevalue"));
    QCOMPARE(db->read("somekey"), QString("somevalue"));

    db->write("somekey", "somevalue2");
    QCOMPARE(db->read("/somekey"), QString("somevalue2"));
    QCOMPARE(db->read("somekey"), QString("somevalue2"));

    db->write("/a/somekey", "somevalue");
    QCOMPARE(db->read("/a/somekey"), QString("somevalue"));
    QCOMPARE(db->read("a/somekey"), QString("somevalue"));

    db->write("/a/b/somekey", "somevalue");
    QCOMPARE(db->read("/a/b/somekey"), QString("somevalue"));
    QCOMPARE(db->read("a/b/somekey"), QString("somevalue"));

    db->write("/x/y/z/somekey", "somevalue");
    QCOMPARE(db->read("/x/y/z/somekey"), QString("somevalue"));
    QCOMPARE(db->read("x/y/z/somekey"), QString("somevalue"));

    db->write("/x/y/z/somekey", "somevalue2");
    QCOMPARE(db->read("/x/y/z/somekey"), QString("somevalue2"));
    QCOMPARE(db->read("x/y/z/somekey"), QString("somevalue2"));

    db->write("/x/y/somekey", "somevalue");
    QCOMPARE(db->read("/x/y/somekey"), QString("somevalue"));
    QCOMPARE(db->read("x/y/somekey"), QString("somevalue"));

    db->write("/x/somekey", "somevalue");
    QCOMPARE(db->read("/x/somekey"), QString("somevalue"));
    QCOMPARE(db->read("x/somekey"), QString("somevalue"));
    QCOMPARE(db->read("/x/y/somekey"), QString("somevalue"));
    QCOMPARE(db->read("x/y/somekey"), QString("somevalue"));
    QCOMPARE(db->read("/x/y/z/somekey"), QString("somevalue2"));
    QCOMPARE(db->read("x/y/z/somekey"), QString("somevalue2"));

    QCOMPARE(db->read("/a/somekey"), QString("somevalue"));
    QCOMPARE(db->read("a/somekey"), QString("somevalue"));
}

QTEST_MAIN(TestDBD)


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
    QCOMPARE(dbTree.getValue(splitPath("/"))->toJson(), QString("{}"));

    path = splitPath("");
    QCOMPARE(dbTree.getValue(splitPath(""))->toJson(), QString("{}"));

    path = splitPath("/somekey");
    dbTree.setValue(path, "somevalue");
    QCOMPARE(dbTree.getValue(path)->toJson(), QString("\"somevalue\""));
    QCOMPARE(dbTree.getValue(path)->isString(), true);
    QCOMPARE(dbTree.getValue(path)->toString(), QString("somevalue"));
    QCOMPARE(dbTree.getValue(splitPath(""))->toJson(QMJSONVALUE_OPTIMIZED), QString("{\"somekey\":\"somevalue\"}"));

    path = splitPath("/some/other/key");
    dbTree.setValue(path, "someothervalue");
    QCOMPARE(dbTree.getValue(path)->toJson(), QString("\"someothervalue\""));
    QCOMPARE(dbTree.getValue(path)->isString(), true);
    QCOMPARE(dbTree.getValue(path)->toString(), QString("someothervalue"));
    //QCOMPARE(dbTree.getValue(splitPath(""))->toJson(QMJSONVALUE_OPTIMIZED), QString("{\"somekey\":\"somevalue\",\"some\":{\"other\":{\"key\":\"someothervalue\"}}}"));

    path = splitPath("/some/other/key");
    dbTree.setValue(path, "someothervalue2");
    QCOMPARE(dbTree.getValue(path)->toJson(), QString("\"someothervalue2\""));
    QCOMPARE(dbTree.getValue(path)->isString(), true);
    QCOMPARE(dbTree.getValue(path)->toString(), QString("someothervalue2"));
    //QCOMPARE(dbTree.getValue(splitPath(""))->toJson(QMJSONVALUE_OPTIMIZED), QString("{\"somekey\":\"somevalue\",\"some\":{\"other\":{\"key\":\"someothervalue2\"}}}"));
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

void TestDBD::testDb1BasicReadWrite()
{
    // test read write on keys that don't exist in db
    DBTree *dbTree = new DBTree("tests/db-1", -1);
    Db *db = new Db(dbTree, false);
    new DbInterfaceAdaptor(db);

    QCOMPARE(db->read("/"), QString(""));
    QCOMPARE(db->read(""), QString(""));

    QCOMPARE(db->read("/notexist"), QString(""));
    QCOMPARE(db->read("notexist"), QString(""));

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

void TestDBD::testDb1VariousTypesRead()
{
    // test read of various types that exist in db
    DBTree *dbTree = new DBTree("tests/db-1", -1);
    Db *db = new Db(dbTree, false);
    new DbInterfaceAdaptor(db);

    QCOMPARE(db->read("/"), QString(""));

    // pull various types data of of the db
    QCOMPARE(db->read("booltrue"), QString("true"));
    QCOMPARE(db->read("boolfalse"), QString("false"));
    QCOMPARE(db->read("string"), QString("blue"));
    QCOMPARE(db->read("number"), QString("56"));
    QCOMPARE(db->read("null"), QString("null"));
    QCOMPARE(db->read("array"), QString(""));
    QCOMPARE(db->read("object"), QString(""));
    QCOMPARE(db->read("object/booltrue"), QString("true"));
    QCOMPARE(db->read("object/boolfalse"), QString("false"));
    QCOMPARE(db->read("object/string"), QString("blue"));
    QCOMPARE(db->read("object/number"), QString("56"));
    QCOMPARE(db->read("object/null"), QString("null"));
    QCOMPARE(db->read("object/array"), QString(""));
    QCOMPARE(db->read("object/object"), QString(""));
    QCOMPARE(db->read("object/object/booltrue"), QString("true"));
    QCOMPARE(db->read("object/object/boolfalse"), QString("false"));
    QCOMPARE(db->read("object/object/string"), QString("blue"));
    QCOMPARE(db->read("object/object/number"), QString("56"));
    QCOMPARE(db->read("object/object/null"), QString("null"));
    QCOMPARE(db->read("object/object/array"), QString(""));
}


void TestDBD::testDbBasicDump()
{
    DBTree *dbTree = new DBTree(":memory:", 1000);
    Db *db = new Db(dbTree, false);
    new DbInterfaceAdaptor(db);

    QCOMPARE(db->read("/"), QString(""));
    QCOMPARE(db->read(""), QString(""));

    db->write("/somekey", "somevalue");
    QCOMPARE(db->read("/somekey"), QString("somevalue"));
    QCOMPARE(db->dump("/somekey"), QString("\"somevalue\""));
    QCOMPARE(db->dump("somekey"), QString("\"somevalue\""));
    QCOMPARE(db->dump("/"), QString("{\"somekey\":\"somevalue\"}"));
    QCOMPARE(db->dump(""), QString("{\"somekey\":\"somevalue\"}"));
}

void TestDBD::testDbBasicExists()
{
    DBTree *dbTree = new DBTree(":memory:", 1000);
    Db *db = new Db(dbTree, false);
    new DbInterfaceAdaptor(db);

    QCOMPARE(db->exists("/"), true);
    QCOMPARE(db->exists(""), true);

    QCOMPARE(db->exists("/notexists"), false);
    QCOMPARE(db->exists("notexists"), false);

    QCOMPARE(db->exists("/not/exists"), false);
    QCOMPARE(db->exists("not/exists"), false);

    db->write("/somekey", "somevalue");
    QCOMPARE(db->exists("/somekey"), true);
    QCOMPARE(db->exists("somekey"), true);
}

void TestDBD::testDbBasicList()
{
    DBTree *dbTree = new DBTree(":memory:", 1000);
    Db *db = new Db(dbTree, false);
    new DbInterfaceAdaptor(db);

    QCOMPARE(db->list("/"), QStringList());
    QCOMPARE(db->list(""), QStringList());

    db->write("/somekey", "somevalue");
    QCOMPARE(db->list("/somekey"), QStringList());

    db->write("/x/a", "somevalue");
    db->write("/x/b", "somevalue");
    db->write("/x/c", "somevalue");

    QStringList results = db->list("/x");
    results.sort();

    QStringList expected;
    expected << "a" << "b" << "c";
    expected.sort();
    QCOMPARE(db->list("/x"), expected);

    db->write("/x/d", "somevalue");

    results = db->list("/x");
    results.sort();

    expected << "d";
    expected.sort();
    QCOMPARE(results, expected);

    QCOMPARE(db->list("/x/d"), QStringList());
}

QTEST_MAIN(TestDBD)

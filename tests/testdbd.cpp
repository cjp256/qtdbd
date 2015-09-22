
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

    auto val = QMPointer<QMJsonValue>(new QMJsonValue("somevalue"));
    dbTree.setValue(path, val);
    QCOMPARE(dbTree.getValue(path)->toJson(), QString("\"somevalue\""));
    QCOMPARE(dbTree.getValue(path)->isString(), true);
    QCOMPARE(dbTree.getValue(path)->toString(), QString("somevalue"));
    QCOMPARE(dbTree.getValue(splitPath(""))->toJson(QMJsonFormat_Optimized, QMJsonSort_CaseSensitive), QString("{\"somekey\":\"somevalue\"}"));

    val = QMPointer<QMJsonValue>(new QMJsonValue("someothervalue"));
    path = splitPath("/some/other/key");
    dbTree.setValue(path, val);
    QCOMPARE(dbTree.getValue(path)->toJson(), QString("\"someothervalue\""));
    QCOMPARE(dbTree.getValue(path)->isString(), true);
    QCOMPARE(dbTree.getValue(path)->toString(), QString("someothervalue"));
    QCOMPARE(dbTree.getValue(splitPath(""))->toJson(QMJsonFormat_Optimized, QMJsonSort_CaseSensitive), QString("{\"some\":{\"other\":{\"key\":\"someothervalue\"}},\"somekey\":\"somevalue\"}"));

    val = QMPointer<QMJsonValue>(new QMJsonValue("someothervalue2"));
    path = splitPath("/some/other/key");
    dbTree.setValue(path, val);
    QCOMPARE(dbTree.getValue(path)->toJson(), QString("\"someothervalue2\""));
    QCOMPARE(dbTree.getValue(path)->isString(), true);
    QCOMPARE(dbTree.getValue(path)->toString(), QString("someothervalue2"));
    QCOMPARE(dbTree.getValue(splitPath(""))->toJson(QMJsonFormat_Optimized, QMJsonSort_CaseSensitive), QString("{\"some\":{\"other\":{\"key\":\"someothervalue2\"}},\"somekey\":\"somevalue\"}"));
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
    QCOMPARE(db->dump(""), QString("{}"));

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
    QCOMPARE(results, expected);

    db->write("/x/d", "somevalue");

    results = db->list("/x");
    results.sort();

    expected << "d";
    expected.sort();
    QCOMPARE(results, expected);

    QCOMPARE(db->list("/x/d"), QStringList());
}

void TestDBD::testDbBasicRm()
{
    DBTree *dbTree = new DBTree(":memory:", 1000);
    Db *db = new Db(dbTree, false);
    new DbInterfaceAdaptor(db);

    QCOMPARE(db->exists(""), true);
    db->rm("/");
    QCOMPARE(db->exists(""), true);

    db->write("/somekey", "somevalue");
    QCOMPARE(db->exists("/somekey"), true);

    db->rm("/somekey");
    QCOMPARE(db->exists("/somekey"), false);

    db->write("/x/a", "somevalue");
    QCOMPARE(db->exists("/x/a"), true);
    db->rm("/x/a");
    QCOMPARE(db->exists("/x/a"), false);
    QCOMPARE(db->exists("/x"), true);

    db->write("/x/a/b", "somevalue");
    QCOMPARE(db->exists("/x/a/b"), true);
    db->rm("/x/a");
    QCOMPARE(db->exists("/x/a/b"), false);
    QCOMPARE(db->exists("/x/a"), false);
    QCOMPARE(db->exists("/x"), true);
}

void TestDBD::testDbBasicInject()
{
    DBTree *dbTree = new DBTree(":memory:", 1000);
    Db *db = new Db(dbTree, false);
    new DbInterfaceAdaptor(db);

    QCOMPARE(db->exists(""), true);

    QString serviceNdvm = QString("{ \
        \"uuid\": \"00000000-0000-0000-0000-000000000002\", \
        \"type\": \"ndvm\", \
        \"name\": \"Network2\", \
        \"slot\": \"-1\", \
        \"hidden\": \"false\", \
        \"start_on_boot\": \"true\", \
        \"start_on_boot_priority\": \"10\", \
        \"provides-network-backend\": \"true\", \
        \"provides-default-network-backend\": \"true\", \
        \"shutdown-priority\": \"-15\", \
        \"hidden-in-ui\": \"false\", \
        \"measured\": \"false\", \
        \"s3-mode\": \"restart\", \
        \"domstore-read-access\": \"true\", \
        \"domstore-write-access\": \"true\", \
        \"image_path\": \"plugins/serviceimages/citrix.png\", \
        \"icbinn-path\": \"\\/config\\/certs\\/Network\", \
        \"boot-sentinel\": \"booted\", \
        \"v4v-firewall-rules\": { \
          \"0\": \"myself -> 0:5555\", \
          \"1\": \"0 -> myself:2222\", \
          \"2\": \"myself -> 0:2222\", \
          \"3\": \"myself:5555 -> 0\", \
          \"4\": \"myself -> 0:4878\" \
        }, \
        \"rpc-firewall-rules\": { \
            \"0\": \"allow destination org.freedesktop.DBus interface org.freedesktop.DBus\", \
            \"1\": \"allow destination com.citrix.xenclient.xenmgr interface org.freedesktop.DBus.Properties member Get\", \
            \"2\": \"allow destination com.citrix.xenclient.networkdaemon\" \
        }, \
        \"policies\": { \
          \"audio-access\": \"false\", \
          \"audio-rec\": \"false\", \
          \"cd-access\": \"false\", \
          \"cd-rec\": \"false\", \
          \"modify-vm-settings\": \"false\" \
        }, \
        \"config\": { \
          \"notify\": \"dbus\", \
          \"debug\": \"true\", \
          \"pae\": \"true\", \
          \"acpi\": \"true\", \
          \"hvm\": \"false\", \
          \"apic\": \"true\", \
          \"nx\": \"true\", \
          \"v4v\": \"true\", \
          \"memory\": \"176\", \
          \"display\": \"none\", \
          \"cmdline\": \"root=\\/dev\\/xvda1 iommu=soft xencons=hvc0\", \
          \"kernel-extract\": \"\\/boot\\/vmlinuz\", \
          \"flask-label\": \"system_u:system_r:ndvm_t\", \
          \"pci\": { \
            \"0\": { \
              \"class\": \"0x0200\", \
              \"force-slot\": \"false\" \
            }, \
            \"1\": { \
              \"class\": \"0x0280\", \
              \"force-slot\": \"false\" \
            } \
          }, \
          \"disk\": { \
            \"0\": { \
              \"path\": \"\\/storage\\/ndvm\\/ndvm.vhd\", \
              \"type\": \"vhd\", \
              \"mode\": \"r\", \
              \"shared\": \"true\", \
              \"device\": \"xvda1\", \
              \"devtype\": \"disk\" \
            }, \
            \"1\": { \
              \"path\": \"\\/storage\\/ndvm\\/ndvm-swap.vhd\", \
              \"type\": \"vhd\", \
              \"mode\": \"w\", \
              \"device\": \"xvda2\", \
              \"devtype\": \"disk\" \
            } \
          }, \
          \"qemu-dm-path\": \"\" \
        } \
      } \
    ");

    auto val = QMJsonValue::fromJson(serviceNdvm);
    auto str = val->toJson(QMJsonFormat_Optimized, QMJsonSort_CaseSensitive);
    db->inject("/vm/somendvm", serviceNdvm);

    auto dumpval = QMJsonValue::fromJson(db->dump("/vm/somendvm"));
    auto dumpstr = dumpval->toJson(QMJsonFormat_Optimized, QMJsonSort_CaseSensitive);

    QCOMPARE(str, dumpstr);
    QCOMPARE(dumpval->toObject()->value("uuid")->toString(), QString("00000000-0000-0000-0000-000000000002"));
    QCOMPARE(dumpval->toObject()->value("config")->toObject()->value("pae")->toString(), QString("true"));

    // twiddle some bits
    val->toObject()->value("uuid")->fromString("12345");
    val->toObject()->value("config")->toObject()->value("pae")->fromString("false");

    str = val->toJson(QMJsonFormat_Optimized, QMJsonSort_CaseSensitive);

    db->inject("/vm/somendvm", str);

    dumpval = QMJsonValue::fromJson(db->dump("/vm/somendvm"));
    dumpstr = dumpval->toJson(QMJsonFormat_Optimized, QMJsonSort_CaseSensitive);

    QCOMPARE(str, dumpstr);
    QCOMPARE(dumpval->toObject()->value("uuid")->toString(), QString("12345"));
    QCOMPARE(dumpval->toObject()->value("config")->toObject()->value("pae")->toString(), QString("false"));
}

QTEST_MAIN(TestDBD)

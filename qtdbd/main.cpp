#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>
#include "db.h"
#include "db_adaptor.h"

int main(int argc, char *argv[])
{
   QCoreApplication app(argc, argv);

   if (!QDBusConnection::sessionBus().isConnected()) {
       fprintf(stderr, "Cannot connect to the D-Bus system bus.\n");
       return 1;
   }

   if (!QDBusConnection::sessionBus().registerService("com.citrix.xenclient.db")) {
       fprintf(stderr, "%s\n",
               qPrintable(QDBusConnection::sessionBus().lastError().message()));
       exit(1);
   }

   Db *db = new Db();
   new DbInterfaceAdaptor(db);

   QDBusConnection::sessionBus().registerObject("/", "com.citrix.xenclient.db", db, QDBusConnection::ExportAllSlots);

   printf("registered...\n");
   app.exec();
}

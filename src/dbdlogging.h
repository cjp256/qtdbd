#ifndef DBDLOGGING_H
#define DBDLOGGING_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <QObject>
#include <QtGlobal>

class DbdLogging : public QObject
{
    Q_OBJECT

public:
    DbdLogging() : debugMode(false), syslogMode(false)
    {
        openlog(NULL, LOG_PID, LOG_DAEMON);
    }

    static void logOutput(QtMsgType type, const QMessageLogContext&, const QString& msg)
    {
        DbdLogging *logger = DbdLogging::logger();

        switch (type) {
        case QtDebugMsg:
            if (logger->debugMode) {
                if (logger->syslogMode) {
                    syslog(LOG_DEBUG, "[DEBUG] %s\n", qPrintable(msg));
                } else {
                    fprintf(stderr, "[DEBUG] %s\n", qPrintable(msg));
                }
            }
            break;
    #if QT_VERSION >= 0x050500
        case QtInfoMsg:
            if (logger->syslogMode) {
                syslog(LOG_INFO, "[INFO] %s\n", qPrintable(msg));
            } else {
                fprintf(stderr, "[INFO] %s\n", qPrintable(msg));
            }
            break;
    #endif
        case QtWarningMsg:
            if (logger->syslogMode) {
                syslog(LOG_WARNING, "[WARNING] %s\n", qPrintable(msg));
            } else {
                fprintf(stderr, "[WARNING] %s\n", qPrintable(msg));
            }
            break;
        case QtCriticalMsg:
            if (logger->syslogMode) {
                syslog(LOG_CRIT, "[CRITICAL] %s\n", qPrintable(msg));
            } else {
                fprintf(stderr, "[CRITICAL] %s\n", qPrintable(msg));
            }
            break;
        case QtFatalMsg:
            if (logger->syslogMode) {
                syslog(LOG_ALERT, "[FATAL] %s\n", qPrintable(msg));
            } else {
                fprintf(stderr, "[FATAL] %s\n", qPrintable(msg));
            }
            abort();
        }
    }


    static DbdLogging *logger() {
        static DbdLogging *myInstance = NULL;

        if (!myInstance) {
            myInstance = new DbdLogging();
        }

        return myInstance;
    }

    bool debugMode;
    bool syslogMode;
};

#endif // DBDLOGGING_H

#include <signal.h>
#include <execinfo.h>
#include <ucontext.h>

#include <QDebug>
#include <QCoreApplication>

#include <backtrace.h>

#define BACKTRACE_SIZE 250

// Note : In order for this to work, -rdynamic must be passed to the linker.
//          With Qt, this can be done by adding the following line to the .pro
//          QMAKE_LFLAGS += -rdynamic

void CtrlHandler(int sig)
{
    static int handler_count = 0;

    int nptrs;
    void *buffer[BACKTRACE_SIZE];
    char **strings;

    // If the handler has been called more than twice, we need to
    // stop. This will hopefully prevent recursive segfaulting if it ever
    // occurs.
    if (handler_count++ > 2)
    {
        exit(EXIT_RECURSIVE);
    }

    switch (sig)
    {
        case SIGFPE:
        case SIGSEGV:
            break;
        case SIGQUIT:
        case SIGINT:
        case SIGHUP:
        case SIGTERM:
            qDebug() << "signal to quit received:" << sig;
            QCoreApplication::quit();
            return;
    }

    qCritical() << "Exception caused by" << strsignal(sig);

    nptrs = backtrace(buffer, BACKTRACE_SIZE);

    strings = backtrace_symbols(buffer, nptrs);
    if (strings == NULL)
    {
        qCritical() << "backtrace_symbols() failed";
        exit(EXIT_BACKTRACE_FAILURE);
    }

    // Note : The first two items in this array are most likely unimportant.
    //          The first seems to always be this signal handler. The second
    //          seems to always be an address that doesn't point to anything we
    //          have information about. We could probably skip them.
    for (int j = 0; j < nptrs; j++)
    {
        // Note : We could attempt to demangle the C++ names here.
        //          Pros : human readable function names
        //          Cons : requires more malloc'ing inside the signal handler
        //
        //          Other : c++filt can always be used on the command line.
        qCritical() << "[bt]:" << strings[j];
    }

    free(strings);

    // No GUI to do anything with so just exit
    exit(EXIT_SIGHANDLER_NORMAL);
}

int setup_unix_signal_handlers()
{
    struct sigaction sigHandler;

    sigHandler.sa_handler = CtrlHandler;
    sigemptyset(&sigHandler.sa_mask);
    sigHandler.sa_flags = 0;
    sigHandler.sa_flags |= SA_RESTART;

    if (sigaction(SIGFPE, &sigHandler, NULL) > 0) {
        return 1;
    }
    if (sigaction(SIGSEGV, &sigHandler, NULL) > 0) {
        return 2;
    }
    if (sigaction(SIGQUIT, &sigHandler, NULL) > 0) {
        return 3;
    }
    if (sigaction(SIGINT, &sigHandler, NULL) > 0) {
        return 4;
    }
    if (sigaction(SIGTERM, &sigHandler, NULL) > 0) {
        return 5;
    }
    if (sigaction(SIGHUP, &sigHandler, NULL) > 0) {
        return 6;
    }

    return 0;
}

void test_signal_handler_fpe(void)
{
    int j = 0;
    int i = 3 / j;
    qDebug() << i;
}

void test_signal_handler_segv(void)
{
    int *j = NULL;
    int i = 3 / *j;
    qDebug() << i;
}

// int main(int argc, char *argv[])
// {
//     // The following code sets up a signal handler designed to catch attempts
//     // to kill this application as well as unexpected behavior that would
//     // cause the application to crash.
//     if (setup_unix_signal_handlers())
//     {
//         exit(EXIT_SETUP_FAILURE);
//     }
// }

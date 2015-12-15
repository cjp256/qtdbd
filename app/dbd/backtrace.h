#if !defined __BACKTRACE_H__
#define __BACKTRACE_H__

//
// Application exit status :
//      1 - failure to setup signal handler
//      2 - backtrace failed
//      3 - exit occurred as a normal result of tripping the signal handler (i.e. SIGFPE or SIGSEGV)
//      8 - recursive segfaulting detected
//

#define EXIT_SETUP_FAILURE     1
#define EXIT_BACKTRACE_FAILURE 2
#define EXIT_SIGHANDLER_NORMAL 3
#define EXIT_RECURSIVE         8

int setup_unix_signal_handlers(void);
void test_signal_handler_fpe(void);
void test_signal_handler_segv(void);

#endif // __BACKTRACE_H__

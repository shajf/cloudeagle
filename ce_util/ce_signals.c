/*
 * =====================================================================================
 *
 *       Filename:  ce_signals.c
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  07/01/2013 13:56:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  CE,2013,CE TEAM
 *
 * =====================================================================================
 */

#include "ce_thread_proc.h"
#include "ce_palloc.h"
#include "ce_signal.h"
#include "ce_string.h"
#include <assert.h>
#define ce_sigwait(a,b) sigwait((a),(b))

ce_int_t
ce_proc_kill(ce_proc_t *proc, int signum)
{

    if (kill(proc->pid, signum) == -1) {
        return errno;
    }

    return CE_OK;
}


/*
 * Replace standard signal() with the more reliable sigaction equivalent
 * from W. Richard Stevens' "Advanced Programming in the UNIX Environment"
 * (the version that does not automatically restart system calls).
 */
ce_sigfunc_t * ce_signal(int signo, ce_sigfunc_t * func)
{
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;

	if (sigaction(signo, &act, &oact) < 0)
		return SIG_ERR;
	return oact.sa_handler;
}


/* we need to roll our own signal description stuff */

#if defined(NSIG)
#define CE_NUMSIG NSIG
#elif defined(_NSIG)
#define CE_NUMSIG _NSIG
#elif defined(__NSIG)
#define CE_NUMSIG __NSIG
#else
#define CE_NUMSIG 33   /* breace on OS/390 with < 33; 32 is o.k. for most */
#endif

static const char *signal_description[CE_NUMSIG];

#define store_desc(index, string) \
        do { \
            if (index >= CE_NUMSIG) { \
                assert(index < CE_NUMSIG); \
            } \
            else { \
                signal_description[index] = string; \
            } \
        } while (0)

void ce_signal_init(ce_pool_t *pglobal)
{
    int sig;

    store_desc(0, "Signal 0");

#ifdef SIGHUP
    store_desc(SIGHUP, "Hangup");
#endif
#ifdef SIGINT
    store_desc(SIGINT, "Interrupt");
#endif
#ifdef SIGQUIT
    store_desc(SIGQUIT, "Quit");
#endif
#ifdef SIGILL
    store_desc(SIGILL, "Illegal instruction");
#endif
#ifdef SIGTRAP
    store_desc(SIGTRAP, "Trace/BPT trap");
#endif
#ifdef SIGIOT
    store_desc(SIGIOT, "IOT instruction");
#endif
#ifdef SIGABRT
    store_desc(SIGABRT, "Abort");
#endif
#ifdef SIGEMT
    store_desc(SIGEMT, "Emulator trap");
#endif
#ifdef SIGFPE
    store_desc(SIGFPE, "Arithmetic exception");
#endif
#ifdef SIGKILL
    store_desc(SIGKILL, "Killed");
#endif
#ifdef SIGBUS
    store_desc(SIGBUS, "Bus error");
#endif
#ifdef SIGSEGV
    store_desc(SIGSEGV, "Segmecetion fault");
#endif
#ifdef SIGSYS
    store_desc(SIGSYS, "Bad system call");
#endif
#ifdef SIGPIPE
    store_desc(SIGPIPE, "Broken pipe");
#endif
#ifdef SIGALRM
    store_desc(SIGALRM, "Alarm clock");
#endif
#ifdef SIGTERM
    store_desc(SIGTERM, "Terminated");
#endif
#ifdef SIGUSR1
    store_desc(SIGUSR1, "User defined signal 1");
#endif
#ifdef SIGUSR2
    store_desc(SIGUSR2, "User defined signal 2");
#endif
#ifdef SIGCLD
    store_desc(SIGCLD, "Child status change");
#endif
#ifdef SIGCHLD
    store_desc(SIGCHLD, "Child status change");
#endif
#ifdef SIGPWR
    store_desc(SIGPWR, "Power-fail restart");
#endif
#ifdef SIGWINCH
    store_desc(SIGWINCH, "Window changed");
#endif
#ifdef SIGURG
    store_desc(SIGURG, "urgent socket condition");
#endif
#ifdef SIGPOLL
    store_desc(SIGPOLL, "Pollable event occurred");
#endif
#ifdef SIGIO
    store_desc(SIGIO, "socket I/O possible");
#endif
#ifdef SIGSTOP
    store_desc(SIGSTOP, "Stopped (signal)");
#endif
#ifdef SIGTSTP
    store_desc(SIGTSTP, "Stopped");
#endif
#ifdef SIGCONT
    store_desc(SIGCONT, "Continued");
#endif
#ifdef SIGTTIN
    store_desc(SIGTTIN, "Stopped (tty input)");
#endif
#ifdef SIGTTOU
    store_desc(SIGTTOU, "Stopped (tty output)");
#endif
#ifdef SIGVTALRM
    store_desc(SIGVTALRM, "virtual timer expired");
#endif
#ifdef SIGPROF
    store_desc(SIGPROF, "profiling timer expired");
#endif
#ifdef SIGXCPU
    store_desc(SIGXCPU, "exceeded cpu limit");
#endif
#ifdef SIGXFSZ
    store_desc(SIGXFSZ, "exceeded file size limit");
#endif
sig=0;
pglobal=pglobal;
   #if 0
    for (sig = 0; sig < CE_NUMSIG; ++sig)
        if (signal_description[sig] == NULL)
            signal_description[sig] = sprintf((char*)pglobal, 
	    "signal #%d",sig);
	#endif
}

const char *ce_signal_description_get(int signum)
{
    return
        (signum >= 0 && signum < CE_NUMSIG)
        ? signal_description[signum]
        : "unknown signal (number)";
}





static void remove_sync_sigs(sigset_t *sig_mask)
{
#ifdef SIGABRT
    sigdelset(sig_mask, SIGABRT);
#endif
#ifdef SIGBUS
    sigdelset(sig_mask, SIGBUS);
#endif
#ifdef SIGEMT
    sigdelset(sig_mask, SIGEMT);
#endif
#ifdef SIGFPE
    sigdelset(sig_mask, SIGFPE);
#endif
#ifdef SIGILL
    sigdelset(sig_mask, SIGILL);
#endif
#ifdef SIGIOT
    sigdelset(sig_mask, SIGIOT);
#endif
#ifdef SIGPIPE
    sigdelset(sig_mask, SIGPIPE);
#endif
#ifdef SIGSEGV
    sigdelset(sig_mask, SIGSEGV);
#endif
#ifdef SIGSYS
    sigdelset(sig_mask, SIGSYS);
#endif
#ifdef SIGTRAP
    sigdelset(sig_mask, SIGTRAP);
#endif
    sigdelset(sig_mask, SIGUSR2);

}

 ce_int_t
 ce_signal_thread(int(*signal_handler)(int signum))
{
    sigset_t sig_mask;

    int (*sig_func)(int signum) = (int (*)(int))signal_handler;


    /* This thread will be the one responsible for handling signals */
    sigfillset(&sig_mask);

    /* On certain platforms, sigwait() returns EINVAL if any of various
     * unblockable signals are included in the mask.  This was first 
     * observed on AIX and Tru64.
     */
#ifdef SIGKILL
    sigdelset(&sig_mask, SIGKILL);
#endif
#ifdef SIGSTOP
    sigdelset(&sig_mask, SIGSTOP);
#endif
#ifdef SIGCONT
    sigdelset(&sig_mask, SIGCONT);
#endif
#ifdef SIGWAITING
    sigdelset(&sig_mask, SIGWAITING);
#endif

    /* no synchronous signals should be in the mask passed to sigwait() */
    remove_sync_sigs(&sig_mask);

    while (1) {

        int signal_received;

        if (ce_sigwait(&sig_mask, &signal_received) != 0)
        {
            /* handle sigwait() error here */
        }
        
        if (sig_func(signal_received) == 1) {
            return CE_OK;
        }

    }
}

 ce_int_t ce_setup_signal_thread(void)
{
    sigset_t sig_mask;
    int rv;

    /* All threads should mask out signals to be handled by
     * the thread doing sigwait().
     *
     * No thread should ever block synchronous signals.
     * See the Solaris man page for pthread_sigmask() for
     * some information.  Solaris chooses to knock out such
     * processes when a blocked synchronous signal is 
     * delivered, skipping any registered signal handler.
     * AIX doesn't call a signal handler either.  At least
     * one level of linux+glibc does call the handler even
     * when the synchronous signal is blocked.
     */
    sigfillset(&sig_mask);
    remove_sync_sigs(&sig_mask);


    if ((rv = sigprocmask(SIG_SETMASK, &sig_mask, NULL)) != 0) {
       
    }

    

    return rv;
}

 ce_int_t ce_signal_block(int signum)
{

    sigset_t sig_mask;
    int rv;

    sigemptyset(&sig_mask);

    sigaddset(&sig_mask, signum);

    if ((rv = pthread_sigmask(SIG_BLOCK, &sig_mask, NULL)) != 0) {
    }
    return CE_OK;
}

 ce_int_t
 ce_signal_unblock(int signum)
{
    sigset_t sig_mask;
    int rv;

    sigemptyset(&sig_mask);

    sigaddset(&sig_mask, signum);

    if ((rv = sigprocmask(SIG_UNBLOCK, &sig_mask, NULL)) != 0) {
       return rv;
    }
    
    return CE_OK;
}

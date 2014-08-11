/*
 * =====================================================================================
 *
 *       Filename:  ce_thread_proc.h
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  07/01/2013 13:56:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */

#ifndef CE_THREAD_PROC_H
#define CE_THREAD_PROC_H

/**
 * @file ce_thread_proc.h
 * @brief CE Thread and Process Library
 */

#include "ce_basicdefs.h"
#include "ce_file.h"
#include "ce_palloc.h"
#include "ce_errno.h"
#include "ce_time.h"
#include "ce_times.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/**
 * @defgroup ce_thread_proc Threads and Process Functions
 *
 */

typedef enum
{
	CE_SHELLCMD, /**< use the shell to invoke the program */
	CE_PROGRAM, /**< invoke the program directly, no copied env */
	CE_PROGRAM_ENV, /**< invoke the program, replicating our environment */
	CE_PROGRAM_PATH, /**< find program on PATH, use our environment */
	CE_SHELLCMD_ENV
/**< use the shell to invoke the program,
 *   replicating our environment
 */
} ce_cmdtype_e;

typedef enum
{
	CE_WAIT, /**< wait for the specified process to finish */
	CE_NOWAIT
/**< do not wait -- just see if it has finished */
} ce_wait_how_e;

/* I am specifically calling out the values so that the macros below make
 * more sense.  Yes, I know I don't need to, but I am hoping this makes what
 * I am doing more clear.  If you want to add more reasons to exit, continue
 * to use bitmasce.
 */
typedef enum
{
	CE_PROC_EXIT = 1, /**< process exited normally */
	CE_PROC_SIGNAL = 2, /**< process exited due to a signal */
	CE_PROC_SIGNAL_CORE = 4
/**< process exited and dumped a core file */
} ce_exit_why_e;

/** did we exit the process */
#define CE_PROC_CHECK_EXIT(x)        (x & CE_PROC_EXIT)
/** did we get a signal */
#define CE_PROC_CHECK_SIGNALED(x)    (x & CE_PROC_SIGNAL)
/** did we get core */
#define CE_PROC_CHECK_CORE_DUMP(x)   (x & CE_PROC_SIGNAL_CORE)

/** @see ce_procattr_io_set */
#define CE_NO_PIPE          0
/** @see ce_procattr_io_set and ce_file_pipe_create_ex */
#define CE_FULL_BLOCK       1
/** @see ce_procattr_io_set and ce_file_pipe_create_ex */
#define CE_FULL_NONBLOCK    2
/** @see ce_procattr_io_set */
#define CE_PARENT_BLOCK     3
/** @see ce_procattr_io_set */
#define CE_CHILD_BLOCK      4
/** @see ce_procattr_io_set */
#define CE_NO_FILE          8

/** @see ce_file_pipe_create_ex */
#define CE_READ_BLOCK       3
/** @see ce_file_pipe_create_ex */
#define CE_WRITE_BLOCK      4

/** @see ce_procattr_io_set 
 * @note Win32 only effective with version 1.2.12, portably introduced in 1.3.0
 */
#define CE_NO_FILE          8

/** @see ce_procattr_limit_set */
#define CE_LIMIT_CPU        0
/** @see ce_procattr_limit_set */
#define CE_LIMIT_MEM        1
/** @see ce_procattr_limit_set */
#define CE_LIMIT_NPROC      2
/** @see ce_procattr_limit_set */
#define CE_LIMIT_NOFILE     3

/**
 * @defgroup CE_OC Other Child Flags
 * @{
 */
#define CE_OC_REASON_DEATH         0     /**< child has died, caller must call
                                           * unregister still */
#define CE_OC_REASON_UNWRITABLE    1     /**< write_fd is unwritable */
#define CE_OC_REASON_RESTART       2     /**< a restart is occuring, perform
                                           * any necessary cleanup (including
                                           * sending a special signal to child)
                                           */
#define CE_OC_REASON_UNREGISTER    3     /**< unregister has been called, do
                                           * whatever is necessary (including
                                           * kill the child) */
#define CE_OC_REASON_LOST          4     /**< somehow the child exited without
                                           * us knowing ... buggy os? */
#define CE_OC_REASON_RUNNING       5     /**< a health check is occuring, 
                                           * for most maiceinence functions
* this is a no-op.
*/

#define CE_DETACH 	6
#define CE_NOTDETACH  -6
/** The CE process type */
typedef struct ce_proc_t
{
	/** The process ID */
	pid_t pid;
	/** Parent's side of pipe to child's stdin */
	ce_file_t *in;
	/** Parent's side of pipe to child's stdout */
	ce_file_t *out;
	/** Parent's side of pipe to child's stdouterr */
	ce_file_t *err;

} ce_proc_t;

/**
 * The prototype for CE child errfn functions.  (See the description
 * of ce_procattr_child_errfn_set() for more information.)
 * It is passed the following parameters:
 * @param pool Pool associated with the ce_proc_t.  If your child
 *             error function needs user data, associate it with this
 *             pool.
 * @param err CE error code describing the error
 * @param description Text description of type of processing which failed
 */
typedef void ( ce_child_errfn_t)(ce_pool_t *proc, ce_int_t err,
                                  const char *description);

/** Opaque Thread structure. */
typedef struct ce_thread_s ce_thread_t;

/** Opaque Thread attributes structure. */
typedef struct ce_threadattr_s ce_threadattr_t;

/** Opaque Process attributes structure. */
typedef struct ce_procattr_s ce_procattr_t;

/** Opaque control variable for one-time atomic variables.  */
typedef struct ce_thread_once_s ce_thread_once_t;

/** Opaque thread private address space. */
typedef struct ce_threadkey_s ce_threadkey_t;

/** Opaque record of child process. */
typedef struct ce_other_child_rec_s ce_other_child_rec_t;

/**
 * The prototype for any CE thread worker functions. 
 */
#define CE_THREAD_FUNC

typedef void *(CE_THREAD_FUNC *ce_thread_start_t)\
(ce_thread_t*, void*);
typedef int (*ce_thread_exit_fun_t)(ce_thread_t*, void*);

struct ce_thread_s
{
	ce_pool_t *pool;
	pthread_t *td;
	void *data;
	ce_thread_start_t func;
	ce_thread_exit_fun_t exit_fun;
	ce_int_t exitval;
	int thread_exit_stat;
#define THD_EXIT 0
#define THD_NO_EXIT 1
#define THD_EXITED 2
};

#define CE_CHECK_THEAD_EXIT(thd)				\
	do{							\
		if(thd->thread_exit_stat==THD_EXIT)		\
		{						\
			if(thd->exit_fun)			\
				thd->exit_fun(thd,thd->data);	\
			thd->thread_exit_stat=THD_EXITED;	\
			return NULL;				\
		}						\
	}while(0)

#define CE_WAIT_THREAD_EXIT(thd) 			         \
do{                                                              \
	thd->thread_exit_stat = THD_EXIT;                        \
	while(1)						 \
	{                                                        \
		if(thd->thread_exit_stat==THD_EXITED)		 \
			break;					 \
		sleep(1);                                        \
	};							 \
}while(0)

struct ce_threadattr_s
{
	ce_pool_t *pool;
	pthread_attr_t attr;
};

struct ce_threadkey_s
{
	ce_pool_t *pool;
	pthread_key_t key;
};

struct ce_thread_once_s
{
	pthread_once_t once;
};

struct ce_procatt_s
{
	ce_pool_t *pool;
	ce_file_t *parent_in;
	ce_file_t *child_in;
	ce_file_t *parent_out;
	ce_file_t *child_out;
	ce_file_t *parent_err;
	ce_file_t **child_err;
	u_char *currdir;
	ce_int32_t cmdtype;
	ce_int32_t detached;
	struct rlimit *limit_cpu;
	struct rlimit *limit_mem;
	struct rlimit *limit_nproc;
	struct rlimit *limit_nofile;
	ce_child_errfn_t *errfn;
	ce_int32_t errchk;
	int uid;
	int gid;
};

typedef enum
{
	CE_KILL_NEVER, /**< process is never sent any signals */
	CE_KILL_ALWAYS, /**< process is sent SIGKILL on ce_pool_t cleanup */
	CE_KILL_AFTER_TIMEOUT, /**< SIGTERM, wait 3 seconds, SIGKILL */
	CE_JUST_WAIT, /**< wait forever for the process to complete */
	CE_KILL_ONLY_ONCE
/**< send SIGTERM and then wait */
} ce_kill_conditions_e;

/* Thread Function definitions */

/**
 * Create and initialize a new threadattr variable
 * @param new_attr The newly created threadattr.
 * @param cont The pool to use
 */
extern ce_int_t ce_threadattr_create(ce_threadattr_t **new_attr,
                                       ce_pool_t *cont);

/**
 * Set if newly created threads should be created in detached state.
 * @param attr The threadattr to affect 
 * @param on Non-zero if detached threads should be created.
 */
extern ce_int_t ce_threadattr_detach_set(ce_threadattr_t *attr,
                                           ce_int32_t on);

/**
 * Get the detach state for this threadattr.
 * @param attr The threadattr to reference
 * @return CE_DETACH if threads are to be detached, or CE_NOTDETACH
 * if threads are to be joinable. 
 */
extern ce_int_t ce_threadattr_detach_get(ce_threadattr_t *attr);

/**
 * Set the stack size of newly created threads.
 * @param attr The threadattr to affect 
 * @param stacceize The stack size in bytes
 */
extern ce_int_t ce_threadattr_stacceize_set(ce_threadattr_t *attr,
                                              size_t stacceize);

/**
 * Set the stack guard area size of newly created threads.
 * @param attr The threadattr to affect 
 * @param guardsize The stack guard area size in bytes
 * @note Thread library implemecetions commonly use a "guard area"
 * after each thread's stack which is not readable or writable such that
 * stack overflows cause a segfault; this consumes e.g. 4K of memory
 * and increases memory management overhead.  Setting the guard area
 * size to zero hence trades off reliable behaviour on stack overflow
 * for performance. */
extern ce_int_t ce_threadattr_guardsize_set(ce_threadattr_t *attr,
                                              size_t guardsize);

/**
 * Create a new thread of execution
 * @param new_thread The newly created thread handle.
 * @param attr The threadattr to use to determine how to create the thread
 * @param func The function to start the new thread in
 * @param data Any data to be passed to the starting function
 * @param cont The pool to use
 */
extern ce_int_t ce_thread_create(ce_thread_t **new_thread,
                                   ce_threadattr_t *attr,
                                   ce_thread_start_t func,
                                   ce_thread_exit_fun_t exit_fun, void *data,
                                   ce_pool_t *cont);

/**
 * stop the current thread
 * @param thd The thread to stop
 * @param retval The return value to pass back to any thread that cares
 */
extern ce_int_t ce_thread_exit(ce_thread_t *thd, ce_int_t retval);

/**
 * block until the desired thread stops executing.
 * @param retval The return value from the dead thread.
 * @param thd The thread to join
 */
extern ce_int_t ce_thread_join(ce_int_t *retval, ce_thread_t *thd);

/**
 * force the current thread to yield the processor
 */
extern void ce_thread_yield(void);

extern pthread_t ce_thread_current(void);

extern int ce_thread_equal(pthread_t tid1, pthread_t tid2);

extern ce_int_t ce_thread_get(pthread_t **thethd, ce_thread_t *thd);

extern ce_int_t ce_thread_put(ce_thread_t **thd, pthread_t *thethd,
                                ce_pool_t *pool);
/**
 * Initialize the control variable for ce_thread_once.  If this isn't
 * called, ce_initialize won't work.
 * @param control The control variable to initialize
 * @param p The pool to allocate data from.
 */
extern ce_int_t ce_thread_once_init(ce_thread_once_t **control,
                                      ce_pool_t *p);

/**
 * Run the specified function one time, regardless of how many threads
 * call it.
 * @param control The control variable.  The same variable should
 *                be passed in each time the function is tried to be
 *                called.  This is how the underlying functions determine
 *                if the function has ever been called before.
 * @param func The function to call.
 */
extern ce_int_t ce_thread_once(ce_thread_once_t *control, void(*func)(void));

/**
 * detach a thread
 * @param thd The thread to detach 
 */
extern ce_int_t ce_thread_detach(ce_thread_t *thd);

extern ce_int_t ce_thread_cancel(ce_thread_t *thd);
/**
 * Return the pool associated with the current thread.
 * @param data The user data associated with the thread.
 * @param key The key to associate with the data
 * @param thread The currently open thread.
 */
extern ce_int_t ce_thread_data_get(void **data, const char *key,
                                     ce_thread_t *thread);

/**
 * Return the pool associated with the current thread.
 * @param data The user data to associate with the thread.
 * @param key The key to use for associating the data with the thread
 * @param cleanup The cleanup routine to use when the thread is destroyed.
 * @param thread The currently open thread.
 */
extern ce_int_t ce_thread_data_set(void *data, const char *key,
                                     ce_int_t(*cleanup)(void *),
                                     ce_thread_t *thread);

/**
 * Create and initialize a new thread private address space
 * @param key The thread private handle.
 * @param dest The destructor to use when freeing the private memory.
 * @param cont The pool to use
 */
extern ce_int_t ce_threadkey_private_create(ce_threadkey_t **key,
                                              void(*dest)(void *),
                                              ce_pool_t *cont);

/**
 * Get a pointer to the thread private memory
 * @param new_mem The data stored in private memory 
 * @param key The handle for the desired thread private memory 
 */
extern ce_int_t
        ce_threadkey_private_get(void **new_mem, ce_threadkey_t *key);

/**
 * Set the data to be stored in thread private memory
 * @param priv The data to be stored in private memory 
 * @param key The handle for the desired thread private memory 
 */
extern ce_int_t ce_threadkey_private_set(void *priv, ce_threadkey_t *key);

/**
 * Free the thread private memory
 * @param key The handle for the desired thread private memory 
 */
extern ce_int_t ce_threadkey_private_delete(ce_threadkey_t *key);

/**
 * Return the pool associated with the current threadkey.
 * @param data The user data associated with the threadkey.
 * @param key The key associated with the data
 * @param threadkey The currently open threadkey.
 */
extern ce_int_t ce_threadkey_data_get(void **data, const char *key,
                                        ce_threadkey_t *threadkey);

/**
 * Return the pool associated with the current threadkey.
 * @param data The data to set.
 * @param key The key to associate with the data.
 * @param cleanup The cleanup routine to use when the file is destroyed.
 * @param threadkey The currently open threadkey.
 */
extern ce_int_t ce_threadkey_data_set(void *data, const char *key,
                                        ce_int_t(*cleanup)(void *),
                                        ce_threadkey_t *threadkey);

/**
 * Create and initialize a new procattr variable
 * @param new_attr The newly created procattr. 
 * @param cont The pool to use
 */
extern ce_int_t ce_procattr_create(ce_procattr_t **new_attr,
                                     ce_pool_t *cont);

/**
 * Determine if any of stdin, stdout, or stderr should be linked to pipes 
 * when starting a child process.
 * @param attr The procattr we care about. 
 * @param in Should stdin be a pipe back to the parent?
 * @param out Should stdout be a pipe back to the parent?
 * @param err Should stderr be a pipe back to the parent?
 * @note If CE_NO_PIPE, there will be no special channel, the child
 * inherits the parent's corresponding stdio stream.  If CE_NO_FILE is 
 * specified, that corresponding stream is closed in the child (and will
 * be INVALID_HANDLE_VALUE when inspected on Win32). This can have ugly 
 * side effects, as the next file opened in the child on Unix will fall
 * into the stdio stream fd slot!
 */
extern ce_int_t ce_procattr_io_set(ce_procattr_t *attr, ce_int32_t in,
                                     ce_int32_t out, ce_int32_t err);

/**
 * Set the child_in and/or parent_in values to existing ce_file_t values.
 * @param attr The procattr we care about. 
 * @param child_in ce_file_t value to use as child_in. Must be a valid file.
 * @param parent_in ce_file_t value to use as parent_in. Must be a valid file.
 * @remark  This is NOT a required initializer function. This is
 *          useful if you have already opened a pipe (or multiple files)
 *          that you wish to use, perhaps persistently across multiple
 *          process invocations - such as a log file. You can save some 
 *          extra function calls by not creating your own pipe since this
 *          creates one in the process space for you.
 * @bug Note that calling this function with two NULL files on some platforms
 * creates an CE_FULL_BLOCK pipe, but this behavior is neither portable nor
 * is it supported.  @see ce_procattr_io_set instead for simple pipes.
 */
extern ce_int_t ce_procattr_child_in_set(ce_procattr_t *attr,
                                           ce_file_t *child_in,
                                           ce_file_t *parent_in);

/**
 * Set the child_out and parent_out values to existing ce_file_t values.
 * @param attr The procattr we care about. 
 * @param child_out ce_file_t value to use as child_out. Must be a valid file.
 * @param parent_out ce_file_t value to use as parent_out. Must be a valid file.
 * @remark This is NOT a required initializer function. This is
 *         useful if you have already opened a pipe (or multiple files)
 *         that you wish to use, perhaps persistently across multiple
 *         process invocations - such as a log file. 
 * @bug Note that calling this function with two NULL files on some platforms
 * creates an CE_FULL_BLOCK pipe, but this behavior is neither portable nor
 * is it supported.  @see ce_procattr_io_set instead for simple pipes.
 */
extern ce_int_t ce_procattr_child_out_set(ce_procattr_t *attr,
                                            ce_file_t *child_out,
                                            ce_file_t *parent_out);

/**
 * Set the child_err and parent_err values to existing ce_file_t values.
 * @param attr The procattr we care about. 
 * @param child_err ce_file_t value to use as child_err. Must be a valid file.
 * @param parent_err ce_file_t value to use as parent_err. Must be a valid file.
 * @remark This is NOT a required initializer function. This is
 *         useful if you have already opened a pipe (or multiple files)
 *         that you wish to use, perhaps persistently across multiple
 *         process invocations - such as a log file. 
 * @bug Note that calling this function with two NULL files on some platforms
 * creates an CE_FULL_BLOCK pipe, but this behavior is neither portable nor
 * is it supported.  @see ce_procattr_io_set instead for simple pipes.
 */
extern ce_int_t ce_procattr_child_err_set(ce_procattr_t *attr,
                                            ce_file_t *child_err,
                                            ce_file_t *parent_err);

/**
 * Set which directory the child process should start executing in.
 * @param attr The procattr we care about. 
 * @param dir Which dir to start in.  By default, this is the same dir as
 *            the parent currently resides in, when the createprocess call
 *            is made. 
 */
extern ce_int_t ce_procattr_dir_set(ce_procattr_t *attr, const char *dir);

/**
 * Set what type of command the child process will call.
 * @param attr The procattr we care about. 
 * @param cmd The type of command.  One of:
 * <PRE>
 *            CE_SHELLCMD     --  Anything that the shell can handle
 *            CE_PROGRAM      --  Executable program   (default) 
 *            CE_PROGRAM_ENV  --  Executable program, copy environment
 *            CE_PROGRAM_PATH --  Executable program on PATH, copy env
 * </PRE>
 */
extern ce_int_t ce_procattr_cmdtype_set(ce_procattr_t *attr,
                                          ce_cmdtype_e cmd);

/**
 * Determine if the child should start in detached state.
 * @param attr The procattr we care about. 
 * @param detach Should the child start in detached state?  Default is no. 
 */
extern ce_int_t ce_procattr_detach_set(ce_procattr_t *attr,
                                         ce_int32_t detach);

/**
 * Set the Resource Utilization limits when starting a new process.
 * @param attr The procattr we care about. 
 * @param what Which limit to set, one of:
 * <PRE>
 *                 CE_LIMIT_CPU
 *                 CE_LIMIT_MEM
 *                 CE_LIMIT_NPROC
 *                 CE_LIMIT_NOFILE
 * </PRE>
 * @param limit Value to set the limit to.
 */
extern ce_int_t ce_procattr_limit_set(ce_procattr_t *attr, ce_int32_t what,
                                        struct rlimit *limit);

/**
 * Specify an error function to be called in the child process if CE
 * encounters an error in the child prior to running the specified program.
 * @param attr The procattr describing the child process to be created.
 * @param errfn The function to call in the child process.
 * @remark At the present time, it will only be called from ce_proc_create()
 *         on platforms where fork() is used.  It will never be called on other
 *         platforms, on those platforms ce_proc_create() will return the error
 *         in the parent process rather than invoke the callback in the now-forked
 *         child process.
 */
extern ce_int_t ce_procattr_child_errfn_set(ce_procattr_t *attr,
                                              ce_child_errfn_t *errfn);

/**
 * Specify that ce_proc_create() should do whatever it can to report
 * failures to the caller of ce_proc_create(), rather than find out in
 * the child.
 * @param attr The procattr describing the child process to be created.
 * @param chk Flag to indicate whether or not extra work should be done
 *            to try to report failures to the caller.
 * @remark This flag only affects ce_proc_create() on platforms where
 *         fork() is used.  This leads to extra overhead in the calling
 *         process, but that may help the application handle such
 *         errors more gracefully.
 */
extern ce_int_t ce_procattr_error_check_set(ce_procattr_t *attr,
                                              ce_int32_t chk);

/**
 * Determine if the child should start in its own address space or using the 
 * current one from its parent
 * @param attr The procattr we care about. 
 * @param addrspace Should the child start in its own address space?  Default
 *                  is no on NetWare and yes on other platforms.
 */
extern ce_int_t ce_procattr_addrspace_set(ce_procattr_t *attr,
                                            ce_int32_t addrspace);

/**
 * Set the username used for running process
 * @param attr The procattr we care about. 
 * @param username The username used
 * @param password User password if needed. Password is needed on WIN32
 *                 or any other platform having
 *                 CE_PROCATTR_USER_SET_REQUIRES_PASSWORD set.
 */
extern ce_int_t ce_procattr_user_set(ce_procattr_t *attr,
                                       const char *username,
                                       const char *password);

/**
 * Set the group used for running process
 * @param attr The procattr we care about. 
 * @param groupname The group name  used
 */
extern ce_int_t ce_procattr_group_set(ce_procattr_t *attr,
                                        const char *groupname);

/**
 * This is currently the only non-portable call in CE.  This executes 
 * a standard unix fork.
 * @param proc The resulting process handle. 
 * @param cont The pool to use. 
 * @remark returns CE_INCHILD for the child, and CE_INPARENT for the parent
 * or an error.
 */
extern ce_int_t ce_proc_fork(ce_proc_t *proc, ce_pool_t *cont);

/**
 * Create a new process and execute a new program within that process.
 * @param new_proc The resulting process handle.
 * @param progname The program to run 
 * @param args the arguments to pass to the new program.  The first 
 *             one should be the program name.
 * @param env The new environment table for the new process.  This 
 *            should be a list of NULL-terminated strings. This argument
 *            is ignored for CE_PROGRAM_ENV, CE_PROGRAM_PATH, and
 *            CE_SHELLCMD_ENV types of commands.
 * @param attr the procattr we should use to determine how to create the new
 *         process
 * @param pool The pool to use.
 * @note This function returns without waiting for the new process to terminate;
 * use ce_proc_wait for that.
 */
extern ce_int_t ce_proc_create(ce_proc_t *new_proc, const char *progname,
                                 const char * const *args,
                                 const char * const *env, ce_procattr_t *attr,
                                 ce_pool_t *pool);

/**
 * Wait for a child process to die
 * @param proc The process handle that corresponds to the desired child process 
 * @param exitcode The returned exit status of the child, if a child process 
 *                 dies, or the signal that caused the child to die.
 *                 On platforms that don't support obtaining this information, 
 *                 the status parameter will be returned as CE_ENOTIMPL.
 * @param exitwhy Why the child died, the bitwise or of:
 * <PRE>
 *            CE_PROC_EXIT         -- process terminated normally
 *            CE_PROC_SIGNAL       -- process was killed by a signal
 *            CE_PROC_SIGNAL_CORE  -- process was killed by a signal, and
 *                                     generated a core dump.
 * </PRE>
 * @param waithow How should we wait.  One of:
 * <PRE>
 *            CE_WAIT   -- block until the child process dies.
 *            CE_NOWAIT -- return immediately regardless of if the 
 *                          child is dead or not.
 * </PRE>
 * @remark The childs status is in the return code to this process.  It is one of:
 * <PRE>
 *            CE_CHILD_DONE     -- child is no longer running.
 *            CE_CHILD_NOTDONE  -- child is still running.
 * </PRE>
 */
extern ce_int_t ce_proc_wait(ce_proc_t *proc, int *exitcode,
                               ce_exit_why_e *exitwhy, ce_wait_how_e waithow);

/**
 * Wait for any current child process to die and return information 
 * about that child.
 * @param proc Pointer to NULL on entry, will be filled out with child's 
 *             information 
 * @param exitcode The returned exit status of the child, if a child process 
 *                 dies, or the signal that caused the child to die.
 *                 On platforms that don't support obtaining this information, 
 *                 the status parameter will be returned as CE_ENOTIMPL.
 * @param exitwhy Why the child died, the bitwise or of:
 * <PRE>
 *            CE_PROC_EXIT         -- process terminated normally
 *            CE_PROC_SIGNAL       -- process was killed by a signal
 *            CE_PROC_SIGNAL_CORE  -- process was killed by a signal, and
 *                                     generated a core dump.
 * </PRE>
 * @param waithow How should we wait.  One of:
 * <PRE>
 *            CE_WAIT   -- block until the child process dies.
 *            CE_NOWAIT -- return immediately regardless of if the 
 *                          child is dead or not.
 * </PRE>
 * @param p Pool to allocate child information out of.
 * @bug Passing proc as a *proc rather than **proc was an odd choice
 * for some platforms... this should be revisited in 1.0
 */
extern ce_int_t ce_proc_wait_all_procs(ce_proc_t *proc, int *exitcode,
                                         ce_exit_why_e *exitwhy,
                                         ce_wait_how_e waithow, ce_pool_t *p);

#define CE_PROC_DETACH_FOREGROUND 0    /**< Do not detach */
#define CE_PROC_DETACH_DAEMONIZE 1     /**< Detach */

/**
 * Detach the process from the controlling terminal.
 * @param daemonize set to non-zero if the process should daemonize
 *                  and become a background process, else it will
 *                  stay in the foreground.
 */
extern ce_int_t ce_proc_detach(int daemonize);

/**
 * Register an other_child -- a child associated to its registered 
 * maintence callback.  This callback is invoked when the process
 * dies, is disconnected or disappears.
 * @param proc The child process to register.
 * @param maintenance maintenance is a function that is invoked with a 
 *                    reason and the data pointer passed here.
 * @param data Opaque context data passed to the maintenance function.
 * @param write_fd An fd that is probed for writing.  If it is ever unwritable
 *                 then the maintenance is invoked with reason 
 *                 OC_REASON_UNWRITABLE.
 * @param p The pool to use for allocating memory.
 * @bug write_fd duplicates the proc->out stream, it's really redundant
 * and should be replaced in the CE 1.0 API with a bitflag of which
 * proc->in/out/err handles should be health checked.
 * @bug no platform currently tests the pipes health.
 */
extern void
        ce_proc_other_child_register(ce_proc_t *proc,
                                      void(*maintenance)(int reason, void *,
                                                         int status),
                                      void *data, ce_file_t *write_fd,
                                      ce_pool_t *p);

/**
 * Stop watching the specified other child.  
 * @param data The data to pass to the maintenance function.  This is
 *             used to find the process to unregister.
 * @warning Since this can be called by a maintenance function while we're
 *          scanning the other_children list, all scanners should protect 
 *          themself by loading ocr->next before calling any maintenance 
 *          function.
 */
extern void ce_proc_other_child_unregister(void *data);

/**
 * Notify the maintenance callback of a registered other child process
 * that application has detected an event, such as death.
 * @param proc The process to check
 * @param reason The reason code to pass to the maintenance function
 * @param status The status to pass to the maintenance function
 * @remark An example of code using this behavior;
 * <pre>
 * rv = ce_proc_wait_all_procs(&proc, &exitcode, &status, CE_WAIT, p);
 * if (CE_STATUS_IS_CHILD_DONE(rv)) {
 * \#if CE_HAS_OTHER_CHILD
 *     if (ce_proc_other_child_alert(&proc, CE_OC_REASON_DEATH, status)
 *             == CE_SUCCESS) {
 *         ;  (already handled)
 *     }
 *     else
 * \#endif
 *         [... handling non-otherchild processes death ...]
 * </pre>
 */
extern ce_int_t ce_proc_other_child_alert(ce_proc_t *proc, int reason,
                                            int status);

/**
 * Test one specific other child processes and invoke the maintenance callback 
 * with the appropriate reason code, if still running, or the appropriate reason 
 * code if the process is no longer healthy.
 * @param ocr The registered other child
 * @param reason The reason code (e.g. CE_OC_REASON_RESTART) if still running
 */
extern void
        ce_proc_other_child_refresh(ce_other_child_rec_t *ocr, int reason);

/**
 * Test all registered other child processes and invoke the maintenance callback 
 * with the appropriate reason code, if still running, or the appropriate reason 
 * code if the process is no longer healthy.
 * @param reason The reason code (e.g. CE_OC_REASON_RESTART) to running processes
 */
extern void ce_proc_other_child_refresh_all(int reason);

/** 
 * Terminate a process.
 * @param proc The process to terminate.
 * @param sig How to kill the process.
 */
extern ce_int_t ce_proc_kill(ce_proc_t *proc, int sig);

/**
 * Register a process to be killed when a pool dies.
 * @param a The pool to use to define the processes lifetime 
 * @param proc The process to register
 * @param how How to kill the process, one of:
 * <PRE>
 *         CE_KILL_NEVER         -- process is never sent any signals
 *         CE_KILL_ALWAYS        -- process is sent SIGKILL on ce_pool_t cleanup
 *         CE_KILL_AFTER_TIMEOUT -- SIGTERM, wait 3 seconds, SIGKILL
 *         CE_JUST_WAIT          -- wait forever for the process to complete
 *         CE_KILL_ONLY_ONCE     -- send SIGTERM and then wait
 * </PRE>
 */
extern void ce_pool_note_subprocess(ce_pool_t *a, ce_proc_t *proc,
                                     ce_kill_conditions_e how);

/**
 * Setup the process for a single thread to be used for all signal handling.
 * @warning This must be called before any threads are created
 */
extern ce_int_t ce_setup_signal_thread(void);

/**
 * Make the current thread listen for signals.  This thread will loop
 * forever, calling a provided function whenever it receives a signal.  That
 * functions should return 1 if the signal has been handled, 0 otherwise.
 * @param signal_handler The function to call when a signal is received
 * ce_int_t ce_signal_thread((int)(*signal_handler)(int signum))
 */
extern ce_int_t ce_signal_thread(int(*signal_handler)(int signum));
#ifdef __cplusplus
}
#endif

#endif  /* ! CE_THREAD_PROC_H */


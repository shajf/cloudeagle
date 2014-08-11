/*
 * =====================================================================================
 *
 *       Filename:  ce_thread_mutex.h
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

#ifndef CE_THREAD_MUTEX_H
#define CE_THREAD_MUTEX_H

/**
 * @file ce_thread_mutex.h
 * @brief CE Thread Mutex Routines
 */

#include "ce_basicdefs.h"
#include "ce_errno.h"
#include "ce_palloc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * @defgroup ce_thread_mutex Thread Mutex Routines
 */

/** Opaque thread-local mutex structure */
typedef struct 
{
	ce_pool_t *pool;
	pthread_mutex_t mutex;
}ce_thread_mutex_t;

#define CE_THREAD_MUTEX_DEFAULT  0x0   /**< platform-optimal lock behavior */
#define CE_THREAD_MUTEX_NESTED   0x1   /**< enable nested (recursive) locce */
#define CE_THREAD_MUTEX_UNNESTED 0x2   /**< disable nested locce */


/**
 * Create and initialize a mutex that can be used to synchronize threads.
 * @param mutex the memory address where the newly created mutex will be
 *        stored.
 * @param flags Or'ed value of:
 * <PRE>
 *           CE_THREAD_MUTEX_DEFAULT   platform-optimal lock behavior.
 *           CE_THREAD_MUTEX_NESTED    enable nested (recursive) locce.
 *           CE_THREAD_MUTEX_UNNESTED  disable nested locce (non-recursive).
 * </PRE>
 * @param pool the pool from which to allocate the mutex.
 * @warning Be cautious in using CE_THREAD_MUTEX_DEFAULT.  While this is the
 * most optimial mutex based on a given platform's performance charateristics,
 * it will behave as either a nested or an unnested lock.
 */
extern  ce_int_t 
ce_thread_mutex_create(ce_thread_mutex_t **mutex,
			  unsigned int flags,
			  ce_pool_t *pool);
/**
 * Acquire the lock for the given mutex. If the mutex is already locked,
 * the current thread will be put to sleep until the lock becomes available.
 * @param mutex the mutex on which to acquire the lock.
 */
extern  ce_int_t ce_thread_mutex_lock(ce_thread_mutex_t *mutex);

/**
 * Attempt to acquire the lock for the given mutex. If the mutex has already
 * been acquired, the call returns immediately with CE_EBUSY. Note: it
 * is important that the CE_STATUS_IS_EBUSY(s) macro be used to determine
 * if the return value was CE_EBUSY, for portability reasons.
 * @param mutex the mutex on which to attempt the lock acquiring.
 */
extern  ce_int_t ce_thread_mutex_trylock(ce_thread_mutex_t *mutex);

/**
 * Release the lock for the given mutex.
 * @param mutex the mutex from which to release the lock.
 */
extern  ce_int_t ce_thread_mutex_unlock(ce_thread_mutex_t *mutex);

/**
 * Destroy the mutex and free the memory associated with the lock.
 * @param mutex the mutex to destroy.
 */
extern  ce_int_t ce_thread_mutex_destroy(ce_thread_mutex_t *mutex);

#ifdef __cplusplus
}
#endif

#endif  /* ! CE_THREAD_MUTEX_H */

/*
 * =====================================================================================
 *
 *       Filename:  ce_thread_rwlock.h
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

#ifndef CE_THREAD_RWLOCK_H
#define CE_THREAD_RWLOCK_H

/**
 * @file ce_thread_rwlock.h
 * @brief CE Reader/Writer Lock Routines
 */

#include "ce_basicdefs.h"
#include "ce_palloc.h"
#include "ce_errno.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/**
 * @defgroup ce_thread_rwlock Reader/Writer Lock Routines
 * @ingroup CE 
 * @{
 */

/** Opaque read-write thread-safe lock. */
typedef struct
{
	ce_pool_t *pool;
	pthread_rwlock_t rwlock;
}ce_thread_rwlock_t;

/**
 * Note: The following operations have undefined results: unlocking a
 * read-write lock which is not locked in the calling thread; write
 * locking a read-write lock which is already locked by the calling
 * thread; destroying a read-write lock more than once; clearing or
 * destroying the pool from which a <b>locked</b> read-write lock is
 * allocated.
 */

/**
 * Create and initialize a read-write lock that can be used to synchronize
 * threads.
 * @param rwlock the memory address where the newly created readwrite lock
 *        will be stored.
 * @param pool the pool from which to allocate the mutex.
 */
extern  ce_int_t ce_thread_rwlock_create(ce_thread_rwlock_t **rwlock,
                                                   ce_pool_t *pool);
/**
 * Acquire a shared-read lock on the given read-write lock. This will allow
 * multiple threads to enter the same critical section while they have acquired
 * the read lock.
 * @param rwlock the read-write lock on which to acquire the shared read.
 */
extern  ce_int_t ce_thread_rwlock_rdlock(ce_thread_rwlock_t *rwlock);

/**
 * Attempt to acquire the shared-read lock on the given read-write lock. This
 * is the same as ce_thread_rwlock_rdlock(), only that the function fails
 * if there is another thread holding the write lock, or if there are any
 * write threads blocking on the lock. If the function fails for this case,
 * CE_EBUSY will be returned. Note: it is important that the
 * CE_STATUS_IS_EBUSY(s) macro be used to determine if the return value was
 * CE_EBUSY, for portability reasons.
 * @param rwlock the rwlock on which to attempt the shared read.
 */
extern  ce_int_t ce_thread_rwlock_tryrdlock(ce_thread_rwlock_t *rwlock);

/**
 * Acquire an exclusive-write lock on the given read-write lock. This will
 * allow only one single thread to enter the critical sections. If there
 * are any threads currently holding the read-lock, this thread is put to
 * sleep until it can have exclusive access to the lock.
 * @param rwlock the read-write lock on which to acquire the exclusive write.
 */
extern  ce_int_t ce_thread_rwlock_wrlock(ce_thread_rwlock_t *rwlock);

/**
 * Attempt to acquire the exclusive-write lock on the given read-write lock. 
 * This is the same as ce_thread_rwlock_wrlock(), only that the function fails
 * if there is any other thread holding the lock (for reading or writing),
 * in which case the function will return CE_EBUSY. Note: it is important
 * that the CE_STATUS_IS_EBUSY(s) macro be used to determine if the return
 * value was CE_EBUSY, for portability reasons.
 * @param rwlock the rwlock on which to attempt the exclusive write.
 */
extern  ce_int_t ce_thread_rwlock_trywrlock(ce_thread_rwlock_t *rwlock);

/**
 * Release either the read or write lock currently held by the calling thread
 * associated with the given read-write lock.
 * @param rwlock the read-write lock to be released (unlocked).
 */
extern  ce_int_t ce_thread_rwlock_unlock(ce_thread_rwlock_t *rwlock);

/**
 * Destroy the read-write lock and free the associated memory.
 * @param rwlock the rwlock to destroy.
 */
extern  ce_int_t ce_thread_rwlock_destroy(ce_thread_rwlock_t *rwlock);


#ifdef __cplusplus
}
#endif

#endif  /* ! CE_THREAD_RWLOCK_H */

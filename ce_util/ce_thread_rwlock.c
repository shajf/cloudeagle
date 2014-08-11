
/*
 * =====================================================================================
 *
 *       Filename:  ce_thread_rwlock.c
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

#include "ce_thread_rwlock.h"



/* The rwlock must be initialized but not locked by any thread when
 * cleanup is called. */
static ce_int_t thread_rwlock_cleanup(void *data)
{
    ce_thread_rwlock_t *rwlock = (ce_thread_rwlock_t *)data;
    ce_int_t st;

    st = pthread_rwlock_destroy(&rwlock->rwlock);
    return st;
} 

ce_int_t 
ce_thread_rwlock_create(ce_thread_rwlock_t **rwlock,
			ce_pool_t *pool)
{
    ce_thread_rwlock_t *new_rwlock;
    ce_int_t st;

    new_rwlock = ( ce_thread_rwlock_t *)ce_palloc(pool, sizeof(ce_thread_rwlock_t));
    new_rwlock->pool = pool;

    if ((st = pthread_rwlock_init(&new_rwlock->rwlock, NULL))) {
        return st;
    }

   // ce_pool_cleanup_register(new_rwlock->pool,
     //                         (void *)new_rwlock, thread_rwlock_cleanup,
       //                       ce_pool_cleanup_null);

    *rwlock = new_rwlock;
    return CE_OK;
}

ce_int_t 
ce_thread_rwlock_rdlock(ce_thread_rwlock_t *rwlock)
{
    ce_int_t st;

    st = pthread_rwlock_rdlock(&rwlock->rwlock);

    return st;
}

ce_int_t 
ce_thread_rwlock_tryrdlock(ce_thread_rwlock_t *rwlock)
{
    ce_int_t st;

    st = pthread_rwlock_tryrdlock(&rwlock->rwlock);

    /* Normalize the return code. */
    if (st == EBUSY)
        st = CE_EBUSY;
    return st;
}

ce_int_t 
ce_thread_rwlock_wrlock(ce_thread_rwlock_t *rwlock)
{
    ce_int_t st;

    st = pthread_rwlock_wrlock(&rwlock->rwlock);

    return st;
}

ce_int_t 
ce_thread_rwlock_trywrlock(ce_thread_rwlock_t *rwlock)
{
    ce_int_t st;

    st = pthread_rwlock_trywrlock(&rwlock->rwlock);
    /* Normalize the return code. */
    if (st == EBUSY)
        st = CE_EBUSY;
    return st;
}

ce_int_t 
ce_thread_rwlock_unlock(ce_thread_rwlock_t *rwlock)
{
    ce_int_t st;

    st = pthread_rwlock_unlock(&rwlock->rwlock);

    return st;
}

ce_int_t 
ce_thread_rwlock_destroy(ce_thread_rwlock_t *rwlock)
{
    return thread_rwlock_cleanup(rwlock);
}

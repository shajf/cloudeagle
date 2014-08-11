/*
 * =====================================================================================
 *
 *       Filename:  ce_thread_mutex.c
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

#include "ce_thread_mutex.h"

static ce_int_t 
thread_mutex_cleanup(void *data)
{
    ce_thread_mutex_t *mutex = (ce_thread_mutex_t *)data;
    ce_int_t rv;

    rv = pthread_mutex_destroy(&mutex->mutex);
    return rv;
} 

ce_int_t 
ce_thread_mutex_create(ce_thread_mutex_t **mutex,
			unsigned int flags,
			ce_pool_t *pool)
{
   flags=flags;
    ce_thread_mutex_t *new_mutex;
    ce_int_t rv;
    

    new_mutex = (ce_thread_mutex_t *)ce_pcalloc(pool, sizeof(ce_thread_mutex_t));
    new_mutex->pool = pool;

    rv = pthread_mutex_init(&new_mutex->mutex, NULL);

    if (rv) {
        return rv;
    }

   // ce_pool_cleanup_register(new_mutex->pool,
     //                         new_mutex, thread_mutex_cleanup,
       //                       ce_pool_cleanup_null);

    *mutex = new_mutex;
    return CE_OK;
}

ce_int_t 
ce_thread_mutex_lock(ce_thread_mutex_t *mutex)
{
    ce_int_t rv;

    rv = pthread_mutex_lock(&mutex->mutex);
    
    return rv;
}

ce_int_t 
ce_thread_mutex_trylock(ce_thread_mutex_t *mutex)
{
    ce_int_t rv;

    rv = pthread_mutex_trylock(&mutex->mutex);
    if (rv) {

        return (rv == EBUSY) ? CE_EBUSY : rv;
    }

    return CE_OK;
}

ce_int_t 
ce_thread_mutex_unlock(ce_thread_mutex_t *mutex)
{
    ce_int_t status;

    status = pthread_mutex_unlock(&mutex->mutex);

    return status;
}

ce_int_t 
ce_thread_mutex_destroy(ce_thread_mutex_t *mutex)
{
	return thread_mutex_cleanup(mutex);
}



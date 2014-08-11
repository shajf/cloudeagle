/*
 * =====================================================================================
 *
 *       Filename:  ce_thread_cond.c
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

#include "ce_thread_cond.h"

#include "ce_time.h"
#include "ce_times.h"

static ce_int_t 
thread_cond_cleanup(void *data)
{
    ce_thread_cond_t *cond = (ce_thread_cond_t *)data;
    ce_int_t rv;

    rv = pthread_cond_destroy(&cond->cond);


    return rv;
} 

 ce_int_t 
ce_thread_cond_create(ce_thread_cond_t **cond,
			ce_pool_t *pool)
{
    ce_thread_cond_t *new_cond;
    ce_int_t rv;

    new_cond = ( ce_thread_cond_t *)ce_palloc(pool, 
				sizeof(ce_thread_cond_t));

    new_cond->pool = pool;

    if ((rv = pthread_cond_init(&new_cond->cond, NULL))) {
        return rv;
    }

//    ce_pool_cleanup_register(new_cond->pool,
  //                            (void *)new_cond, thread_cond_cleanup,
    //                          ce_pool_cleanup_null);

    *cond = new_cond;
    return CE_OK;
}

ce_int_t
ce_thread_cond_wait(ce_thread_cond_t *cond,
			ce_thread_mutex_t *mutex)
{
    ce_int_t rv;

    rv = pthread_cond_wait(&cond->cond, &mutex->mutex);

    return rv;
}
#if 0
 ce_int_t 
ce_thread_cond_timedwait(ce_thread_cond_t *cond,
			ce_thread_mutex_t *mutex,
			time_t timeout)
{
    ce_int_t rv;
    ce_time_t then;
    struct timespec abstime;

    then = ce_time_now() + timeout;
    abstime.tv_sec = ce_time_sec(then);
    abstime.tv_nsec = ce_time_usec(then) * 1000; /* nanoseconds */

    rv = pthread_cond_timedwait(&cond->cond, &mutex->mutex, &abstime);
    
    if (ETIMEDOUT == rv) {
        return CE_TIMEUP;
    }
    return rv;
}
#endif

ce_int_t ce_thread_cond_signal(ce_thread_cond_t *cond)
{
    ce_int_t rv;

    rv = pthread_cond_signal(&cond->cond);
    return rv;
}

ce_int_t 
ce_thread_cond_broadcast(ce_thread_cond_t *cond)
{
    ce_int_t rv;

    rv = pthread_cond_broadcast(&cond->cond);
    
    return rv;
}

ce_int_t 
ce_thread_cond_destroy(ce_thread_cond_t *cond)
{
    return thread_cond_cleanup(cond);
}



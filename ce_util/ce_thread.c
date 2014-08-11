/*
 * =====================================================================================
 *
 *       Filename:  ce_thread.c
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

#include "ce_basicdefs.h"

#include "ce_thread_proc.h"
#if 0
/* Destroy the threadattr object */
static ce_int_t 
threadattr_cleanup(void *data)
{
	ce_threadattr_t *attr = data;
	
	ce_int_t rv;

	rv = pthread_attr_destroy(&attr->attr);
	
	return rv;
}
#endif
ce_int_t 
ce_threadattr_create(ce_threadattr_t **new_thread,
 		       ce_pool_t *pool)
{
	ce_int_t st;

	(*new_thread) = (ce_threadattr_t *)ce_palloc(pool, 
			     sizeof(ce_threadattr_t));
	(*new_thread)->pool = pool;
	st = pthread_attr_init(&(*new_thread)->attr);

	if (st == 0) 
	{
		return CE_OK;
	}

    return st;
}

#define DETACH_ARG(v) ((v) ? 1 : 0)

ce_int_t 
ce_threadattr_detach_set(ce_threadattr_t *attr,
			   ce_int32_t on)
{
	ce_int_t st;

	if ((st = pthread_attr_setdetachstate(&attr->attr, 
                                            DETACH_ARG(on))) == 0)
       {
		return CE_OK;
       }
	else 
	{

        return st;
        }
}

ce_int_t 
ce_threadattr_detach_get(ce_threadattr_t *attr)
{
	int state;

	pthread_attr_getdetachstate(&attr->attr, &state);
    
	if (state == 1)
		return CE_DETACH;
	
	return CE_NOTDETACH;
}

ce_int_t 
ce_threadattr_stacceize_set(ce_threadattr_t *attr,
			      size_t stacceize)
{
	int st;

	st = pthread_attr_setstacceize(&attr->attr, 
				         stacceize);
	if (st == 0) 
	{
		return CE_OK;
        }

	return st;
}

ce_int_t
ce_threadattr_guardsize_set(ce_threadattr_t *attr,
			      size_t size)
{
	ce_int_t rv;

	rv = pthread_attr_setguardsize(&attr->attr, size);
	if (rv == 0) 
	{
		return CE_OK;
        }
	return CE_ERROR;
}

static void *
dummy_worker(void *opaque)
{
	ce_thread_t *thread = (ce_thread_t*)opaque;
	return thread->func(thread, thread->data);
}

ce_int_t 
ce_thread_create(ce_thread_t **new_thread,
		   ce_threadattr_t *attr,
		   ce_thread_start_t func,
		   ce_thread_exit_fun_t exit_fun,
		   void *data,
		   ce_pool_t *pool)
{
	ce_int_t st;
	pthread_attr_t *temp;

	(*new_thread) = (ce_thread_t *)ce_pcalloc(pool, 
					      sizeof(ce_thread_t));

	if ((*new_thread) == NULL) 
	{
		return CE_ENOMEM;
        }

	(*new_thread)->td = (pthread_t *)ce_pcalloc(pool, 
					       sizeof(pthread_t));

	if ((*new_thread)->td == NULL) 
        {
		return CE_ENOMEM;
        }

	(*new_thread)->data = data;
	(*new_thread)->func = func;
	(*new_thread)->pool=pool;
	(*new_thread)->exit_fun=exit_fun;
	(*new_thread)->thread_exit_stat = THD_NO_EXIT;

	if (attr)
		temp = &attr->attr;
	else
		temp = NULL;


	if ((st = pthread_create((*new_thread)->td, 
				   temp,
				   dummy_worker, 
				   (*new_thread))) == 0) 
        {
		return CE_OK;
	}
	
	else 
	{

		return st;
        }
}

pthread_t
ce_thread_current(void)
{
	return pthread_self();
}

int
ce_thread_equal(pthread_t tid1,
		  pthread_t tid2)
{
	return pthread_equal(tid1, tid2);
}

ce_int_t
ce_thread_exit(ce_thread_t *thd,
		ce_int_t retval)
{
	thd->exitval = retval;
	ce_destroy_pool(thd->pool);
	pthread_exit(NULL);
	return CE_OK;
}

ce_int_t ce_thread_cancel(ce_thread_t *thd)
{
	thd=thd;
	return CE_OK;
}

ce_int_t 
ce_thread_join( ce_int_t *retval,
		ce_thread_t *thd)
{
	ce_int_t st;
	void *thread_stat;

	if ((st = pthread_join(*thd->td,
				&thread_stat)) == 0)
        {
		*retval = thd->exitval;
		return CE_OK;
        }
    
	else 
	{

		return st;
	}
}

ce_int_t 
ce_thread_detach(ce_thread_t *thd)
{
	ce_int_t st;

	if ((st = pthread_detach(*thd->td)) == 0) 
	{

		return CE_OK;
	}
	
	else 
	{

		return st;
	}
}

void ce_thread_yield(void)
{
    	pthread_yield();
}

ce_int_t ce_thread_data_get(void **data, 
				const char *key,
		  		ce_thread_t *thread)
{
	//return ce_pool_userdata_get(data, key, thread->pool);
	data=data;
	key=key;
	thread=thread;
	return CE_OK;
}

ce_int_t 
ce_thread_data_set(void *data, const char *key,
		     ce_int_t (*cleanup)(void *),
	       	     ce_thread_t *thread)
{
	//return ce_pool_userdata_set(data, key, cleanup, thread->pool);
	data=data;
	key=key;
	cleanup=cleanup;
	thread=thread;
	return CE_OK;
}

ce_int_t 
ce_thread_get(pthread_t **thethd,
	 	ce_thread_t *thd)
{
    *thethd = thd->td;
    return CE_OK;
}

ce_int_t 
ce_thread_put(ce_thread_t **thd,
		pthread_t *thethd,
		ce_pool_t *pool)
{
	if (pool == NULL) 
	{
		return CE_ENOPOOL;
	}

	if ((*thd) == NULL) 
	{
		(*thd) = (ce_thread_t *)ce_pcalloc(pool, 
						sizeof(ce_thread_t));
		(*thd)->pool = pool;
    	}

	(*thd)->td = thethd;
	return CE_OK;
}

ce_int_t ce_thread_once_init(ce_thread_once_t **control,
				ce_pool_t *p)
{
	static const pthread_once_t once_init = PTHREAD_ONCE_INIT;

	*control = (ce_thread_once_t *)ce_palloc(p, 
	sizeof(**control));
	(*control)->once = once_init;
	return CE_OK;
}

ce_int_t 
ce_thread_once(ce_thread_once_t *control,
		void (*func)(void))
{
	return pthread_once(&control->once, func);
}



/*
 * =====================================================================================
 *
 *       Filename:  ce_kv_filter.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/27/2012 06:10:11 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng 
 *   Organization:  CE,2012,CE PROJECT TEAM
 *
 * =====================================================================================
 */
#include "ce_kv_filter.h"

ce_kv_filter_t *
ce_create_kv_filter(ce_pool_t *pool,
		      void *condition,
		      void *private_data,
		      int or_and,
		      kv_filter_callback_fun filter_fun)
{
	ce_kv_filter_t *res=NULL;
	res=(ce_kv_filter_t *)ce_palloc(pool,
					sizeof(ce_kv_filter_t));
	if(res==NULL)
	{
		return NULL;
	}
	res->pool=pool;
	res->filter_or_and=or_and;
	res->condition=condition;
	res->private_data=private_data;
	res->filter_fun=filter_fun;
	return res;
}

int 
ce_exec_filters(struct list_head *filters,
		void *kv_entry,
		size_t kv_entry_size,
		void *kv_attr)
{
	ce_kv_filter_t *f;
	int ret=0;
	if(kv_entry==NULL||list_empty(filters))
	{
		return 0;
	}
	list_for_each_entry(f,filters,anchor)
	{
		if(f->filter_fun==NULL)
		{
			continue;
		}
		if(f->filter_or_and==FILTER_AND)
		{
			ret=f->filter_fun(kv_entry,
					 kv_entry_size,
					 kv_attr,
					 f->condition,
					 f->private_data);
			if(ret==0)
			{
				return 0;
			}
		}

		else
		{
			
			ret=f->filter_fun(kv_entry,
					 kv_entry_size,
					 kv_attr,
					 f->condition,
					 f->private_data);
			if(ret==1)
			{
				return 1;
			}
		}	
	}
	return ret;
}


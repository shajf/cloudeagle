/*
 * =====================================================================================
 *
 *       Filename:  ce_reader.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/15/2012 05:03:23 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  nsfocus,2012,ce project team
 *
 * =====================================================================================
 */
#include "ce_reader.h"
#include "ce_kv_filter.h"
ce_reader_t*
ce_create_reader(u_char *read_obj,
		   ce_pool_t *pool,	
		   ce_kviter_t *kv_iter,
		   read_proc_callback read_proc,
		   u_char *private_data)
{
	if(pool==NULL)
	{
		return NULL;
	}
	ce_reader_t *reader=(ce_reader_t *)ce_palloc(pool,
				sizeof(ce_reader_t));
	if(reader==NULL)
	{
		return NULL;
	}
	reader->pool=pool;
	reader->kv_iter=kv_iter;
	reader->private_data=private_data;
	reader->read_obj=read_obj;
	reader->read_num=0;
	reader->read_proc=read_proc;
	return reader;
}

ce_int_t
ce_run_reader(ce_reader_t *reader)
{
	if(reader==NULL||reader->kv_iter==NULL)
	{
		return CE_ERROR;
	}
	ce_kviter_t *kv_iter=reader->kv_iter;
	u_char *key;
	size_t key_size;
	ce_int_t rc=CE_OK;

	while(kv_iter->k_iter.has_next(kv_iter))
	{
		rc=kv_iter->k_iter.next(kv_iter,&key,&key_size);
		if(!list_empty(&(kv_iter->k_iter.filters))&&\
		   ce_exec_filters(&(kv_iter->k_iter.filters),
				     key,
				     key_size,
				     kv_iter->k_iter.attr))
		{
			continue;
		}
	
		if(rc!=CE_OK)
		{
			break;
		}
		rc=reader->read_proc(key,key_size,
				      kv_iter,
				      reader->private_data);
		if(rc!=CE_OK)
		{
			
			break;
		}
		if(rc==CE_EXIT)
		{
			return CE_OK;
		}
	}

	return rc;		
}

void 
ce_init_iter(ce_iter_t *iter,
	       ce_pool_t *pool,
	       u_char *cur_state,
	       u_char *end_state,
	       u_char *iter_obj,
	       u_char *attr,
	       iter_has_next_callback_fun has_next,
	       iter_next_callback_fun next)
{
	iter->pool=pool;
	iter->attr=attr;

	iter->cur_state=cur_state;
	iter->end_state=end_state;
	iter->iter_obj=iter_obj;
	iter->has_next=has_next;
	iter->next=next;
}

void
ce_register_filter(ce_reader_t *reader,
		    ce_kv_filter_t *kv_filter,
		    int k_or_v_filter)
{
	ce_iter_t *iter=(k_or_v_filter==V_FILTER)?\
			  &(reader->kv_iter->v_iter):&(reader->kv_iter->k_iter);
	
	list_add_tail(&kv_filter->anchor,&iter->filters);		
}

void 
ce_reset_reader(ce_reader_t *reader)
{
	if(reader&&reader->rcv_fun)
	{
		reader->rcv_fun(reader,
				reader->old_status,
				reader->private_data);
	}
}


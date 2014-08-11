/*
 * =====================================================================================
 *
 *       Filename:  ce_writer.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/04/2012 01:47:08 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng,shajianfeng001314@gmail.com
 *   Organization:  ,2012,CE PROJECT TEAM
 *
 * =====================================================================================
 */
#include "ce_writer.h"

static size_t 
write_proxy(ce_writer_t *writer,u_char *key,
	    size_t key_size,u_char *value,size_t value_size)
{
	if(writer->writer_ops&&writer->writer_ops->write_fun)
	{
		size_t n= writer->writer_ops->write_fun(key,key_size,
				value,value_size,writer->private_data);
		writer->writen_n+=n;
		return n;
	}

	else
	{
		return 0;
	}
}

static int 
write_done_proxy(ce_writer_t *writer)
{
	
	if(writer->writer_ops&&writer->writer_ops->write_done)
	{
		return writer->writer_ops->write_done(writer->private_data);
	}

	else
	{
		return CE_OK;
	}
}

static size_t 
flush_proxy(ce_writer_t *writer)
{
	
	if(writer->writer_ops&&writer->writer_ops->flush_fun)
	{
		return writer->writer_ops->flush_fun(writer->private_data);
	}

	else
	{
		return 0;
	}
}

ce_writer_t *ce_create_writer(ce_pool_t *pool,
					ce_writer_ops_t *w_ops,
					u_char *private_data)
{
	if(pool==NULL||w_ops==NULL)
	{
		return NULL;
	}

	ce_writer_t *writer=(ce_writer_t*)ce_palloc(pool,
							 sizeof(ce_writer_t));
	if(writer==NULL)
	{
		return NULL;
	}
	writer->pool=pool;
	writer->writer_ops=w_ops;
	writer->private_data=private_data;
	writer->writen_n=0;
	writer->write=write_proxy;
	writer->flush=flush_proxy;
	writer->write_done=write_done_proxy;
	return writer;	
}

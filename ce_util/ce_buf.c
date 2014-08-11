/**
 * @file ce_buf.c
 * @brief 
 * 
 * @author Shajianfeng
 * @modified by chenhanbing
 * @version 1
 * @date 2013-08-21
 * @Copyright 2000-2013 CE. All rights reserved.
 */


#include "ce_buf.h"
#include "ce_string.h"

ce_buf_t * ce_init_buf(u_char *mem,size_t mem_size)
{
	if(mem==NULL)
	{
		return NULL;
	}

	if(mem_size<sizeof(ce_buf_t))
	{
		return NULL;
	}

	ce_buf_t *buf=(ce_buf_t*)mem;
	buf->start=(u_char*)(buf+1);
	buf->pos=buf->start;
	buf->last=buf->start;
	buf->end=(u_char*)(((u_char*)buf)+mem_size);
	return buf;	
}

ce_buf_t *	ce_create_buf(size_t buf_size)
{
	if(buf_size<sizeof(ce_buf_t))
	{
		buf_size=sizeof(ce_buf_t);
	}
	u_char *mem=(u_char*)malloc(buf_size);
	if(mem==NULL)
	{
		return NULL;
	}
	return ce_init_buf(mem,buf_size);
}

void ce_destroy_buf(ce_buf_t *buf)
{
	if(buf)
	{
		free(buf);
	}
}

int ce_buf_full(ce_buf_t *buf,size_t size)
{
	return buf->pos+size>buf->end;
}

u_char * ce_push_buf(ce_buf_t *buf,size_t size)
{
	if(buf==NULL||size<=0||ce_buf_full(buf,size))
	{
		return NULL;
	}
	u_char *res=buf->pos;
	ce_memzero(buf->pos,size);
	buf->pos=buf->pos+size;
	return res;
}

size_t ce_buf_size(ce_buf_t *buf)
{
	return (buf->end-buf->start);
}

size_t ce_buf_content_size(ce_buf_t* buf)
{
	return (buf->pos-buf->start);
}

u_char * ce_buf_content(ce_buf_t *buf)
{
	return (buf->start);
}

void ce_reset_buf(ce_buf_t *buf)
{
	buf->pos=buf->start;
	buf->last=buf->start;
}


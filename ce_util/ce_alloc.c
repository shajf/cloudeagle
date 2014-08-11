/**
 * @file ce_alloc.c
 * @brief  内存申请函数的封装,alloc,calloc,realloc表现形式与标准c库函数一致
 * 
 * @author shajianfeng
 * @modified by Chenhanbing
 * @version 1
 * @date 2013-08-21
 * 
 * @Copyright 2000-2013 CE. All rights reserved.
 */

#include "ce_alloc.h"
#include "ce_string.h"

void* ce_alloc(size_t size)
{
    void  *p;

    p = malloc(size);
    if (p == NULL) 
    {
      return NULL;
    }
    return p;
}

void* ce_calloc(size_t size)
{
    void  *p;

    p = ce_alloc(size);

    if (p) 
    {
        ce_memzero(p, size);
    }
    return p;
}

void* ce_realloc(void* base,size_t size)
{
	void *p;
	p=realloc(base,size);

	if(p==NULL)
	{
		return NULL;
	}
	return p;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 显示申请按alignment个字节对齐的，含有size字节长度的动态内存。
 * 		  注意该操作可能会导致内存碎片，也就是导致内存浪费,单独使用需谨慎
 *
 * @param alignment 按多少字节对齐,必须是2的幂
 * @param size   动态内存的大小
 *
 * @returns  成功返回size字节的动态内存
 * 			 失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
void * ce_memalign(size_t alignment, size_t size)
{
    void  *p;
    int    err;

    err = posix_memalign(&p, alignment, size);

    if (err) 
    {
      
        p = NULL;
    }

    return p;
}




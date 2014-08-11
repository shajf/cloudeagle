/**
 * @file ce_palloc.c
 * @brief  内存池的实现，提供创建内存池，销毁内存池等一系列操作
 * 
 * @author shajianfeng
 * @modified by chenhanbing
 * @version 1
 * @date 2013-08-21
 * @Copyright 2000-2013 CE. All rights reserved.
 */

#include "ce_palloc.h"
#include "ce_alloc.h"
#include "ce_string.h"

static void *ce_palloc_block(ce_pool_t *pool, size_t size);
static void *ce_palloc_large(ce_pool_t *pool, size_t size);

/* --------------------------------------------------------------------------*/
/**
 * @brief  创建内存池，每一个内存块的大小为size
 *
 * @param size  内存池大小
 *
 * @returns  成功返回内存池指针
 * 			 失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
ce_pool_t *ce_create_pool(size_t size/*, ce_log_t *log*/)
{
    ce_pool_t  *p;

    p = (ce_pool_t *)ce_memalign(CE_POOL_ALIGNMENT, size);
    if (p == NULL) 
    {
        return NULL;
    }

    p->d.last = (u_char *) p + sizeof(ce_pool_t);  /// last指向ce_pool_t后的位置，第一次内存分配从这个位置开始
    p->d.end = (u_char *) p + size;                 /// 初始化内存池结束的位置
    p->d.next = NULL;
    p->d.failed = 0;
    
	/// 内存块最大值为一个内存页面的大小(for x86,页的大小为4096)
    size = size - sizeof(ce_pool_t);
    p->max = (size < CE_MAX_ALLOC_FROM_POOL) ? size : CE_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->large = NULL;
    p->cleanup = NULL;
   // p->log = log;
    return p;
}


/* --------------------------------------------------------------------------*/
/**
 * @brief  销毁内存池
 *
 * @param pool  指向内存池的指针
 */
/* ----------------------------------------------------------------------------*/
void ce_destroy_pool(ce_pool_t *pool)
{
    ce_pool_t          *p, *n;
    ce_pool_large_t    *l;
    ce_pool_cleanup_t  *c;

	/// 调用cleanup中的handler函数(如果有)，清除特定资源
    for (c = pool->cleanup; c; c = c->next) 
    {
        if (c->handler) 
		{
           // ce_log_debug1(CE_LOG_DEBUG_ALLOC, pool->log, 0,
             //              "run cleanup: %p", c);
            c->handler(c->data);
        }
    }
	
	/// 释放large数据块内存
    for (l = pool->large; l; l = l->next) 
    {
        if (l->alloc) 
		{
            ce_free(l->alloc);
        }
    }
	
	/// 释放整个pool
    for (p = pool, n = pool->d.next; /* void */; p = n, n = n->d.next) 
    {
        ce_free(p);

        if (n == NULL) 
		{
            break;
        }
    }
}


/* --------------------------------------------------------------------------*/
/**
 * @brief  内存池重置，该操作会删除所有的大内存块，重置所有的小内存块（只移动last指针）
 *
 * @param pool  内存池的指针
 */
/* ----------------------------------------------------------------------------*/
void ce_reset_pool(ce_pool_t *pool)
{
    ce_pool_t        *p;
    ce_pool_large_t  *l;
     
	/// 删除所有的大内存块（因为大内存块的分配只对前3个进行检查，所以大内存块需要及时删除)
    for (l = pool->large; l; l = l->next) 
    {
        if (l->alloc) 
		{
            ce_free(l->alloc);
        }
    }

    pool->large = NULL;
	
	/// 重置所有的小内存块
    for (p = pool; p; p = p->d.next) 
    {
        p->d.last = (u_char *) p + sizeof(ce_pool_t);
    }
}


/* --------------------------------------------------------------------------*/
/**
 * @brief  向内存池申请分配size字节的内存
 *
 * @param pool   指向一个内存池的指针
 * @param size   需要分配的内存长度
 *
 * @returns     成功返回已分配的内存的地址
 * 				失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
void *ce_palloc(ce_pool_t *pool, size_t size)
{
    u_char      *m;
    ce_pool_t  *p;

    if (size <= pool->max) 
    {
        p = pool->current;

        do {
            m = ce_align_ptr(p->d.last, CE_ALIGNMENT);  /// 内存地址对齐（CE_ALIGNMENT为4字节，应该为cpu二级缓存大小)
														  /// 该操作将m移动到对齐内存的地址上

            if ((size_t) (p->d.end - m) >= size)  /// 如果当前内存块有足够的空间，将该内存块上空间分出来
	   	    {
                p->d.last = m + size;
                return m;
            }
            p = p->d.next;  /// 如果当前内存块有效容量不够分配，移动到下一个内存块

        } while (p);

        return ce_palloc_block(pool, size);   /// 如果内存仍然不够，开辟新的内存块
    }

    return ce_palloc_large(pool, size);    /// 如果要求一块大内存，另外分配
}


/* --------------------------------------------------------------------------*/
/**
 * @brief  向内存池申请分配size字节大小的内存（不做对齐处理，用时间换空间）
 *
 * @param pool  指向内存池的指针
 * @param size  申请分配的大小
 *
 * @returns   	成功返回已分配内存的地址
 * 				失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
void *ce_pnalloc(ce_pool_t *pool, size_t size)
{
    u_char      *m;
    ce_pool_t  *p;

    if (size <= pool->max) 
    {
        p = pool->current;
        do 
		{
            m = p->d.last;
            if ((size_t) (p->d.end - m) >= size) 
	  	    {
                p->d.last = m + size;
                return m;
            }

            p = p->d.next;

        } while (p);

        return ce_palloc_block(pool, size);
    }

    return ce_palloc_large(pool, size);
}


/* --------------------------------------------------------------------------*/
/**
 * @brief  为pool内存池开辟新的内存块，并申请使用size大小的内存
 *
 * @param pool  内存池的指针
 * @param size  需要使用的内存大小 
 *
 * @returns   成功返回已经分配内存的地址
 * 			  失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
static void *ce_palloc_block(ce_pool_t *pool, size_t size)
{
    u_char      *m;
    size_t       psize;
    ce_pool_t  *p, *new_instance, *current;

    psize = (size_t) (pool->d.end - (u_char *) pool);  /// 计算第一块内存块的大小

    m =(u_char*)ce_memalign(CE_POOL_ALIGNMENT, psize/*, pool->log*/);  /// 分配与第一块内存块大小相同的内存块
    if (m == NULL) 
    {
        return NULL;
    }

    new_instance = (ce_pool_t *) m;

    new_instance->d.end = m + psize;  /// 同样设置该内存块的结束地址
    new_instance->d.next = NULL;
    new_instance->d.failed = 0;

    m += sizeof(ce_pool_data_t);
    m = ce_align_ptr(m, CE_ALIGNMENT);  /// 地址对齐
    new_instance->d.last = m + size;

    current = pool->current;
    
	/* 调用ce_palloc_block的条件是从current到最后的内存块都找不到合适的能分配的
	 * 该操作会给所有从current到最后一个内存块的failed都+1,表示该内存地址不能复用的次数+1
	 * 如果超过4次不能重复用该内存块，放弃在该内存块上再次申请内存，以后再也不在该内存块上申请
	 * 目的是为了提高效率，4次是经验值
	 */
    for (p = current; p->d.next; p = p->d.next) 
    {
		/// 如果某一个内存块已经4次faild，current(当前内存块)指向这儿
        if (p->d.failed++ > 4)   /// failed值只在这而被修改
		{
            current = p->d.next;
        }
    }

    p->d.next = new_instance;
    
	
    pool->current = current ? current : new_instance;

    return m;
}


/* --------------------------------------------------------------------------*/
/**
 * @brief  申请一块大内存块
 *
 * @param pool   指向内存池的指针 
 * @param size   要申请的内存的大小 
 *
 * @returns   成功返回指向已申请到的内存的地址
 * 			  失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
static void *ce_palloc_large(ce_pool_t *pool, size_t size)
{
    void               *p;
    ce_uint_t         n;
    ce_pool_large_t  *large;
	
	/// 重新申请一块大小为size的新内存
    p = ce_alloc(size /*pool->log*/);
    if (p == NULL) 
    {
        return NULL;
    }

    n = 0;
    
	/// 查找可用的large指针
    for (large = pool->large; large; large = large->next) 
    {
        if (large->alloc == NULL) 
		{
            large->alloc = p;
            return p;
        }
	/// 如果查找次数超过3,放弃查找
        if (n++ > 3) 
		{
            break;
        }
    }
	
    /// 直接分配一个large的内存块
    large =(ce_pool_large_t *)ce_palloc(pool, 
					sizeof(ce_pool_large_t));
    if (large == NULL) 
	{
        ce_free(p);
        return NULL;
    }
    
	/// 将large插入链表中，并且连接刚才申请的大小为size的内存 
    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


/* --------------------------------------------------------------------------*/
/**
 * @brief  直接申请分配一块按alignment对齐的size大小的内存块，挂在内存池的大内存链表下
 *
 * @param pool  指向内存池的指针
 * @param size  要申请的字节数
 * @param alignment   对齐方式
 *
 * @returns   成功返回已经分配的内存的地址
 * 			  失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
void *ce_pmemalign(ce_pool_t *pool, size_t size, size_t alignment)
{
    void              *p;
    ce_pool_large_t  *large;

    p = ce_memalign(alignment, size/*, pool->log*/);
    if (p == NULL) 
    {
        return NULL;
    }

    large =(ce_pool_large_t*)ce_palloc(pool, 
					sizeof(ce_pool_large_t));
    if (large == NULL)
    {
        ce_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


/* --------------------------------------------------------------------------*/
/**
 * @brief  调用该函数释放大内存(保留大内存的头部供下次分配，只释放数据部分)
 *
 * @param pool  指向内存池的指针
 * @param p   指向大内存的指针
 *
 * @returns   成功返回CE_OK(0)
 * 			  失败返回CE_ERROR(-1)
 */
/* ----------------------------------------------------------------------------*/
ce_int_t ce_pfree(ce_pool_t *pool, void *p)
{
    ce_pool_large_t  *l;

    for (l = pool->large; l; l = l->next) 
    {
        if (p == l->alloc) 
	{
           /* ce_log_debug1(CE_LOG_DEBUG_ALLOC, pool->log, 0,
                           "free: %p", l->alloc);*/
            ce_free(l->alloc);
            l->alloc = NULL;

            return CE_OK;
        }
    }

    return CE_ERROR;
}


/* --------------------------------------------------------------------------*/
/**
 * @brief  申请size的内存，并且初始化为0
 *
 * @param pool  指向内存池的指针
 * @param size  申请的内存长度
 *
 * @returns  已经分配的内存的地址   
 */
/* ----------------------------------------------------------------------------*/
void *ce_pcalloc(ce_pool_t *pool, size_t size)
{
    void *p;
    p = ce_palloc(pool, size);
    if (p) 
    {
        ce_memzero(p, size);
    }
    return p;
}


/* --------------------------------------------------------------------------*/
/**
 * @brief  申请一块size大小的内存，挂在cleanup链中，返回一个结构体，如果需要，自己
 * 		   在该结构体中注册回调函数，该回调函数会在destory内存池的时候调用
 *
 * @param p  指向内存池的指针
 * @param size  申请的内存大小
 *
 * @returns  成功返回一个ce_pool_cleanup_t的结构体
 * 			 失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
ce_pool_cleanup_t *ce_pool_cleanup_add(ce_pool_t *p, size_t size)
{
    ce_pool_cleanup_t  *c;
	
    c =(ce_pool_cleanup_t*)ce_palloc(p, sizeof(ce_pool_cleanup_t));
    if (c == NULL) 
    {
        return NULL;
    }

    if (size) 
    {
        c->data = ce_palloc(p, size);  /// 申请size大小的内存，挂在结构体的data上
        if (c->data == NULL) 
		{
            return NULL;
        }

    } 
    
    else 
    {
        c->data = NULL;
    }

    c->handler = NULL;
    c->next = p->cleanup;

    p->cleanup = c;

   // ce_log_debug1(CE_LOG_DEBUG_ALLOC, p->log, 0, "add cleanup: %p", c);

    return c;
}

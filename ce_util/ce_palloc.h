/*
 * =====================================================================================
 *
 *       Filename:  ce_palloc.h
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

#ifndef _CE_PALLOC_H
#define _CE_PALLOC_H

#include "ce_basicdefs.h"

/*
 * CE_MAX_ALLOC_FROM_POOL should be (ce_pagesize - 1), i.e. 4095 on x86.
 *
 */

#define CE_MAX_ALLOC_FROM_POOL  (CE_PAGE_SIZE - 1)

#define CE_DEFAULT_POOL_SIZE    (16 * 1024)

#define CE_POOL_ALIGNMENT       16
#define CE_MIN_POOL_SIZE                                                     \
    ce_align((sizeof(ce_pool_t) + 2 * sizeof(ce_pool_large_t)),            \
              CE_POOL_ALIGNMENT)


typedef void (*ce_pool_cleanup_pt)(void *data);

typedef struct ce_pool_cleanup_s  ce_pool_cleanup_t;


struct ce_pool_cleanup_s 
{
	ce_pool_cleanup_pt   handler;
	ce_pool_cleanup_t   *next;
	void *data;
};


typedef struct ce_pool_large_s  ce_pool_large_t;

struct ce_pool_large_s 
{
	ce_pool_large_t     *next;
	void	*alloc;
};


typedef struct 
{
    u_char               *last;  /// 当前内存池分配的末位置，即下一次分配开始的位置
    u_char               *end;   /// 内存池结束位置
    ce_pool_t       *next;      /// 指向下一个内存块
    ce_uint_t        failed;    /// 内存池分配失败次数
} ce_pool_data_t;

struct ce_pool_t
{
    ce_pool_data_t      d;        /// 内存池的数据块
    size_t                   max;  /// 内存池数据块的最大值 
    ce_pool_t           *current; /// 指向当前内存池
    ce_pool_large_t     *large;   /// 大块内存链表，分配空间超过max时使用
    ce_pool_cleanup_t   *cleanup; /// 释放内存池的callback 
};

#ifdef __cplusplus
extern "c" {
#endif

extern void *ce_alloc(size_t size/*, ce_log_t *log*/);

extern void *ce_calloc(size_t size/*, ce_log_t *log*/);

extern ce_pool_t *ce_create_pool(size_t size/*, ce_log_t *log*/);

extern void ce_destroy_pool(ce_pool_t *pool);

extern void ce_reset_pool(ce_pool_t *pool);

extern void *ce_palloc(ce_pool_t *pool, size_t size);

extern void *ce_pnalloc(ce_pool_t *pool, size_t size);

extern void *ce_pcalloc(ce_pool_t *pool, size_t size);

extern void *ce_pmemalign(ce_pool_t *pool, size_t size, size_t alignment);

extern ce_int_t ce_pfree(ce_pool_t *pool, void *p);

extern ce_pool_cleanup_t *ce_pool_cleanup_add(ce_pool_t *p, size_t size);

#ifdef __cplusplus
}
#endif

#endif /* _CE_PALLOC_H*/


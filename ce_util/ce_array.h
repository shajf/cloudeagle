/*
 * =====================================================================================
 *
 *       Filename:  ce_array.h
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

#ifndef _CE_ARRAY_H
#define _CE_ARRAY_H
#include "ce_basicdefs.h"
#include "ce_palloc.h"

typedef struct
{
	u_char 	         *elts;   /// 数组元素保存空间
	ce_uint_t      nelts;    /// 已使用的元素个数
	size_t		 size;		  /// 每一个元素的长度
	ce_uint_t	 nalloc;      /// 已分配的元素个数
ce_pool_t	*pool;
}ce_array_t;

#ifdef __cplusplus
extern "c" {
#endif
extern void
create_array_core(ce_array_t *arr,
		 ce_pool_t *pool,
		 ce_uint_t nelts,
		 size_t elt_size,
		 ce_int_t clear);

extern ce_array_t *
ce_array_create(ce_pool_t *pool,
		  ce_uint_t n,
		  size_t size);

extern void 
ce_array_destroy(ce_array_t *arr);

extern u_char *
ce_array_push(ce_array_t *arr);

extern u_char *
ce_array_push_noclear(ce_array_t *arr);

extern u_char *
ce_array_push_n(ce_array_t *arr,ce_uint_t n);

extern ce_int_t 
ce_is_empty_array(ce_array_t *arr);

extern void 
ce_array_clear(ce_array_t *arr);

extern u_char *
ce_array_pop(ce_array_t *arr);

extern u_char *
ce_array_get(ce_array_t *arr,
		size_t index);
extern void 
ce_array_cat(ce_array_t *dst,
	       ce_array_t *src);

extern ce_array_t *
ce_array_copy(ce_pool_t *pool,
		ce_array_t *arr);

extern ce_array_t *
ce_array_copy_hdr(ce_pool_t *pool,
	            ce_array_t *arr);

extern ce_array_t *
ce_array_append(ce_pool_t *pool,
		  ce_array_t *first,
		  ce_array_t *second);


extern char* 
ce_array_pstrcat(ce_pool_t *pool,
		   ce_array_t *arr,
		   char sep);
extern void copy_array_hdr_core(ce_array_t *res,ce_array_t *arr);

#ifdef __cplusplus
}
#endif

#endif /*_CE_ARRAY_H*/

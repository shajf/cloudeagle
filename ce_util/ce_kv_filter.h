/*
 * =====================================================================================
 *
 *       Filename:  ce_kv_filter.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/27/2012 05:51:05 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2012,CE PROJECT TEAM
 *
 * =====================================================================================
 */
#ifndef CE_KV_FILTER_H
#define CE_KV_FILTER_H

#include "ce_basicdefs.h"
#include "ce_palloc.h"
#include "ce_list.h"

#define FILTER_OR	1
#define FILTER_AND	2

#define K_FILTER	1
#define V_FILTER	2

typedef struct ce_kv_filter_t ce_kv_filter_t;

typedef int (*kv_filter_callback_fun)(void *kv_entry,
				      size_t kv_entry_size,
				      void *kv_attr,
				      void *condition,
				      void *private_data);
struct ce_kv_filter_t
{
	struct list_head anchor;
	ce_pool_t *pool;
	kv_filter_callback_fun filter_fun;
	void *condition;
	void *private_data;
	int filter_or_and;
};

#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/
extern ce_kv_filter_t *
ce_create_kv_filter(ce_pool_t *pool,
		      void *condition,
		      void *private_data,
		      int or_and,
		      kv_filter_callback_fun filter_fun);

extern int 
ce_exec_filters(struct list_head *filters,
		void *kv_entry,
		size_t kv_entry_size,
		void *kv_attr);
#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_KV_FILTER_H*/

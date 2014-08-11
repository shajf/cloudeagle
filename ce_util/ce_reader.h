/*
 * =====================================================================================
 *
 *       Filename:  ce_reader.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/15/2012 02:17:22 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng 
 *   Organization:  nsfocus,2012,ce project team
 *
 * =====================================================================================
 */
#ifndef CE_READER_H
#define CE_READER_H

#include "ce_basicdefs.h"
#include "ce_palloc.h"
#include "ce_string.h"
#include "ce_list.h"
#include "ce_kv_filter.h"

#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/

typedef struct ce_reader_t ce_reader_t;
typedef struct ce_kviter_t ce_kviter_t;
typedef struct ce_iter_t	ce_iter_t;

typedef int (*iter_has_next_callback_fun)(ce_kviter_t *iter);
typedef ce_int_t (*iter_next_callback_fun)(ce_kviter_t *iter,u_char **value_ptr,size_t *value_size);

typedef ce_int_t (*read_proc_callback)(u_char* key,size_t key_size,ce_kviter_t *values,u_char *private_data);

struct ce_iter_t
{
	struct list_head filters;
	u_char * attr;
	ce_pool_t *pool;
	u_char *cur_state;
	u_char *end_state;
	u_char *iter_obj;
	iter_has_next_callback_fun has_next;
	iter_next_callback_fun next;
};

struct ce_kviter_t 
{
	ce_pool_t *pool;
	ce_iter_t k_iter;
	ce_iter_t v_iter;
};

typedef void (*recover_old_status)(ce_reader_t *reader,
				   u_char *old_status,
				   u_char *private_data);

struct  ce_reader_t
{
	ce_pool_t *pool;
	ce_kviter_t *kv_iter;
	u_char *private_data;
	u_char *read_obj;
	size_t read_num;
	u_char *old_status;
	recover_old_status rcv_fun;
	read_proc_callback read_proc;
};
extern ce_reader_t*
ce_create_reader(u_char *read_obj,
		   ce_pool_t *pool,	
		   ce_kviter_t *kv_iter,
		   read_proc_callback read_proc,
		   u_char *private_data);
extern ce_int_t
ce_run_reader(ce_reader_t *reader);

extern void 
ce_reset_reader(ce_reader_t *reader);

extern void 
ce_init_iter(ce_iter_t *iter,
	       ce_pool_t *pool,
	       u_char *cur_state,
	       u_char *end_state,
	       u_char *iter_obj,
	       u_char *attr,
	       iter_has_next_callback_fun has_next,
	       iter_next_callback_fun next);
extern void
ce_register_filter(ce_reader_t *reader,
		    ce_kv_filter_t *kv_filter,
		    int k_or_v_filter);
#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_READER_H*/

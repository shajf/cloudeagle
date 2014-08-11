/*
 * =====================================================================================
 *
 *       Filename:  ce_map.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/26/2012 11:32:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef CE_MAP_H
#define CE_MAP_H
#include "ce_basicdefs.h"
#include "ce_string.h"
typedef unsigned long ce_hash_t;
typedef struct ce_map_t ce_map_t;
typedef struct ce_map_entry_t ce_map_entry_t;
typedef struct ce_map_keys_t ce_map_keys_t;

typedef void (*kv_free_fun_ptr)(void *private_data,
				u_char *key,
				void *value);

struct ce_map_entry_t
{
	ce_hash_t  m_hash;
	u_char*  m_key;
	void*   m_value;
};

struct ce_map_keys_t
{
	int m_key_num;
	int m_key_usable;
	ce_map_entry_t m_key_entries[1];
};

struct ce_map_t
{
	int ma_used;
	void *private_data;
	kv_free_fun_ptr kv_free_fun;
	ce_map_keys_t *ma_keys;
};

#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/

extern ce_map_t *ce_create_map(void* private_data,
				kv_free_fun_ptr kv_free_fun);

extern void * ce_map_get_value(ce_map_t *mp,ce_str_t *key);

extern ce_int_t ce_map_set_value(ce_map_t *mp,
				      ce_str_t *key,
				      void *value);

extern ce_int_t ce_map_del_value(ce_map_t *mp,ce_str_t *key);

extern void ce_clear_map(ce_map_t *mp);
extern int ce_map_coceins(ce_map_t *mp,ce_str_t *key);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_MAP_H*/

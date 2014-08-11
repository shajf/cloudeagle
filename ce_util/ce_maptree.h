/*
 * =====================================================================================
 *
 *       Filename:  ce_maptree.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/26/2012 11:32:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng 
 *   Organization:  CE
 *
 * =====================================================================================
 */
#ifndef CE_TREEMAP_H
#define CE_MAPTREE_H
#include "ce_basicdefs.h"
#include "ce_rbtree.h"
#include "ce_palloc.h"

typedef struct ce_maptree_t ce_maptree_t;
typedef struct ce_maptree_node_t ce_maptree_node_t;
typedef int (*key_cmp_fun)(u_char *key1,u_char *key2);
typedef void (*maptree_node_clean_fun)(void *private_data,u_char *key,u_char *value);

struct ce_maptree_t
{
	struct rb_root maptree_node_root;
	ce_pool_t *pool; /*if null,will create map tree in heap*/
	void *private_data;
	key_cmp_fun key_cmp;
	maptree_node_clean_fun node_clean;
};

struct ce_maptree_node_t
{
	struct rb_node rb_maptree_node;
	u_char *key;
	u_char *value;		
};
#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/
extern ce_int_t ce_init_maptree(ce_maptree_t *mpt,void *private_data,key_cmp_fun k_cmp,
				     maptree_node_clean_fun mpt_clean);

extern ce_maptree_t *ce_create_maptree(ce_pool_t *pool,void *private_data,key_cmp_fun k_cmp,
					maptree_node_clean_fun mpt_clean);

extern ce_int_t ce_clear_maptree(ce_maptree_t *mpt);
extern void ce_destroy_maptree(ce_maptree_t *mpt);
extern ce_int_t ce_maptree_set(ce_maptree_t *mpt,u_char *key,u_char *value);
extern u_char *ce_maptree_get(ce_maptree_t *mpt,u_char *key);
extern void ce_maptree_del(ce_maptree_t *mpt,u_char *key);
#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_MAPTREE_H*/


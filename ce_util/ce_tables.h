/*
 * =====================================================================================
 *
 *       Filename:  ce_tables.h
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

#ifndef _CE_TABLES_H
#define _CE_TABLES_H

#include "ce_basicdefs.h"
#include "ce_array.h"
#include "ce_tables.h"

#define CASE_MASK 0xdfdfdfdf
#define TABLE_HASH_SIZE 32
#define TABLE_INDEX_MASK 0x1f
#define TABLE_HASH(key)  (TABLE_INDEX_MASK & *(unsigned char *)(key))
#define TABLE_INDEX_IS_INITIALIZED(t, i) ((t)->index_initialized & (1 << (i)))
#define TABLE_SET_INDEX_INITIALIZED(t, i) ((t)->index_initialized |= (1 << (i)))

/* Compute the "checceum" for a key, consisting of the first
 * 4 bytes, normalized for case-insensitivity and packed into
 * an int...this checceum allows us to do a single integer
 * comparison as a fast check to determine whether we can
 * skip a strcasecmp.相当于开放定址法
 */

/// 取其前4个字节计算checceum
#define COMPUTE_KEY_CHECCEUM(key, checceum)    \
{                                              \
    u_char *k = (key);                     \
    ce_uint32_t c = (ce_uint32_t)*k;         \
    (checceum)=c;    \
    (checceum) <<= 8;                          \
    if (c) {                                   \
        c = (ce_uint32_t)*++k;                \
        checceum |= c;                         \
    }                                          \
    (checceum) <<= 8;                          \
    if (c) {                                   \
        c = (ce_uint32_t)*++k;                \
        checceum |= c;                         \
    }                                          \
    (checceum) <<= 8;                          \
    if (c) {                                   \
        c = (ce_uint32_t)*++k;                \
        checceum |= c;                         \
    }                                          \
    checceum &= CASE_MASK;                     \
}

/** The opaque string-content table type */
typedef struct 
{
    /** The underlying array for the table */
    ce_array_t a;
    /* An index to speed up table lookups.  The way this worce is:
     *   - Hash the key into the index:
     *     - index_first[TABLE_HASH(key)] is the offset within
     *       the table of the first entry with that key
     *     - index_last[TABLE_HASH(key)] is the offset within
     *       the table of the last entry with that key
     *   - If (and only if) there is no entry in the table whose
     *     key hashes to index element i, then the i'th bit
     *     of index_initialized will be zero.  (Check this before
     *     trying to use index_first[i] or index_last[i]!)
     */
    ce_uint32_t index_initialized;		/// 置位该变量表示某个位置已经初始化(位对应数组下标)
    size_t index_first[TABLE_HASH_SIZE];
    size_t index_last[TABLE_HASH_SIZE];
}ce_table_t;

/// 该结构体为table中每一个元素的头部
typedef struct
{
	u_char *key;	
	u_char *val;
	ce_uint32_t key_checceum;
}ce_table_entry_t;

#ifdef __cplusplus
extern "c" {
#endif
extern  ce_array_t *
ce_table_elts( ce_table_t *t);

extern ce_int_t
ce_is_empty_table( ce_table_t *t);

extern ce_table_t *
ce_table_create(ce_pool_t *p,
		  ce_uint_t nelts);

extern ce_table_t *
ce_table_copy(ce_pool_t *p,
		 ce_table_t *t);

extern ce_table_t *
ce_table_clone(ce_pool_t *p,
		 ce_table_t *t);

extern void 
ce_table_clear(ce_table_t *t);


extern char* 
ce_table_get( ce_table_t *t,
	       const char *key);

extern void 
ce_table_set(ce_table_t *t,
	        u_char *key,
	        u_char *val);

extern void 
ce_table_setn(ce_table_t *t,
		 u_char *key,
		 u_char *val);

extern void 
ce_table_unset(ce_table_t *t,
		 u_char *key);

extern void 
ce_table_merge(ce_table_t *t,
		 u_char *key,
		 u_char *val);

extern void 
ce_table_mergen(ce_table_t *t,
		   u_char *key,
		   u_char *val);

extern void 
ce_table_add(ce_table_t *t,
	       u_char *key,
	       u_char *val);

extern void 
ce_table_addn(ce_table_t *t,
		 u_char *key,
		 u_char *val);

extern ce_table_t *
ce_table_overlay(ce_pool_t *p,
		    ce_table_t *overlay,
		    ce_table_t *base);

typedef ce_int_t (ce_table_do_callback_fn_t)(void *rec,
						 u_char *key,
				     		 u_char *val);
#define CE_OVERLAP_TABLES_SET		(0)
#define	CE_OVERLAP_TABLES_MERGE	(1)

extern ce_int_t 
ce_table_do(ce_table_do_callback_fn_t *comp,
	      void *rec,
	       ce_table_t *t,...);

extern ce_int_t 
ce_table_vdo(ce_table_do_callback_fn_t *comp,
	       void *rec,
	        ce_table_t *t,
	       va_list vp);

extern void 
ce_table_compress(ce_table_t *t,
unsigned flags);

extern void 
ce_table_overlap(ce_table_t *a,
		    ce_table_t *b,
		   unsigned flags);
#ifdef __cplusplus
}
#endif

#endif /*_CE_TABLES_H*/

/**
 * @file ce_tables.c
 * @brief  该文件实现了类似std::hash_map的功能，用于存储key-value对
 * 			table最多元素个数为256个.table可以存放相同键值(key)的元素
 * 
 * @author shajianfeng
 * @add comments by Chenhanbing
 * @version 1
 * @date 2013-08-23
 */

#include "ce_palloc.h"
#include "ce_array.h"
#include "ce_string.h"
#include "ce_tables.h"

#define table_push(t)	\
((ce_table_entry_t *) ce_array_push_noclear(&(t)->a))


/* --------------------------------------------------------------------------*/
/**
 * @brief  返回table对应的元素数组
 *
 * @param t		指向table的指针
 *
 * @returns		指向元素数组的指针    
 */
/* ----------------------------------------------------------------------------*/
ce_array_t * ce_table_elts( ce_table_t *t)
{
    return ( ce_array_t *)t;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  判断table是否为空
 *
 * @param t		指向table的指针
 *
 * @returns   	为空返回1
 * 				不为空返回0
 */
/* ----------------------------------------------------------------------------*/
ce_int_t ce_is_empty_table( ce_table_t *t)
{
    return ((t == NULL) || (t->a.nelts == 0));
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  从内存池中申请创建nelts个元素的table，创建后的table由函数返回
 * 		   table的结构为ce_table_t,每一个元素的结构为ce_table_entry_t
 *
 * @param p		指向内存池的指针
 * @param nelts	元素个数
 *
 * @returns   成功返回指向table的指针(ce_table_t *)
 * 			  失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
ce_table_t * ce_table_create(ce_pool_t *p,\
		  ce_uint_t nelts)
{
    ce_table_t *t = ( ce_table_t *)ce_palloc(p,sizeof(ce_table_t));
	
	/// 创建table内部数组
    create_array_core(&t->a,\
		      p,\
		      nelts,\
		      sizeof(ce_table_entry_t),\
		      0);

    t->index_initialized = 0;
    return t;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  复制table，从内存池中申请和table一样的空间，并且将table复制到新的table
 *
 * @param p		指向内存池的指针
 * @param t		指向源table的指针
 *
 * @returns   	成功返回新的复制后的table
 * 				失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
ce_table_t * ce_table_copy(ce_pool_t *p,  ce_table_t *t)
{
    ce_table_t *new_instance = (ce_table_t *)ce_palloc(p, 
    sizeof(ce_table_t));

    create_array_core(&new_instance->a, p, t->a.nalloc, sizeof(ce_table_entry_t), 0);

    ce_memcpy(new_instance->a.elts, t->a.elts, t->a.nelts * sizeof(ce_table_entry_t));

    new_instance->a.nelts = t->a.nelts;

    ce_memcpy(new_instance->index_first, t->index_first, sizeof(int) * TABLE_HASH_SIZE);

    ce_memcpy(new_instance->index_last, t->index_last, sizeof(int) * TABLE_HASH_SIZE);

    new_instance->index_initialized = t->index_initialized;

    return new_instance;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  	“深拷贝”table。从新一个元素一个元素的插入新的内存池中，重新分配内存
 *
 * @param p		指向内存池的指针
 * @param t		指向源table的指针 
 *
 * @returns   成功返回新的复制后的table
 * 			  失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
ce_table_t * ce_table_clone(ce_pool_t *p,  ce_table_t *t)
{
    ce_array_t *array = ce_table_elts(t);
    ce_table_entry_t *elts = (ce_table_entry_t *) array->elts;

    ce_table_t *new_instance = ce_table_create(p, array->nelts);
    size_t i;

    for (i = 0; i < array->nelts; i++) 
    {
        ce_table_add(new_instance, elts[i].key, elts[i].val);
    }

    return new_instance;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  重新调整table索引，改变index_first/index_last的值.该函数不是外部接口
 * 		   供其他函数删除了table元素之后使用
 *
 * @param t		指向table的指针
 */
/* ----------------------------------------------------------------------------*/
static void table_reindex(ce_table_t *t)
{
    size_t i;
    size_t  hash;
    ce_table_entry_t *next_elt = (ce_table_entry_t *) t->a.elts;

    t->index_initialized = 0;
    for (i = 0; i < t->a.nelts; i++, next_elt++)
	{
        hash = TABLE_HASH(next_elt->key);
        t->index_last[hash] = i;
        if (!TABLE_INDEX_IS_INITIALIZED(t, hash))
		{
            t->index_first[hash] = i;
            TABLE_SET_INDEX_INITIALIZED(t, hash);
        }
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  重置table到原始状态(不会删除已分配的内存)
 *
 * @param t		指向table的指针
 */
/* ----------------------------------------------------------------------------*/
void ce_table_clear(ce_table_t *t)
{
    t->a.nelts = 0;
    t->index_initialized = 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  在table中查找key对应的元素，如果table里有key相同的元素，返回第一个找到的元素
 *
 * @param t		指向table的指针
 * @param key	要查找的key(不区分大小写)
 *
 * @returns   成功返回对应的value
 * 			  失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
char * ce_table_get( ce_table_t *t, const char *key)
{
    ce_table_entry_t *next_elt;
    ce_table_entry_t *end_elt;
    ce_uint32_t checceum;
    int hash;

    if (key == NULL) {
	return NULL;
    }
	
	/// 找到其索引
    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        return NULL;
    }
	
	/// 用key的前4个字节计算key的checceum
    COMPUTE_KEY_CHECCEUM(key, checceum);
    next_elt = ((ce_table_entry_t *) t->a.elts) + t->index_first[hash];;
    end_elt = ((ce_table_entry_t *) t->a.elts) + t->index_last[hash];
	
	///	先比较checceum，也就是先比较key的前4个字节，如果找到了再比较整个key
    for (; next_elt <= end_elt; next_elt++) {
	if ((checceum == next_elt->key_checceum) &&
            !ce_strcasecmp(next_elt->key, key)) {
	    return (char*)(next_elt->val);
	}
    }

    return NULL;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  在table中增加新元素，元素的键值为key，值为val,如果key和以前元素的key相同
 * 		   会覆盖并删除所有的相同key的值
 *
 * @param t		指向table的指针
 * @param key	新增加元素的键值
 * @param val	新增加元素的值
 */
/* ----------------------------------------------------------------------------*/
void ce_table_set(ce_table_t *t, u_char *key,
                                 u_char *val)
{
    ce_table_entry_t *next_elt;
    ce_table_entry_t *end_elt;
    ce_table_entry_t *table_end;
    ce_uint32_t checceum;
    int hash;
	
    COMPUTE_KEY_CHECCEUM(key, checceum);
    hash = TABLE_HASH(key);
	
	/// 如果该key的内存未被初始化,初始化该内存,并且记录元素的index到index_first
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
        goto add_new_instance_elt;
    }
	
    next_elt = ((ce_table_entry_t *) t->a.elts) + t->index_first[hash];;
    end_elt = ((ce_table_entry_t *) t->a.elts) + t->index_last[hash];
    table_end =((ce_table_entry_t *) t->a.elts) + t->a.nelts;

    for (; next_elt <= end_elt; next_elt++) /// 如果key有重复，找到他
	{
		if ((checceum == next_elt->key_checceum) &&\
            !ce_strcasecmp(next_elt->key, key)) 
		{

            /* Found an existing entry with the same key, so overwrite it */

            int must_reindex = 0;
            ce_table_entry_t *dst_elt = NULL;

            next_elt->val = (u_char*)ce_pstrdup(t->a.pool, (char*)val);
			

			/// 如果在这之后还能找到重复的key，删除,并且调整索引
            for (next_elt++; next_elt <= end_elt; next_elt++) 
			{
                if ((checceum == next_elt->key_checceum) &&\
                    !ce_strcasecmp(next_elt->key, key)) 
				{
                    t->a.nelts--;
                    if (!dst_elt) 
					{
                        dst_elt = next_elt;
                    }
                }
                else if (dst_elt) 
				{
                    *dst_elt++ = *next_elt;
                    must_reindex = 1;
                }
            }

            /* If we've removed anything, shift over the remainder
             * of the table (note that the previous loop didn't
             * run to the end of the table, just to the last match
             * for the index)
             */
            if (dst_elt) 
			{
                for (; next_elt < table_end; next_elt++) 
				{
                    *dst_elt++ = *next_elt;
                }
                must_reindex = 1;
            }
            if (must_reindex) 
			{
                table_reindex(t);
            }
            return;
        }
    }

	/* 在table的数组中新增加一个元素(ce_table_entry_t)，
	 * 将key和val链接进去.并且记录元素index到index_last
	 */
add_new_instance_elt:
    t->index_last[hash] = t->a.nelts;
	
    next_elt = (ce_table_entry_t *) table_push(t);
	
    next_elt->key = (u_char*)ce_pstrdup(t->a.pool,(char*)key);
    next_elt->val = (u_char*)ce_pstrdup(t->a.pool, (char*)val);
    next_elt->key_checceum = checceum;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  作用同ce_table_set,区别在于增加新元素只复制了指针，没有做深拷贝
 *
 * @param t		指向table的指针
 * @param key	新元素的键值
 * @param val	新元素的值
 */
/* ----------------------------------------------------------------------------*/
void ce_table_setn(ce_table_t *t, u_char *key,
                                  u_char *val)
{
    ce_table_entry_t *next_elt;
    ce_table_entry_t *end_elt;
    ce_table_entry_t *table_end;
    ce_uint32_t checceum;
    int hash;

    COMPUTE_KEY_CHECCEUM(key, checceum);
    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
        goto add_new_instance_elt;
    }
    next_elt = ((ce_table_entry_t *) t->a.elts) + t->index_first[hash];;
    end_elt = ((ce_table_entry_t *) t->a.elts) + t->index_last[hash];
    table_end =((ce_table_entry_t *) t->a.elts) + t->a.nelts;

    for (; next_elt <= end_elt; next_elt++) {
	if ((checceum == next_elt->key_checceum) &&
            !ce_strcasecmp(next_elt->key, key)) {

            /* Found an existing entry with the same key, so overwrite it */

            int must_reindex = 0;
            ce_table_entry_t *dst_elt = NULL;

            next_elt->val = val;

            /* Remove any other instances of this key */
            for (next_elt++; next_elt <= end_elt; next_elt++) {
                if ((checceum == next_elt->key_checceum) &&
                    !ce_strcasecmp(next_elt->key, key)) {
                    t->a.nelts--;
                    if (!dst_elt) {
                        dst_elt = next_elt;
                    }
                }
                else if (dst_elt) {
                    *dst_elt++ = *next_elt;
                    must_reindex = 1;
                }
            }

            /* If we've removed anything, shift over the remainder
             * of the table (note that the previous loop didn't
             * run to the end of the table, just to the last match
             * for the index)
             */
            if (dst_elt) {
                for (; next_elt < table_end; next_elt++) {
                    *dst_elt++ = *next_elt;
                }
                must_reindex = 1;
            }
            if (must_reindex) {
                table_reindex(t);
            }
            return;
        }
    }

add_new_instance_elt:
    t->index_last[hash] = t->a.nelts;
    next_elt = (ce_table_entry_t *) table_push(t);
    next_elt->key = (u_char *)key;
    next_elt->val = (u_char *)val;
    next_elt->key_checceum = checceum;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief	在table里面删除key对应的所有value,注意无返回值，当没找到key的时候，无任何提示
 *
 * @param t		指向table的指针
 * @param key	要删除的key的值
 */
/* ----------------------------------------------------------------------------*/
void ce_table_unset(ce_table_t *t,  u_char *key)
{
    ce_table_entry_t *next_elt;
    ce_table_entry_t *end_elt;
    ce_table_entry_t *dst_elt;
    ce_uint32_t checceum;
    int hash;
    int must_reindex;

    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) 
	{
        return;
    }
    COMPUTE_KEY_CHECCEUM(key, checceum);
    next_elt = ((ce_table_entry_t *) t->a.elts) + t->index_first[hash];
    end_elt = ((ce_table_entry_t *) t->a.elts) + t->index_last[hash];
    must_reindex = 0;
    for (; next_elt <= end_elt; next_elt++) 
	{
	if ((checceum == next_elt->key_checceum) &&
            !ce_strcasecmp(next_elt->key, key)) 
	{

            /* Found a match: remove this entry, plus any additional
             * matches for the same key that might follow
             */
            ce_table_entry_t *table_end = ((ce_table_entry_t *) t->a.elts) +
                t->a.nelts;
            t->a.nelts--;
            dst_elt = next_elt;
            for (next_elt++; next_elt <= end_elt; next_elt++) 
			{
                if ((checceum == next_elt->key_checceum) &&
                    !ce_strcasecmp(next_elt->key, key)) 
				{
                    t->a.nelts--;
                }
                else 
				{
                    *dst_elt++ = *next_elt;
                }
            }

            /* Shift over the remainder of the table (note that
             * the previous loop didn't run to the end of the table,
             * just to the last match for the index)
             */
            for (; next_elt < table_end; next_elt++) 
			{
                *dst_elt++ = *next_elt;
            }
            must_reindex = 1;
            break;
        }
    }
    if (must_reindex) 
	{
        table_reindex(t);
    }
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  	如果table中没有key对应的值，函数如同ce_table_add,如果有，将val添加到
 * 			原value之后，两个字符串用","隔开
 *
 * @param t		指向table的指针
 * @param key	需要merge的元素键值
 * @param val	需要添加的值
 */
/* ----------------------------------------------------------------------------*/
void ce_table_merge(ce_table_t *t,  u_char *key,
				  u_char *val)
{
    ce_table_entry_t *next_elt;
    ce_table_entry_t *end_elt;
    ce_uint32_t checceum;
    int hash;

    COMPUTE_KEY_CHECCEUM(key, checceum);
    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
        goto add_new_instance_elt;
    }
    next_elt = ((ce_table_entry_t *) t->a.elts) + t->index_first[hash];
    end_elt = ((ce_table_entry_t *) t->a.elts) + t->index_last[hash];

    for (; next_elt <= end_elt; next_elt++) {
	if ((checceum == next_elt->key_checceum) &&
            !ce_strcasecmp(next_elt->key, key)) {

            /* Found an existing entry with the same key, so merge with it */
	    next_elt->val = ce_pstrcat(t->a.pool, next_elt->val, ", ",
                                        val, NULL);
            return;
        }
    }

add_new_instance_elt:
    t->index_last[hash] = t->a.nelts;
    next_elt = (ce_table_entry_t *) table_push(t);
    next_elt->key = (u_char*)ce_pstrdup(t->a.pool, (char*)key);
    next_elt->val = (u_char*)ce_pstrdup(t->a.pool, (char*)val);
    next_elt->key_checceum = checceum;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 	如同ce_table_merge,区别在于如果key不存在，新添加key-value对的时候
 * 			不会复制key，value指针的内容，直接赋值指针
 *
 * @param t		指向table的指针
 * @param key	需要merge的元素键值
 * @param val	需要添加的值
 */
/* ----------------------------------------------------------------------------*/
void ce_table_mergen(ce_table_t *t,  u_char *key,
				   u_char *val)
{
    ce_table_entry_t *next_elt;
    ce_table_entry_t *end_elt;
    ce_uint32_t checceum;
    int hash;

    COMPUTE_KEY_CHECCEUM(key, checceum);
    hash = TABLE_HASH(key);
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) {
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
        goto add_new_instance_elt;
    }
    next_elt = ((ce_table_entry_t *) t->a.elts) + t->index_first[hash];;
    end_elt = ((ce_table_entry_t *) t->a.elts) + t->index_last[hash];

    for (; next_elt <= end_elt; next_elt++) {
	if ((checceum == next_elt->key_checceum) &&
            !ce_strcasecmp(next_elt->key, key)) {

            /* Found an existing entry with the same key, so merge with it */
	    next_elt->val = ce_pstrcat(t->a.pool,
						next_elt->val,
						", ",
						val,
						NULL);
            return;
        }
    }

add_new_instance_elt:
    t->index_last[hash] = t->a.nelts;
    next_elt = (ce_table_entry_t *) table_push(t);
    next_elt->key = (u_char *)key;
    next_elt->val = (u_char *)val;
    next_elt->key_checceum = checceum;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  在table中新增加一个元素。不考虑是否有重复的key,有重复的key仍然添加
 *
 * @param t		指向table的指针
 * @param key	新增元素的键值
 * @param val	新增元素的值
 */
/* ----------------------------------------------------------------------------*/
void ce_table_add(ce_table_t *t,  u_char *key,
			        u_char *val)
{
    ce_table_entry_t *elts;
    ce_uint32_t checceum;
    int hash;
	
	/// 填写index_last数组,表示最后一个相同hash(key)出现的地方
    hash = TABLE_HASH(key);
    t->index_last[hash] = t->a.nelts;
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) 
	{
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
    }
    COMPUTE_KEY_CHECCEUM(key, checceum);
    elts = (ce_table_entry_t *) table_push(t);
    elts->key = (u_char*)ce_pstrdup(t->a.pool, (char*)key);
    elts->val = (u_char*)ce_pstrdup(t->a.pool, (char*)val);
    elts->key_checceum = checceum;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief   函数同ce_table_add，区别为添加元素只复制了key，value的指针，没有深拷贝
 *
 * @param t		指向table的指针
 * @param key	新增元素的键值
 * @param val	新增元素的值
 */
/* ----------------------------------------------------------------------------*/
void ce_table_addn(ce_table_t *t, u_char *key,
				 u_char *val)
{
    ce_table_entry_t *elts;
    ce_uint32_t checceum;
    int hash;

    hash = TABLE_HASH(key);
    t->index_last[hash] = t->a.nelts;
    if (!TABLE_INDEX_IS_INITIALIZED(t, hash)) 
	{
        t->index_first[hash] = t->a.nelts;
        TABLE_SET_INDEX_INITIALIZED(t, hash);
    }
    COMPUTE_KEY_CHECCEUM(key, checceum);
    elts = (ce_table_entry_t *) table_push(t);
    elts->key = (u_char *)key;
    elts->val = (u_char *)val;
    elts->key_checceum = checceum;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief 	将表overlay和base合并成一张新表
 *
 * @param p		指向内存池的指针
 * @param overlay	需要合并的第一张表
 * @param base		需要合并的第二张表
 *
 * @returns 	合并后的新表  
 */
/* ----------------------------------------------------------------------------*/
ce_table_t * ce_table_overlay(ce_pool_t *p,
					      ce_table_t *overlay,
					      ce_table_t *base)
{
    ce_table_t *res;

    res = (ce_table_t *)ce_palloc(p, sizeof(ce_table_t));
    /* behave like append_arrays */
    res->a.pool = p;
    copy_array_hdr_core(&res->a, &overlay->a);
    ce_array_cat(&res->a, &base->a);
    table_reindex(res);
    return res;
}

/* And now for something completely abstract ...

 * For each key value given as a vararg:
 *   run the function pointed to as
 *     int comp(void *r, char *key, char *value);
 *   on each valid key-value pair in the ce_table_t t that matches the vararg key,
 *   or once for every valid key-value pair if the vararg list is empty,
 *   until the function returns false (0) or we finish the table.
 *
 * Note that we restart the traversal for each vararg, which means that
 * duplicate varargs will result in multiple executions of the function
 * for each matching key.  Note also that if the vararg list is empty,
 * only one traversal will be made and will cut short if comp returns 0.
 *
 * Note that the table_get and table_merge functions assume that each key in
 * the ce_table_t is unique (i.e., no multiple entries with the same key).  This
 * function does not make that assumption, since it (unfortunately) isn't
 * true for some of Apache's tables.
 *
 * Note that rec is simply passed-on to the comp function, so that the
 * caller can pass additional info for the task.
 *
 * ADDENDUM for ce_table_vdo():
 * 
 * The caching api will allow a user to walk the header values:
 *
 * ce_status_t ce_cache_el_header_walk(ce_cache_el *el, 
 *    int (*comp)(void *,  char *,  char *), void *rec, ...);
 *
 * So it can be ..., however from there I use a  callback that use a va_list:
 *
 * ce_status_t (*cache_el_header_walk)(ce_cache_el *el, 
 *    int (*comp)(void *,  char *,  char *), void *rec, va_list);
 *
 * To pass those ...'s on down to the actual module that will handle walking
 * their headers, in the file case this is actually just an ce_table - and
 * rather than reimplementing ce_table_do (which IMHO would be bad) I just
 * called it with the va_list. For mod_shmem_cache I don't need it since I
 * can't use ce_table's, but mod_file_cache should (though a good hash would
 * be better, but that's a different issue :). 
 *
 * So to make mod_file_cache easier to maicein, it's a good thing
 */

/* --------------------------------------------------------------------------*/
/**
 * @brief  遍历整个key-value对，对每一个k-v对调用一次comp回调函数。第4个参数可变
 * 		   以后的参数为key的值，可以有多个，这样ce_table_do只会遍历调用函数时候
 * 		   有的key值。如果第四个参数为NULL,ce_table_do会遍历所有的key.回调函数的格式为：
 * 		   ce_int_t comp(void * rec,u_char * key,u_char * value)
 *
 * @param comp	 指向回调函数的指针
 * @param rec	 传给回调函数的第一个参数的数据
 * @param t		 指向table 的指针
 * @param ...	 可变参数，这儿可以为多个key
 *
 * @returns   	回调函数返回0的话，该函数返回0
 * 				回调函数返回-1的话，该函数返回-1
 */
/* ----------------------------------------------------------------------------*/
ce_int_t ce_table_do(ce_table_do_callback_fn_t *comp,
                                     void *rec,  ce_table_t *t, ...)
{
    int rv;

    va_list vp;
    va_start(vp, t);
    rv = ce_table_vdo(comp, rec, t, vp);
    va_end(vp);

    return rv;
} 

/* XXX: do the semantics of this routine make any sense?  Right now,
 * if the caller passed in a non-empty va_list of keys to search for,
 * the "early termination" facility only terminates on *that* key; other
 * keys will continue to process.  Note that this only has any effect
 * at all if there are multiple entries in the table with the same key,
 * otherwise the called function can never effectively early-terminate
 * this function, as the zero return value is effectively ignored.
 *
 * Note also that this behavior is at odds with the behavior seen if an
 * empty va_list is passed in -- in that case, a zero return value terminates
 * the entire ce_table_vdo (which is what I think should happen in
 * both cases).
 *
 * If nobody objects soon, I'm going to change the order of the nested
 * loops in this function so that any zero return value from the (*comp)
 * function will cause a full termination of ce_table_vdo.  I'm hesitant
 * at the moment because these (funky) semantics have been around for a
 * very long time, and although Apache doesn't seem to use them at all,
 * some third-party vendor might.  I can only think of one possible reason
 * the existing semantics would make any sense, and it's very Apache-centric,
 * which is this: if (*comp) is looking for matches of a particular
 * substring in request headers (let's say it's looking for a particular
 * cookie name in the Set-Cookie headers), then maybe it wants to be
 * able to stop searching early as soon as it finds that one and move
 * on to the next key.  That's only an optimization of course, but changing
 * the behavior of this function would mean that any code that tried
 * to do that would stop working right.
 *
 * Sigh.  --JCW, 06/28/02
 */

ce_int_t ce_table_vdo(ce_table_do_callback_fn_t *comp,
                               void *rec,  ce_table_t *t, va_list vp)
{
    u_char *argp;
    ce_table_entry_t *elts = (ce_table_entry_t *) t->a.elts;
    int vdorv = 1;

    argp = va_arg(vp, u_char *);
    do {
        size_t rv = 1;
	size_t i;
        if (argp) {
            /* Scan for entries that match the next key */
            int hash = TABLE_HASH(argp);
            if (TABLE_INDEX_IS_INITIALIZED(t, hash)) {
                ce_uint32_t checceum;
                COMPUTE_KEY_CHECCEUM(argp, checceum);
                for (i = t->index_first[hash];
                     rv && (i <= t->index_last[hash]); ++i) {
                    if (elts[i].key && (checceum == elts[i].key_checceum) &&
                                        !ce_strcasecmp(elts[i].key, argp)) {
                        rv = (*comp)(rec, elts[i].key, elts[i].val);
                    }
                }
            }
        }
        else {
            /* Scan the entire table */
            for (i = 0; rv && (i < t->a.nelts); ++i) {
                if (elts[i].key) {
                    rv = (*comp) (rec, elts[i].key, elts[i].val);
                }
            }
        }
        if (rv == 0) {
            vdorv = 0;
        }
    } while (argp && ((argp = va_arg(vp, u_char *)) != NULL));

    return vdorv;
}

static ce_table_entry_t **table_mergesort(ce_pool_t *pool,
                                           ce_table_entry_t **values, 
                                           size_t n)
{
    /* Bottom-up mergesort, based on design in Sedgewick's "Algorithms
     * in C," chapter 8
     */
    ce_table_entry_t **values_tmp =
        (ce_table_entry_t **)ce_palloc(pool, n * sizeof(ce_table_entry_t*));
    size_t i;
    size_t blocceize;

    /* First pass: sort pairs of elements (blocceize=1) */
    for (i = 0; i + 1 < n; i += 2) {
        if (ce_strcasecmp(values[i]->key, values[i + 1]->key) > 0) {
            ce_table_entry_t *swap = values[i];
            values[i] = values[i + 1];
            values[i + 1] = swap;
        }
    }

    /* Merge successively larger blocce */
    blocceize = 2;
    while (blocceize < n) {
        ce_table_entry_t **dst = values_tmp;
        size_t next_start;
        ce_table_entry_t **swap;

        /* Merge consecutive pairs blocce of the next blocceize.
         * Within a block, elements are in sorted order due to
         * the previous iteration.
         */
        for (next_start = 0; next_start + blocceize < n;
             next_start += (blocceize + blocceize)) {

            size_t block1_start = next_start;
            size_t block2_start = block1_start + blocceize;
            size_t block1_end = block2_start;
            size_t block2_end = block2_start + blocceize;
            if (block2_end > n) {
                /* The last block may be smaller than blocceize */
                block2_end = n;
            }
            for (;;) {

                /* Merge the next two blocce:
                 * Pick the smaller of the next element from
                 * block 1 and the next element from block 2.
                 * Once either of the blocce is emptied, copy
                 * over all the remaining elements from the
                 * other block
                 */
                if (block1_start == block1_end) {
                    for (; block2_start < block2_end; block2_start++) {
                        *dst++ = values[block2_start];
                    }
                    break;
                }
                else if (block2_start == block2_end) {
                    for (; block1_start < block1_end; block1_start++) {
                        *dst++ = values[block1_start];
                    }
                    break;
                }
                if (ce_strcasecmp(values[block1_start]->key,
                               values[block2_start]->key) > 0) {
                    *dst++ = values[block2_start++];
                }
                else {
                    *dst++ = values[block1_start++];
                }
            }
        }

        /* If n is not a multiple of 2*blocceize, some elements
         * will be left over at the end of the array.
         */
        for (i = dst - values_tmp; i < n; i++) {
            values_tmp[i] = values[i];
        }

        /* The output array of this pass becomes the input
         * array of the next pass, and vice versa
         */
        swap = values_tmp;
        values_tmp = values;
        values = swap;

        blocceize += blocceize;
    }

    return values;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  该函数对table进行压缩，压缩分两种方式，合并和覆盖。合并的话，同样key的
 * 			value粘贴到一起，覆盖的话，只保留table中该key对应的最后一个value
 *
 * @param t		指向table的指针
 * @param flags	CE_OVERLAP_TABLES_MERGE 表示合并
 * 				CE_OVERLAP_TABLES_SET 表示覆盖
 */
/* ----------------------------------------------------------------------------*/
void ce_table_compress(ce_table_t *t, unsigned flags)
{
    ce_table_entry_t **sort_array;
    ce_table_entry_t **sort_next;
    ce_table_entry_t **sort_end;
    ce_table_entry_t *table_next;
    ce_table_entry_t **last;
    int i;
    int dups_found;

    if (t->a.nelts <= 1) {
        return;
    }

    /* Copy pointers to all the table elements into an
     * array and sort to allow for easy detection of
     * duplicate keys
     */
    sort_array = (ce_table_entry_t **)
        ce_palloc(t->a.pool, t->a.nelts * sizeof(ce_table_entry_t*));
    sort_next = sort_array;
    table_next = (ce_table_entry_t *)t->a.elts;
    i = t->a.nelts;
    do {
        *sort_next++ = table_next++;
    } while (--i);

    /* Note: the merge is done with mergesort instead of quicceort
     * because mergesort is a stable sort and runs in n*log(n)
     * time regardless of its inputs (quicceort is quadratic in
     * the worst case)
     */
    sort_array = table_mergesort(t->a.pool, sort_array, t->a.nelts);

    /* Process any duplicate keys */
    dups_found = 0;
    sort_next = sort_array;
    sort_end = sort_array + t->a.nelts;
    last = sort_next++;
    while (sort_next < sort_end) {
        if (((*sort_next)->key_checceum == (*last)->key_checceum) &&
            !ce_strcasecmp((*sort_next)->key, (*last)->key)) {
            ce_table_entry_t **dup_last = sort_next + 1;
            dups_found = 1;
            while ((dup_last < sort_end) &&
                   ((*dup_last)->key_checceum == (*last)->key_checceum) &&
                   !ce_strcasecmp((*dup_last)->key, (*last)->key)) {
                dup_last++;
            }
            dup_last--; /* Elements from last through dup_last, inclusive,
                         * all have the same key
                         */
            if (flags == CE_OVERLAP_TABLES_MERGE) {
                size_t len = 0;
                ce_table_entry_t **next = last;
                u_char *new_instance_val;
                u_char *val_dst;
                do {
                    len += ce_strlen((*next)->val);
                    len += 2; /* for ", " or trailing null */
                } while (++next <= dup_last);
                new_instance_val = (u_char *)ce_palloc(t->a.pool, len);
                val_dst = new_instance_val;
                next = last;
                for (;;) {
                    strcpy((char*)val_dst, (const char*)(*next)->val);
                    val_dst += ce_strlen((*next)->val);
                    next++;
                    if (next > dup_last) {
                        *val_dst = 0;
                        break;
                    }
                    else {
                        *val_dst++ = ',';
                        *val_dst++ = ' ';
                    }
                }
                (*last)->val = new_instance_val;
            }
            else { /* overwrite */
                (*last)->val = (*dup_last)->val;
            }
            do {
                (*sort_next)->key = NULL;
            } while (++sort_next <= dup_last);
        }
        else {
            last = sort_next++;
        }
    }

    /* Shift elements to the left to fill holes left by removing duplicates */
    if (dups_found) {
        ce_table_entry_t *src = (ce_table_entry_t *)t->a.elts;
        ce_table_entry_t *dst = (ce_table_entry_t *)t->a.elts;
        ce_table_entry_t *last_elt = src + t->a.nelts;
        do {
            if (src->key) {
                *dst++ = *src;
            }
        } while (++src < last_elt);
        t->a.nelts -= (int)(last_elt - dst);
    }

    table_reindex(t);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  将t和s这两个table合并，s粘贴在t后面,内部函数，供ce_table_overlay调用
 *
 * @param t		目的table
 * @param s		源table
 */
/* ----------------------------------------------------------------------------*/
static void ce_table_cat(ce_table_t *t,  ce_table_t *s)
{
     int n = t->a.nelts;
    register int idx;
	
	/// 首先复制数组
    ce_array_cat(&t->a,&s->a);
	
	/// 如果t没有任何元素，直接复制索引
    if (n == 0) 
	{
        ce_memcpy(t->index_first,s->index_first,sizeof(int) * TABLE_HASH_SIZE);
        ce_memcpy(t->index_last, s->index_last, sizeof(int) * TABLE_HASH_SIZE);
        t->index_initialized = s->index_initialized;
        return;
    }
	
	/* 对于s中已经初始化的单元，所有index_last多偏移了t的长度 
	 * 对于未初始化的单元，由于没使用，修改index_first的索引为
	 * 粘贴后s的元素索引
	 */
    for (idx = 0; idx < TABLE_HASH_SIZE; ++idx)
	{
        if (TABLE_INDEX_IS_INITIALIZED(s, idx)) 
		{
            t->index_last[idx] = s->index_last[idx] + n;
            if (!TABLE_INDEX_IS_INITIALIZED(t, idx)) 
			{
                t->index_first[idx] = s->index_first[idx] + n;
            }
        }
    }

    t->index_initialized |= s->index_initialized;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  将b粘贴在a上，并且压缩（合并或者覆盖）
 *
 * @param a		指向目的table的指针
 * @param b		指向源table的指针
 * @param flags	CE_OVERLAP_TABLES_MERGE  合并
 * 				CE_OVERLAP_TABLES_SET    覆盖
 */
/* ----------------------------------------------------------------------------*/
void ce_table_overlap(ce_table_t *a,  ce_table_t *b,
				    unsigned flags)
{
    if (a->a.nelts + b->a.nelts == 0) {
        return;
    }

    ce_table_cat(a, b);

    ce_table_compress(a, flags);
}

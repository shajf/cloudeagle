/**
 * @file ce_array.c
 * @brief  动态数组的实现,提供的接口表现形式如同一个用数组实现的堆栈。
 * 		   数组有一个默认的长度（创建的时候），也可以动态的push，pop元素
 * 
 * @author shajianfeng
 * @add comments by chenhanbing
 * @version 1
 * @date 2013-08-23
 */


#include "ce_array.h"
#include "ce_palloc.h"
#include "ce_string.h"

/* --------------------------------------------------------------------------*/
/**
 * @brief  在ce_pool_t内存池中申请内存空间，创建一个动态数组,如果nelts个数为0,
 * 			将会推迟内存分配到数组第一次使用。ce_array_create会调用该函数
 *
 * @param arr       动态数组的指针
 * @param pool   	内存池的指针
 * @param nelts     元素个数
 * @param elt_size  每个元素的大小
 * @param clear	    该参数为true的时候（非0的时候），数组元素的内存在分配的时候会初始化为0
 * 					为false 的时候(0的时候)不会对内存初始化
 */
/* ----------------------------------------------------------------------------*/
void create_array_core(ce_array_t *arr, ce_pool_t *pool, ce_uint_t nelts,
		size_t elt_size, ce_int_t clear)
{
	/*
	 * Assure sanity if someone asce for
	 * array of zero elts.
	 */

	if (nelts < 1)
	{
		nelts = 1;
	}

	if (clear)
	{
		arr->elts = (u_char*) ce_pcalloc(pool, nelts * elt_size);
	}
	else
	{
		arr->elts = (u_char*) ce_palloc(pool, nelts * elt_size);
	}

	arr->pool = pool;
	arr->size = elt_size;
	arr->nelts = 0; /* No active elements yet... */
	arr->nalloc = nelts; /* ...but this many allocated */
}

ce_int_t ce_is_empty_array(ce_array_t *arr)
{
	return ((arr == NULL) || (arr->nelts == 0));
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  在ce_pool_t内存池中申请内存空间，创建一个动态数组
 *
 * @param pool	指向ce_pool_t内存池的指针
 * @param nelts		数组元素的个数
 * @param elt_size	 每一个数组元素的长度
 *
 * @returns 成功返回动态数组的指针(ce_array_t)
 * 			失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
ce_array_t * ce_array_create(ce_pool_t *pool, ce_uint_t nelts, size_t elt_size)
{
	ce_array_t *arr;

	arr = (ce_array_t *) ce_palloc(pool, sizeof(ce_array_t));
	if (arr == NULL)
	{
		return NULL;
	}

	create_array_core(arr, pool, nelts, elt_size, 1);

	return arr;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  重置数组，将已使用元素计数计为0
 *	
 * @param arr	指向数组的指针
 */
/* ----------------------------------------------------------------------------*/
void ce_array_clear(ce_array_t *arr)
{
	arr->nelts = 0;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  取出数组的最后一个元素，返回最后一个元素的地址，并且把已使用元素个数-1
 *
 * @param arr	指向动态数组的指针
 *
 * @returns  成功返回数组最后一个元素的地址
 * 			 失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
u_char *ce_array_pop(ce_array_t *arr)
{
	if (ce_is_empty_array(arr))
	{
		return NULL;
	}

	return arr->elts + (arr->size * (--arr->nelts));
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  数组的随机读取操作，类似下标操作符arr[i]
 *
 * @param arr	指向动态数组的指针
 * @param i		要访问元素的位置
 *
 * @returns		成功返回该元素的地址
 * 				失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
u_char * ce_array_get(ce_array_t *arr, size_t i)
{
	if (ce_is_empty_array(arr) || i >= arr->nelts)
	{
		return NULL;
	}

	return arr->elts + (arr->size * i);
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  往数组中压入新的元素（提供新的元素的内存空间）
 *
 * @param arr	指向动态数组的指针
 *
 * @returns   返回新元素的地址
 */
/* ----------------------------------------------------------------------------*/
u_char * ce_array_push(ce_array_t *arr)
{
	ce_pool_t *p;
	
	/// 如果数组上所有的空间已经分配完
	if (arr->nelts == arr->nalloc)
	{
		ce_uint_t size = arr->size * arr->nalloc;

		p = arr->pool;
		
        /// 如果内存池的该内存块还能分配一个元素的空间，分配给数组
		if ((u_char*) arr->elts + size == p->d.last && p->d.last + arr->size
				<= p->d.end)
		{
			ce_memset(p->d.last,0,arr->size);
			p->d.last += arr->size;
			arr->nalloc++;

		}

		else
		{
			/* 如果当前压入的数据是第一个数据，即压入之前没有分配数组的内存，就分配1个元素的内存
			 * 否则如果当前压入元素之前已经有n个元素，一次性再申请2倍nalloc的空间
			 */
			ce_uint_t new_size = (arr->nalloc <= 0) ? 1 : arr->nalloc * 2;
			u_char *new_data;
			new_data = (u_char*) ce_palloc(arr->pool, arr->size * new_size);
			
			/// 把以前的数据移动到新的空间中  TODO 内存泄漏
			ce_memcpy(new_data,
					arr->elts, arr->nalloc * arr->size);
			ce_memset(new_data + arr->nalloc * arr->size, 0,
					arr->size * (new_size - arr->nalloc));
			arr->elts = new_data;
			arr->nalloc = new_size;
		}
	}
	++arr->nelts;
	
	/// 返回新元素的地址
	return arr->elts + (arr->size * (arr->nelts - 1));
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  同ce_array_push操作，区别在与如果需要新分配内存的话，新分配的内存是没有
 * 		   置0的
 *
 * @param arr	指向动态数组的指针
 *
 * @returns		返回新元素的地址	   
 */
/* ----------------------------------------------------------------------------*/
u_char *ce_array_push_noclear(ce_array_t *arr)
{
	ce_pool_t *p;

	if (arr->nelts == arr->nalloc)
	{
		ce_uint_t size = arr->size * arr->nalloc;

		p = arr->pool;

		if ((u_char*) arr->elts + size == p->d.last && p->d.last + arr->size
				<= p->d.end)
		{
			p->d.last += arr->size;

			arr->nalloc++;

		}

		else
		{
			ce_uint_t new_size = (arr->nalloc <= 0) ? 1 : arr->nalloc * 2;

			u_char *new_data;

			new_data = (u_char*) ce_palloc(arr->pool, arr->size * new_size);

			ce_memcpy(new_data, arr->elts, arr->nalloc * arr->size);

			arr->elts = new_data;

			arr->nalloc = new_size;
		}
	}
	++arr->nelts;

	return arr->elts + (arr->size * (arr->nelts - 1));
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  将dst数组和src数组合并，合并后src追加到dst的末尾
 *
 * @param dst	目的数组的指针
 * @param src	源数组的指针
 */
/* ----------------------------------------------------------------------------*/
void ce_array_cat(ce_array_t *dst, ce_array_t *src)
{
	size_t elt_size = dst->size;
	
	/// 如果dst数组空间不够
	if (dst->nelts + src->nelts > dst->nalloc)
	{
		/* 如果dst中元素个数为0,将产生一个新空间
		 * 如果dst中元素个数不为0,产生2*dst->nalloc的空间
		 */
		size_t new_size = (dst->nalloc <= 0) ? 1 : dst->nalloc * 2;
		u_char *new_data;
		
		/// 可能new_size也不够src的长度，循环增加新空间的大小，直到可以容纳src的长度
		while (dst->nelts + src->nelts > new_size)
		{
			new_size *= 2;
		}
		
		/// 确定新的size之后,申请新size大小的内存空间，将dst的数组拷贝到新的内存空间中
		new_data = (u_char*) ce_pcalloc(dst->pool, elt_size * new_size);
		memcpy(new_data, dst->elts, dst->nalloc * elt_size);

		dst->elts = new_data;
		dst->nalloc = new_size;
	}
	
	/// 将src拷贝到dst之后内存空间中
	memcpy(dst->elts + dst->nelts * elt_size, src->elts, elt_size * src->nelts);

	dst->nelts += src->nelts;
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  该函数的作用是将数组arr拷贝到内存池pool中
 *
 * @param pool	指向内存池的指针
 * @param arr	指向动态数组的指针
 *
 * @returns    返回拷贝后的动态数组指针
 */
/* ----------------------------------------------------------------------------*/
ce_array_t *ce_array_copy(ce_pool_t *pool, ce_array_t *arr)
{
	ce_array_t *res = (ce_array_t *) ce_palloc(pool, sizeof(ce_array_t));
	
	/// 在pool中申请足够放入arr大小的内存空间
	create_array_core(res, pool, arr->nalloc, arr->size, 0);

	ce_memcpy(res->elts, arr->elts, arr->size * arr->nelts);
	res->nelts = arr->nelts;
	memset(res->elts + res->size * res->nelts, 0, res->size * (res->nalloc
			- res->nelts));
	return res;
}

/* This cute function copies the array header *only*, but arranges
 * for the data section to be copied on the first push or arraycat.
 * It's useful when the elements of the array being copied are
 * read only, but new stuff *might* get added on the end; we have the
 * overhead of the full copy only where it is really needed.
 */

/* --------------------------------------------------------------------------*/
/**
 * @brief  提供数组的浅拷贝操作.将arr复制到res中，仅仅复制头部，用于只读的操作
 *
 * @param res	目的数组指针
 * @param arr	源数组指针
 */
/* ----------------------------------------------------------------------------*/
void copy_array_hdr_core(ce_array_t *res, ce_array_t *arr)
{
	res->elts = arr->elts;
	res->size = arr->size;
	res->nelts = arr->nelts;
	res->nalloc = arr->nelts; /* Force overflow on push */
}

/* --------------------------------------------------------------------------*/
/**
 * @brief  同样提供数组的浅拷贝操作，只是参数不同
 *
 * @param pool	指向内存池的指针
 * @param arr	指向源数组指针
 *
 * @returns		拷贝后的目的数组指针   
 */
/* ----------------------------------------------------------------------------*/
ce_array_t *ce_array_copy_hdr(ce_pool_t *pool, ce_array_t *arr)
{
	ce_array_t *res;

	res = (ce_array_t *) ce_palloc(pool, sizeof(ce_array_t));
	res->pool = pool;
	copy_array_hdr_core(res, arr);
	return res;
}

/* The above is used here to avoid consing multiple new array bodies... */

/* --------------------------------------------------------------------------*/
/**
 * @brief  
 *
 * @param pool
 * @param first
 * @param second
 *
 * @returns   
 */
/* ----------------------------------------------------------------------------*/
ce_array_t *ce_array_append(ce_pool_t *pool, ce_array_t *first,
		ce_array_t *second)
{
	ce_array_t *res = ce_array_copy_hdr(pool, first);

	ce_array_cat(res, second);
	return res;
}

/* ce_array_pstrcat generates a new string from the apr_pool_t coceining
 * the concatenated sequence of substrings referenced as elements within
 * the array.  The string will be empty if all substrings are empty or null,
 * or if there are no elements in the array.
 * If sep is non-NUL, it will be inserted between elements as a separator.
 */

/* --------------------------------------------------------------------------*/
/**
 * @brief  将动态数组转化成字符串，如果sep参数不为NULL（0）,在每个元素之间添加
 * 		   sep作为分隔符
 *
 * @param p		指向内存池的指针，函数返回的字符串将在这儿分配内存
 * @param arr	需要转换的动态数组的指针
 * @param sep	分割符
 *
 * @returns		成功返回转化后的字符串
 * 				失败返回NULL
 */
/* ----------------------------------------------------------------------------*/
char* ce_array_pstrcat(ce_pool_t *p, ce_array_t *arr, char sep)
{
	char *cp, *res, **strpp;
	size_t len;
	size_t i;

	if (arr->nelts <= 0 || arr->elts == NULL)
	{ /* Empty table? */
		return (char *) ce_pcalloc(p, 1);
	}

	/* Pass one --- find length of required string */

	len = 0;
	for (i = 0, strpp = (char **) arr->elts;; ++strpp)
	{
		if (strpp && *strpp != NULL)
		{
			len += ce_strlen(*strpp);
		}

		if (++i >= arr->nelts)
		{
			break;
		}
		if (sep)
		{
			++len;
		}
	}

	/* Allocate the required string */

	res = (char *) ce_palloc(p, len + 1);
	cp = res;

	/* Pass two --- copy the argument strings into the result space */

	for (i = 0, strpp = (char **) arr->elts;; ++strpp)
	{
		if (strpp && *strpp != NULL)
		{
			len = ce_strlen(*strpp);
			ce_memcpy(cp, *strpp, len);
			cp += len;
		}

		if (++i >= arr->nelts)

		{
			break;
		}

		if (sep)
		{
			*cp++ = sep;
		}
	}

	*cp = '\0';

	/* Return the result string */

	return res;
}


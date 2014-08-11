/*
 * =====================================================================================
 *
 *       Filename:  ce_alloc.h
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

#ifndef _CE_ALLOC_H
#define _CE_ALLOC_H

#include "ce_basicdefs.h"

#define   ce_free	free

#ifdef __cplusplus
extern "c" {
#endif

extern void *ce_alloc(size_t size);
extern void *ce_calloc(size_t size);
extern void *ce_memalign(size_t alignment, size_t size);
extern void *ce_realloc(void* base,size_t size);

#ifdef __cplusplus
}
#endif

#endif /* _CE_ALLOC_H*/

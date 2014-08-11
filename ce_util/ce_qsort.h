
/*
 * =====================================================================================
 *
 *       Filename:  ce_qsort.h
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

#ifndef _QSORT_H
#define _QSORT_H

#include "ce_basicdefs.h"

typedef ce_int_t (*qsort_cmp)(u_char*, u_char*);

typedef struct st_stack
{
  u_char **low,**high;
} stack_node;

#define PUSH(LOW,HIGH)  {stack_ptr->low = LOW; stack_ptr++->high = HIGH;}
#define POP(LOW,HIGH)   {LOW = (--stack_ptr)->low; HIGH = stack_ptr->high;}

/* The following stack size is enough for ulong ~0 elements */
#define STACK_SIZE	(8 * sizeof(unsigned long int))
#define THRESHOLD_FOR_INSERT_SORT 10

#ifdef __cplusplus
extern "c" {
#endif
extern ce_int_t ce_qsort2(u_char**base_ptr, size_t count,qsort_cmp cmp);
#ifdef __cplusplus
}
#endif

#endif /*_QSORT_H*/

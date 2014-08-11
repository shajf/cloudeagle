/*
 * =====================================================================================
 *
 *       Filename:  ce_qsort.c
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  07/01/2013 13:56:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  CE,2013,CE TEAM
 *
 * =====================================================================================
 */

#include "ce_qsort.h"
#include "ce_alloc.h"
#include "ce_string.h"

static inline void SWAP(u_char **A, u_char **B)			
{							
											
     u_char *tmp = *A; *A = *B; *B = tmp;
} 

static inline void MEDIAN(u_char** low, u_char** mid, u_char** high,qsort_cmp cmp)				
{							
    if (cmp(*high,*low) < 0)				
      SWAP(high, low);			
    if (cmp(*mid, *low) < 0)				
      SWAP(mid, low);			
    else if (cmp(*high, *mid) < 0)			
      SWAP(mid, high);			
}

/****************************************************************************
** Store ranges on stack to avoid recursion
** Use insert sort on small ranges
** Optimize for sorting of pointers 
** Use median comparison to find partition element
*****************************************************************************/
ce_int_t ce_qsort2(u_char **base_ptr, size_t count,qsort_cmp cmp)
{
  u_char **low, **high, **pivot;
  stack_node stack[STACK_SIZE], *stack_ptr;
  if (count <= 1)
    return CE_ERROR;

  low  = base_ptr;
  high =low+ count-1;
  stack_ptr = stack + 1;
  
  stack[0].low=stack[0].high=0;

  pivot =(u_char**)malloc(sizeof(u_char*));
 

  /* The following loop sorts elements between high and low */
  do
  {
    u_char **low_ptr, **high_ptr, **mid;

    count=((size_t) (high - low))+1;
    /* If count is small, then an insert sort is faster than qsort */
    if (count < THRESHOLD_FOR_INSERT_SORT)
    {
      for (low_ptr = low + 1; low_ptr <= high; low_ptr += 1)
      {
	u_char **ptr;
	for (ptr = low_ptr; ptr > low && cmp(*(ptr - 1), *ptr) > 0;
	     ptr -= 1)
	  SWAP(ptr, ptr - 1);
      }
      POP(low, high);
      continue;
    }

    /* Try to find a good middle element */
    mid=low + (count >> 1);
    if (count > 40)				/* Must be bigger than 24 */
    {
      size_t step = count/8;

      MEDIAN(low, low + step, low+step*2,cmp);
      MEDIAN(mid - step, mid, mid+step,cmp);
      MEDIAN(high - 2 * step, high-step, high,cmp);
      /* Put best median in 'mid' */
      MEDIAN(low+step, mid, high-step,cmp);
      low_ptr  = low;
      high_ptr = high;
    }
    else
    {
      MEDIAN(low, mid, high,cmp);
      /* The low and high argument are already in sorted against 'pivot' */
      low_ptr  = low + 1;
      high_ptr = high - 1;
    }
    ce_memcpy(pivot,mid,sizeof(u_char*));
    do
    {
      while (cmp(*low_ptr, *pivot) < 0)
	low_ptr += 1;
      while (cmp(*pivot, *high_ptr) < 0)
	high_ptr -= 1;

      if (low_ptr < high_ptr)
      {
	SWAP(low_ptr, high_ptr);
	low_ptr += 1;
	high_ptr -= 1;
      }
      else 
      {
	if (low_ptr == high_ptr)
	{
	  low_ptr += 1;
	  high_ptr -= 1;
	}
	break;
      }
    }
    while (low_ptr <= high_ptr);

    /*
      Prepare for next iteration.
       Skip partitions of size 1 as these doesn't have to be sorted
       Push the larger partition and sort the smaller one first.
       This ensures that the stack is keept small.
    */

    if ((int) (high_ptr - low) <= 0)
    {
      if ((int) (high - low_ptr) <= 0)
      {
	POP(low, high);			/* Nothing more to sort */
      }
      else
	low = low_ptr;			/* Ignore small left part. */
    }
    else if ((int) (high - low_ptr) <= 0)
      high = high_ptr;			/* Ignore small right part. */
    else if ((high_ptr - low) > (high - low_ptr))
    {
      PUSH(low, high_ptr);		/* Push larger left part */
      low = low_ptr;
    }
    else
    {
      PUSH(low_ptr, high);		/* Push larger right part */
      high = high_ptr;
    }
  } while (stack_ptr > stack);
  
  ce_free(pivot);
  return CE_OK;
}

/*
 * =====================================================================================
 *
 *       Filename:  ce_atomic.h
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
#ifndef CE_ATOMIC_H
#define CE_ATOMIC_H


#include "ce_basicdefs.h"
#include "ce_string.h"
#include "ce_palloc.h"

typedef long ce_atomic_int_t;
typedef unsigned long ce_atomic_uint_t;
typedef ce_atomic_uint_t ce_atomic_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup ce_atomic Atomic Operations
 * @ingroup CE 
 * @{
 */

/**
 * this function is required on some platforms to initialize the
 * atomic operation's internal structures
 * @param p pool
 * @return CE_OK on successful completion
 * @remark Programs do NOT need to call this directly. CE will call this
 *         automatically from ce_initialize.
 * @internal
 */
extern ce_int_t ce_atomic_init(ce_pool_t *p);

/*
 * Atomic operations on -bit values
 * Note: Each of these functions internally implements a memory barrier
 * on platforms that require it
 */

/**
 * atomically read an ce_atomic_t from memory
 * @param mem the pointer
 */
extern ce_atomic_int_t ce_atomic_read(volatile ce_atomic_t *mem);

/**
 * atomically set an ce_atomic_t in memory
 * @param mem pointer to the object
 * @param val value that the object will assume
 */
extern void ce_atomic_set(volatile ce_atomic_t *mem, ce_atomic_int_t val);

/**
 * atomically add 'val' to an ce_atomic_t
 * @param mem pointer to the object
 * @param val amount to add
 * @return old value pointed to by mem
 */
extern ce_atomic_int_t ce_atomic_add(volatile ce_atomic_t *mem, ce_atomic_int_t val);

/**
 * atomically subtract 'val' from an ce_atomic_t
 * @param mem pointer to the object
 * @param val amount to subtract
 */
extern void ce_atomic_sub(volatile ce_atomic_t *mem, ce_atomic_int_t val);

/**
 * atomically increment an ce_atomic_t by 1
 * @param mem pointer to the object
 * @return old value pointed to by mem
 */
extern ce_atomic_int_t ce_atomic_inc(volatile ce_atomic_t *mem);

/**
 * atomically decrement an ce_atomic_t by 1
 * @param mem pointer to the atomic value
 * @return zero if the value becomes zero on decrement, otherwise non-zero
 */
extern ce_atomic_int_t ce_atomic_dec(volatile ce_atomic_t *mem);

/**
 * compare an ce_atomic_t's value with 'cmp'.
 * If they are the same swap the value with 'with'
 * @param mem pointer to the value
 * @param with what to swap it with
 * @param cmp the value to compare it to
 * @return the old value of *mem
 */
extern ce_atomic_int_t ce_atomic_cas(volatile ce_atomic_t *mem, ce_atomic_int_t with,
                              ce_atomic_int_t cmp);

/**
 * exchange an ce_atomic_t's value with 'val'.
 * @param mem pointer to the value
 * @param val what to swap it with
 * @return the old value of *mem
 */
extern ce_atomic_int_t ce_atomic_xchg(volatile ce_atomic_t *mem, ce_atomic_int_t val);

/**
 * compare the pointer's value with cmp.
 * If they are the same swap the value with 'with'
 * @param mem pointer to the pointer
 * @param with what to swap it with
 * @param cmp the value to compare it to
 * @return the old value of the pointer
 */
extern void* ce_atomic_casptr(volatile void **mem, void *with, const void *cmp);

/**
 * exchange a pair of pointer values
 * @param mem pointer to the pointer
 * @param with what to swap it with
 * @return the old value of the pointer
 */
extern void* ce_atomic_xchgptr(volatile void **mem, void *with);

/** @} */

#ifdef __cplusplus
}
#endif

#endif	/* !CE_ATOMIC_H */

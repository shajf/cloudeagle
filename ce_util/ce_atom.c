
/*
 * =====================================================================================
 *
 *       Filename:  ce_atom.c
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
#include "ce_atomic.h"


ce_int_t ce_atomic_init(ce_pool_t *p)
{
    ce_unused(p);
    return CE_OK;
}

ce_atomic_int_t ce_atomic_read(volatile ce_atomic_t *mem)
{
    return *mem;
}

void ce_atomic_set(volatile ce_atomic_t *mem, ce_atomic_int_t val)
{
    *mem = val;
}

ce_atomic_int_t ce_atomic_add(volatile ce_atomic_t *mem, ce_atomic_int_t val)
{
    return __sync_fetch_and_add(mem, val);
}

void ce_atomic_sub(volatile ce_atomic_t *mem, ce_atomic_int_t val)
{
    __sync_fetch_and_sub(mem, val);
}

ce_atomic_int_t ce_atomic_inc(volatile ce_atomic_t *mem)
{
    return __sync_fetch_and_add(mem, 1);
}

ce_atomic_int_t ce_atomic_dec(volatile ce_atomic_t *mem)
{
    return __sync_sub_and_fetch(mem, 1);
}

ce_atomic_int_t ce_atomic_cas(volatile ce_atomic_t *mem, ce_atomic_int_t with,
                                           ce_atomic_int_t cmp)
{
    return __sync_val_compare_and_swap(mem, cmp, with);
}

ce_atomic_int_t ce_atomic_xchg(volatile ce_atomic_t *mem, ce_atomic_int_t val)
{
    __sync_synchronize();

    return __sync_lock_test_and_set(mem, val);
}

void* ce_atomic_casptr(volatile void **mem, void *with, const void *cmp)
{
    return (void*) __sync_val_compare_and_swap(mem, cmp, with);
}

void* ce_atomic_xchgptr(volatile void **mem, void *with)
{
    __sync_synchronize();

    return (void*) __sync_lock_test_and_set(mem, with);
}


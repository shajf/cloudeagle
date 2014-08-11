/*
 *Authors: shajianfeng <csp001314@163.com>
 * */

#ifndef _CSP_KERNEL_H
#define _CSP_KERNEL_H
#ifdef __KERNEL__
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/module.h>
#include <net/netlink.h>
#include <net/net_namespace.h>
#include <linux/netdevice.h>
#include <linux/slab.h>
#else
#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
typedef uint8_t __u8;
typedef uint16_t __u16;
typedef uint32_t __u32;
typedef uint64_t __u64;
typedef int8_t __s8;
typedef int16_t __s16;
typedef int32_t __s32;
typedef int64_t __s64;
#endif

#include <csp_kernel_record.h>
#include <csp_kernel_manager.h>

extern void exit_kman_test(void);
extern void init_kman_test(void);
#endif /*_CSP_KERNEL_H*/

/*
 * =====================================================================================
 *
 *       Filename:  ce_basicdefs.h
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

#ifndef _ce_basicdefs_h
#define _ce_basicdefs_h

/**basic header file**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/mman.h>
#include <bits/mman.h>
#include <dirent.h>
#include <glob.h>
#include <errno.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <assert.h>
/* System headers the network I/O library needs */

#include <sys/uio.h>
#include <sys/select.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#define ce_cdecl
#define ce_libc_cdecl

/**basic types**/
typedef unsigned char ce_byte_t;
typedef short ce_short_t;
typedef u_short ce_ushort_t;
typedef int ce_int_t;
typedef u_int ce_uint_t;
typedef long ce_long_t;
typedef u_long ce_ulong_t;
typedef int8_t ce_int8_t;
typedef uint8_t ce_uint8_t;
typedef int16_t ce_int16_t;
typedef uint16_t ce_uint16_t;
typedef int32_t ce_int32_t;
typedef uint32_t ce_uint32_t;
typedef int64_t ce_int64_t;
typedef uint64_t ce_uint64_t;
typedef float ce_float_t;
typedef double ce_double_t;
typedef int ce_pid_t;

/*forward declearation*/
typedef struct ce_pool_t ce_pool_t;

typedef enum
{
	CE_FALSE, CE_TRUE, CE_NOVAL
} ce_bool_t;
typedef enum
{
	T_POOL,/*temp pool*/
	P_POOL,/*p.. pool*/
	M_POOL,/*M ..POOL*/
	PTN
} pool_type_e;

typedef struct
{
	ce_pool_t* my_pool[PTN];
} ce_my_pool_t;

static inline void ce_set_mypool(ce_my_pool_t *mypool, ce_pool_t *pool,
                                  int type)
{
	(mypool)->my_pool[type] = pool;
}

static inline ce_pool_t *
ce_get_mypool(ce_my_pool_t *mypool, int type)
{
	return (mypool)->my_pool[type];
}

/**basic const values**/
#define CE_USET_NULL	NULL
#define CE_USET_ZERO  0
#define CE_USET_NGONE	-1
#define CE_SIZE_MAX   (~(size_t)0)

#define CE_INT32_LEN	sizeof("-2147483648")-1
#define	CE_INT64_LEN	sizeof("-9223372036854775808")-1
#define	CE_PTR_SIZE	sizeof(void*)
#if ((__GNU__==2)&&(__GNUC_MINOR__<8))
#define CE_MAX_UINT32_VALUE	(uint32_t)0xffffffffLL
#else
#define CE_MAX_UINT32_VALUE	(uint32_t)0xffffffff
#endif
#define CE_INT_MAX		(1<<(8*sizeof(int)-1))-1
#define CE_PAGE_SIZE		4096
#define CE_PAGESIZE_SHIFT	12
#define CE_CACHELINE_SIZE 	512
#define CE_ALIGNMENT		sizeof(void*)
#define CE_MEMSIZE_ALIGNMENT  sizeof(void*)

#define LF	(u_char)10
#define CR      (u_char)13
#define	CRLF	"\x0d\x0a"

#define CE_OK		0
#define CE_ERROR	-1
#define CE_EXIT	-2
#define CE_AGAIN	-3
#define CE_BUSY	-4
#define CE_DONE	-5
#define CE_ENOPOOL	-7
#define CE_EOF	EOF
/**basic micro funs**/
#define ce_align(d,a)		(((d)+(a-1))&~(a-1))
#define ce_align_ptr(p,a)	(u_char*)(((uintptr_t)(p)+((uintptr_t)a-1))&~((uintptr_t)a-1))
#define ce_abs(value)		((value)>=0?(value):-(value))
#define ce_max(a,b)		((a<b)?(b):(a))
#define ce_min(a,b)		((a>b)?(b):(a))
#define CE_EOL_STR              "\r\n"
#define ce_signal_helper(n)     SIG##n
#define ce_signal_value(n)      ce_signal_helper(n)
#define CE_ALIGN_DEFAULT(len)   ce_align(len,sizeof(void*))

#define ce_random               random

#define CE_SHUTDOWN_SIGNAL      QUIT
#define CE_TERMINATE_SIGNAL     TERM
#define CE_RECONFIGURE_SIGNAL   HUP

#define CE_REOPEN_SIGNAL        USR1
#define CE_CHANGEBIN_SIGNAL     USR2

#define CE_OFFSET(p_type,field) \
        ((long) (((char *) (&(((p_type)NULL)->field))) - ((char *) NULL)))

#define CE_OFFSETOF(s_type,field) CE_OFFSET(s_type*,field)

#define ce_value_helper(n)   #n
#define ce_value(n)          ce_value_helper(n)
#define ce_unused(v) ((v)=(v))
#endif /*_ce_basicdefs_h*/


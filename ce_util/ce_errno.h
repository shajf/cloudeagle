/*
 * =====================================================================================
 *
 *       Filename:  ce_errno.h
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

#ifndef _CE_ERRNO_H
#define _CE_ERRNO_H
#include "ce_basicdefs.h"

typedef int               ce_err_t;

#define CE_EPERM         EPERM
#define CE_ENOENT        ENOENT
#define CE_ENOPATH       ENOENT
#define CE_ESRCH         ESRCH
#define CE_EINTR         EINTR
#define CE_ECHILD        ECHILD
#define CE_ENOMEM        ENOMEM
#define CE_EACCES        EACCES
#define CE_EBUSY         EBUSY
#define CE_EEXIST        EEXIST
#define CE_EXDEV         EXDEV
#define CE_ENOTDIR       ENOTDIR
#define CE_EISDIR        EISDIR
#define CE_EINVAL        EINVAL
#define CE_ENOSPC        ENOSPC
#define CE_EPIPE         EPIPE
#define CE_EINPROGRESS   EINPROGRESS
#define CE_EADDRINUSE    EADDRINUSE
#define CE_ECONNABORTED  ECONNABORTED
#define CE_ECONNRESET    ECONNRESET
#define CE_ENOTCONN      ENOTCONN
#define CE_ETIMEDOUT     ETIMEDOUT
#define CE_ECONNREFUSED  ECONNREFUSED
#define CE_ENAMETOOLONG  ENAMETOOLONG
#define CE_ENETDOWN      ENETDOWN
#define CE_ENETUNREACH   ENETUNREACH
#define CE_EHOSTDOWN     EHOSTDOWN
#define CE_EHOSTUNREACH  EHOSTUNREACH
#define CE_ENOSYS        ENOSYS
#define CE_ECANCELED     ECANCELED
#define CE_EILSEQ        EILSEQ
#define CE_ENOMOREFILES  0
#define CE_SYS_NERR 32
#define ce_errno                  errno
#define ce_socket_errno           errno
#define ce_set_errno(err)         errno = err
#define ce_set_socket_errno(err)  errno = err

#ifdef __cplusplus
extern "c"{
#endif /*__cplusplus*/

extern u_char *ce_strerror(ce_err_t err, u_char *errstr, size_t size);

extern ce_uint_t ce_strerror_init(void);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _CE_ERRNO_H_INCLUDED_ */

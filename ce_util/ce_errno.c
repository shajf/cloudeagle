/*
 * =====================================================================================
 *
 *       Filename:  ce_errno.c
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

#include "ce_errno.h"
#include "ce_string.h"

static ce_str_t  *ce_sys_errlist;
static ce_str_t   ce_unknown_error = ce_string("Unknown error");


u_char *ce_strerror(ce_err_t err, 
		      u_char *errstr,
		      size_t size)
{
    ce_str_t  *msg;

    msg = ((ce_uint_t) err < CE_SYS_NERR) ?\
    &ce_sys_errlist[err]:&ce_unknown_error;

    size = ce_min(size, msg->len);

    return ce_cpymem(errstr, msg->data, size);
}


ce_uint_t ce_strerror_init(void)
{
    char       *msg;
    u_char     *p;
    size_t      len;
    ce_err_t   err;

    /*
     * ce_strerror() is not ready to work at this stage, therefore,
     * malloc() is used and possible errors are logged using strerror().
     */

    len = CE_SYS_NERR * sizeof(ce_str_t);

    ce_sys_errlist =(ce_str_t*)malloc(len);
    if (ce_sys_errlist == NULL) {
        goto failed;
    }

    for (err = 0; err < CE_SYS_NERR; err++) {
        msg = strerror(err);
        len = ce_strlen(msg);

        p =(u_char*)malloc(len);
        if (p == NULL) {
            goto failed;
        }

        ce_memcpy(p, msg, len);
        ce_sys_errlist[err].len = len;
        ce_sys_errlist[err].data = p;
    }

    return CE_OK;

failed:

    err = errno;

    return CE_ERROR;
}

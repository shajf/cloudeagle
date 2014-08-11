/*
 * =====================================================================================
 *
 *       Filename:  ce_times.h
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

#ifndef CE_TIMES_H
#define CE_TIMES_H

#include "ce_basicdefs.h"
#include "ce_rbtree.h"
#include "ce_time.h"
#include "ce_string.h"

#ifdef __cplusplus
extern "c" {
#endif

typedef struct 
{
    time_t       sec;
    ce_uint_t  msec;
    ce_int_t   gmtoff;
} ce_time_t;

extern void ce_time_init(void);
extern void ce_time_update(void);

extern void ce_time_sigsafe_update(void);
extern u_char *ce_http_time(u_char *buf, time_t t);
extern u_char *ce_http_cookie_time(u_char *buf, time_t t);
extern void ce_gmtime(time_t t, ce_tm_t *tp);

extern time_t ce_next_time(time_t when);

#define ce_next_time_n      "mktime()"

extern volatile ce_time_t  *ce_cached_time;

extern volatile ce_str_t    ce_cached_err_log_time;

/*
 * milliseconds elapsed since epoch and truncated to ce_msec_t,
 * used in event timers
 */
//extern volatile ce_msec_t  ce_current_msec;


#define ce_time()           ce_cached_time->sec
#define ce_timeofday()      (ce_time_t *)ce_cached_time

#ifdef __cplusplus
}
#endif
#endif /* CE_TIMES_H*/

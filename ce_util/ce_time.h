/*
 * =====================================================================================
 *
 *       Filename:  ce_time.h
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

#ifndef _CE_TIME_H
#define _CE_TIME_H

#include "ce_basicdefs.h"
#include <sys/time.h>
#include <time.h>


typedef struct tm             ce_tm_t;

#define ce_tm_sec            tm_sec
#define ce_tm_min            tm_min
#define ce_tm_hour           tm_hour
#define ce_tm_mday           tm_mday
#define ce_tm_mon            tm_mon
#define ce_tm_year           tm_year
#define ce_tm_wday           tm_wday
#define ce_tm_isdst          tm_isdst

#define ce_tm_sec_t          int
#define ce_tm_min_t          int
#define ce_tm_hour_t         int
#define ce_tm_mday_t         int
#define ce_tm_mon_t          int
#define ce_tm_year_t         int
#define ce_tm_wday_t         int

#define ce_tm_gmtoff         tm_gmtoff
#define ce_tm_zone           tm_zone

#define ce_timezone(isdst) (- (isdst ? timezone + 3600 : timezone) / 60)

#ifdef __cplusplus
extern "c" {
#endif

void ce_timezone_update(void);
void ce_localtime(time_t s, ce_tm_t *tm);
void ce_libc_localtime(time_t s, struct tm *tm);
void ce_libc_gmtime(time_t s, struct tm *tm);


#define ce_gettimeofday(tp)  (void) gettimeofday(tp, NULL);
#define ce_msleep(ms)        (void) usleep(ms * 1000)
#define ce_sleep(s)          (void) sleep(s)
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _CE_TIME_H*/

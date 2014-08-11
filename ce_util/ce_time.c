/*
 * =====================================================================================
 *
 *       Filename:  ce_time.c
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

#include "ce_time.h"

/*
 * Linux does not test /etc/localtime change in localtime(),
 * but may stat("/etc/localtime") several times in every strftime(),
 * therefore we use it to update timezone.
 */

void ce_timezone_update(void)
{
    time_t      s;
    struct tm  *t;
    char        buf[4];

    s = time(0);

    t = localtime(&s);

    strftime(buf, 4, "%H", t);
}


void ce_localtime(time_t s, ce_tm_t *tm)
{
    ce_tm_t  *t;
    t = localtime(&s);
    *tm = *t;
    tm->ce_tm_mon++;
    tm->ce_tm_year += 1900;
}


void ce_libc_localtime(time_t s, struct tm *tm)
{
    struct tm  *t;
    t = localtime(&s);
    *tm = *t;
}


void ce_libc_gmtime(time_t s, struct tm *tm)
{
    struct tm  *t;

    t = gmtime(&s);
    *tm = *t;
}

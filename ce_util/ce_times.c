/*
 * =====================================================================================
 *
 *       Filename:  ce_times.c
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
#include "ce_basicdefs.h"
#include "ce_times.h"
/*
 * The time may be updated by signal handler or by several threads.
 * The time update operations are rare and require to hold the
 * ce_time_lock.
 * The time read operations are frequent, 
 * so they are lock-free and get time
 * values and strings from the current slot.
 * Thus thread may get the corrupted
 * values only if it is preempted while copying and 
 * then it is not scheduled
 * to run more than CE_TIME_SLOTS seconds.
 */

#define CE_TIME_SLOTS   64

static ce_uint_t        slot;
static  pthread_mutex_t    ce_time_lock;

volatile ce_time_t     *ce_cached_time;
volatile ce_str_t       ce_cached_err_log_time;


/*
 * locatime() is not Async-Signal-Safe functions, 
 * therefore,they must not be called by a signal handler, 
 * so we use the cached GMT offset value.
 * Fortunately the value is changed only two times a year.
 */

static ce_int_t         cached_gmtoff;

static ce_time_t        cached_time[CE_TIME_SLOTS];

static u_char            cached_err_log_time[CE_TIME_SLOTS]
                                    [sizeof("1970/09/28 12:00:00")];

#if 0
static char  *week[] = { "Sun", "Mon", "Tue", 
			"Wed", "Thu", "Fri", "Sat" };

static char  *months[] = { "Jan", "Feb", "Mar", 
			   "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", 
			   "Oct", "Nov", "Dec" };
#endif
void ce_time_init(void)
{
    ce_cached_err_log_time.len = \
    sizeof("1970/09/28 12:00:00") - 1;

    ce_cached_time = &cached_time[0];

    ce_time_update();
}


void ce_time_update(void)
{
    u_char          *p1;
    ce_tm_t         tm, gmt;
    time_t           sec;
    ce_uint_t       msec;
    ce_time_t      *tp;
    struct timeval   tv;

    pthread_mutex_init(&ce_time_lock,NULL);

    if (pthread_mutex_trylock(&ce_time_lock)) 
    {
        return;
    }

    ce_gettimeofday(&tv);

    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;


    tp = &cached_time[slot];

    if (tp->sec == sec) 
    {
        tp->msec = msec;
        pthread_mutex_unlock(&ce_time_lock);
        return;
    }

    if (slot == CE_TIME_SLOTS - 1) 
    {
        slot = 0;
    } 
    
    else 
    {
        slot++;
    }

    tp = &cached_time[slot];

    tp->sec = sec;
    tp->msec = msec;

    ce_gmtime(sec, &gmt);



#if (CE_HAVE_GETTIMEZONE)

    tp->gmtoff = ce_gettimezone();
    ce_gmtime(sec + tp->gmtoff * 60, &tm);

#elif (CE_HAVE_GMTOFF)

    ce_localtime(sec, &tm);
    cached_gmtoff = (ce_int_t) (tm.ce_tm_gmtoff / 60);
    tp->gmtoff = cached_gmtoff;

#else

    ce_localtime(sec, &tm);
    cached_gmtoff = ce_timezone(tm.ce_tm_isdst);
    tp->gmtoff = cached_gmtoff;

#endif


    p1 = &cached_err_log_time[slot][0];

    (void) ce_sprintf(p1, "%4d/%02d/%02d %02d:%02d:%02d",
                       tm.ce_tm_year, tm.ce_tm_mon,
                       tm.ce_tm_mday, tm.ce_tm_hour,
                       tm.ce_tm_min, tm.ce_tm_sec);

    ce_cached_time = tp;
    ce_cached_err_log_time.data = p1;
    
    pthread_mutex_unlock(&ce_time_lock);
}

void ce_time_sigsafe_update(void)
{
    u_char          *p;
    ce_tm_t         tm;
    time_t           sec;
    ce_time_t      *tp;
    struct timeval   tv;


   
    if (pthread_mutex_trylock(&ce_time_lock)) 
    {
        return;
    }

    ce_gettimeofday(&tv);

    sec = tv.tv_sec;

    tp = &cached_time[slot];

    if (tp->sec == sec) 
    {

        pthread_mutex_unlock(&ce_time_lock);
        return;
    }

    if (slot == CE_TIME_SLOTS - 1) 
    {
        slot = 0;
    } 
    
    else 
    {
        slot++;
    }

    ce_gmtime(sec + cached_gmtoff * 60, &tm);

    p = &cached_err_log_time[slot][0];

    (void) ce_sprintf(p, "%4d/%02d/%02d %02d:%02d:%02d",
                       tm.ce_tm_year, tm.ce_tm_mon,
                       tm.ce_tm_mday, tm.ce_tm_hour,
                       tm.ce_tm_min, tm.ce_tm_sec);


    ce_cached_err_log_time.data = p;
    pthread_mutex_unlock(&ce_time_lock);
}

void ce_gmtime(time_t t, ce_tm_t *tp)
{
    ce_int_t   yday;
    ce_uint_t  n, sec, min, hour, mday, mon, year, wday, days, leap;

    /* the calculation is valid for positive time_t only */

    n = (ce_uint_t) t;

    days = n / 86400;

    /* Jaunary 1, 1970 was Thursday */

    wday = (4 + days) % 7;

    n %= 86400;
    hour = n / 3600;
    n %= 3600;
    min = n / 60;
    sec = n % 60;

    /*
     * the algorithm based on Gauss' formula,
     * see src/http/ce_http_parse_time.c
     */

    /* days since March 1, 1 BC */
    days = days - (31 + 28) + 719527;

    /*
     * The "days" should be adjusted to 1 only, however, some March 1st's go
     * to previous year, so we adjust them to 2.  This causes also shift of the
     * last Feburary days to next year, but we catch the case when "yday"
     * becomes negative.
     */

    year = (days + 2) * 400 / (365 * 400 + 100 - 4 + 1);

    yday = days - (365 * year + year / 4 - year / 100 + year / 400);

    if (yday < 0) 
    {
        leap = (year % 4 == 0) && (year % 100 || (year % 400 == 0));
        yday = 365 + leap + yday;
        year--;
    }

    /*
     * The empirical formula that maps "yday" to month.
     * There are at least 10 variants, some of them are:
     *     mon = (yday + 31) * 15 / 459
     *     mon = (yday + 31) * 17 / 520
     *     mon = (yday + 31) * 20 / 612
     */

    mon = (yday + 31) * 10 / 306;

    /* the Gauss' formula that evaluates days before the month */

    mday = yday - (367 * mon / 12 - 30) + 1;

    if (yday >= 306) 
    {

        year++;
        mon -= 10;

        /*
         * there is no "yday" in Win32 SYSTEMTIME
         *
         * yday -= 306;
         */

    } 
    
    else 
    {

        mon += 2;

        /*
         * there is no "yday" in Win32 SYSTEMTIME
         *
         * yday += 31 + 28 + leap;
         */
    }

    tp->ce_tm_sec = (ce_tm_sec_t) sec;
    tp->ce_tm_min = (ce_tm_min_t) min;
    tp->ce_tm_hour = (ce_tm_hour_t) hour;
    tp->ce_tm_mday = (ce_tm_mday_t) mday;
    tp->ce_tm_mon = (ce_tm_mon_t) mon;
    tp->ce_tm_year = (ce_tm_year_t) year;
    tp->ce_tm_wday = (ce_tm_wday_t) wday;
}


time_t ce_next_time(time_t when)
{
    time_t     now, next;
    struct tm  tm;

    now = ce_time();

    ce_libc_localtime(now, &tm);

    tm.tm_hour = (int) (when / 3600);
    when %= 3600;
    tm.tm_min = (int) (when / 60);
    tm.tm_sec = (int) (when % 60);

    next = mktime(&tm);

    if (next == -1) 
    {
        return -1;
    }

    if (next - now > 0) 
    {
        return next;
    }

    tm.tm_mday++;

    /* mktime() should normalize a date (Jan 32, etc) */

    next = mktime(&tm);

    if (next != -1) 
    {
        return next;
    }

    return -1;
}

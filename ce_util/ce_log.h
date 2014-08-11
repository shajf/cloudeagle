#ifndef _CE_LOG_H_INCLUDED_
#define _CE_LOG_H_INCLUDED_

#include "ce_basicdefs.h"
#include "ce_file.h"

#define CE_LOG_STDERR            0
#define CE_LOG_EMERG             1
#define CE_LOG_ALERT             2
#define CE_LOG_CRIT              3
#define CE_LOG_ERR               4
#define CE_LOG_WARN              5
#define CE_LOG_NOTICE            6
#define CE_LOG_INFO              7
#define CE_LOG_DEBUG             8

typedef struct {
    ce_uint_t           log_level;
    ce_file_t     file;

    void                *data;
}ce_log_t;


#define CE_MAX_ERROR_STR   2048

#define ce_log(level,fmt,...) ce_log_error_core(level,0,fmt,##__VA_ARGS__)

#define DEBUG_LOG(fmt, ...) ce_log_error_core(CE_LOG_DEBUG,0,fmt,##__VA_ARGS__)

#define INFO_LOG(fmt, ...) ce_log_error_core(CE_LOG_INFO,0,fmt,##__VA_ARGS__)

#define NOTICE_LOG(fmt, ...) ce_log_error_core(CE_LOG_NOTICE,0,fmt,##__VA_ARGS__)

#define WARN_LOG(fmt, ...) ce_log_error_core(CE_LOG_WARN,0,fmt,##__VA_ARGS__)

#define ERR_LOG(fmt, ...) ce_log_error_core(CE_LOG_ERR,0,fmt,##__VA_ARGS__)

#define CRIT_LOG(fmt, ...) ce_log_error_core(CE_LOG_CRIT,0,fmt,##__VA_ARGS__)

#define ALERT_LOG(fmt, ...) ce_log_error_core(CE_LOG_ALERT,0,fmt,##__VA_ARGS__)

#define EMERG_LOG(fmt, ...) ce_log_error_core(CE_LOG_EMERG,0,fmt,##__VA_ARGS__)

void ce_log_error_core(ce_uint_t level,ce_err_t err,
    const char *fmt, ...);


void ce_log_init(u_char *name);

void ce_log_stderr(ce_err_t err, const char *fmt, ...);
u_char *ce_log_errno(u_char *buf, u_char *last, ce_err_t err);


static inline void
ce_write_stderr(char *text)
{
    (void) ce_write_fd(ce_stderr, text, strlen(text));
}



#endif /* _CE_LOG_H_INCLUDED_ */

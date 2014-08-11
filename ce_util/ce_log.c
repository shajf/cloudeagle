#include "ce_log.h"

static ce_log_t log_s,*log=&log_s;

static ce_str_t err_levels[] = {
    ce_null_string,
    ce_string("emerg"),
    ce_string("alert"),
    ce_string("crit"),
    ce_string("error"),
    ce_string("warn"),
    ce_string("notice"),
    ce_string("info"),
    ce_string("debug")
};


void
ce_log_error_core(ce_uint_t level,ce_err_t err,
    const char *fmt, ...)

{
    va_list  args;
    u_char  *p, *last, *msg;
    u_char   errstr[CE_MAX_ERROR_STR];

    if(log->log_level<level||log->file.fd == CE_INVALID_FILE) {
        return;
    }

    last = errstr + CE_MAX_ERROR_STR;

    ce_memcpy(errstr, ce_cached_err_log_time.data,
               ce_cached_err_log_time.len);

    p = errstr + ce_cached_err_log_time.len;

    p = ce_slprintf(p, last, " [%V] ", &err_levels[level]);

    /* pid#tid */
    p = ce_slprintf(p, last, "%P#"": ",
                    getpid());


    msg = p;


    va_start(args, fmt);
    p = ce_vslprintf(p, last, fmt, args);
    va_end(args);

    if (err) {
        p = ce_log_errno(p, last, err);
    }

    if (p > last - CE_LINEFEED_SIZE) {
        p = last - CE_LINEFEED_SIZE;
    }

    ce_linefeed(p);

    (void) ce_write_fd(log->file.fd, errstr, p - errstr);

    if (level > CE_LOG_WARN
        || log->file.fd == ce_stderr)
    {
        return;
    }

    msg -= (6 + err_levels[level].len + 3);

    (void) ce_sprintf(msg, "cwaf: [%V] ", &err_levels[level]);

    (void) ce_write_console(ce_stderr, msg, p - msg);
}


void 
ce_log_stderr(ce_err_t err, const char *fmt, ...)
{
    u_char   *p, *last;
    va_list   args;
    u_char    errstr[CE_MAX_ERROR_STR];

    last = errstr + CE_MAX_ERROR_STR;
    p = errstr + 6;

    ce_memcpy(errstr, "cwaf: ", 6);

    va_start(args, fmt);
    p = ce_vslprintf(p, last, fmt, args);
    va_end(args);

    if (err) {
        p = ce_log_errno(p, last, err);
    }

    if (p > last - CE_LINEFEED_SIZE) {
        p = last - CE_LINEFEED_SIZE;
    }

    ce_linefeed(p);

    (void) ce_write_console(ce_stderr, errstr, p - errstr);
}


u_char *
ce_log_errno(u_char *buf, u_char *last, ce_err_t err)
{
    if (buf > last - 50) {

        /* leave a space for an error code */

        buf = last - 50;
        *buf++ = '.';
        *buf++ = '.';
        *buf++ = '.';
    }

    buf = ce_slprintf(buf, last, " (%d: ", err);

    buf = ce_strerror(err, buf, last - buf);

    if (buf < last) {
        *buf++ = ')';
    }

    return buf;
}


void
ce_log_init(u_char *name)
{

    log->log_level = CE_LOG_NOTICE;


    log->file.fd = ce_open_file(name, CE_FILE_RDWR,
                                     O_CREAT|O_APPEND,
                                    CE_FILE_DEFAULT_ACCESS);

    if (log->file.fd == CE_INVALID_FILE) {
        ce_log_stderr(ce_errno,
                       "[alert] could not open error log file: "
                        "ce_open_file \"%s\" failed", name);

        log->file.fd = ce_stderr;
    }
}


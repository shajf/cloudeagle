/*
* by shajianfeng
*/

#include "ce_line_conf_file.h"

int ce_cfg_closefile(ce_configfile_t *cfp){

    return (cfp->close == NULL) ? 0 : cfp->close(cfp->param);
}

static ce_int_t cfg_close(void *param)
{
    ce_file_t *file = (ce_file_t*)param;

    return ce_close_file(file->fd);
}

static ce_int_t cfg_getch(char *ch, void *param)
{
    ce_file_t *file = (ce_file_t*)param;
    ssize_t rc =  ce_file_getc(ch, param);
    if(rc==0)
	return CE_EOF;
    
    if(rc==CE_ERROR)
	return CE_ERROR;
    return CE_OK;	
}

static ce_int_t cfg_getstr(void *buf, size_t bufsiz, void *param)
{
    ce_file_t *file = (ce_file_t*)param;
    ssize_t rc = ce_file_gets(file,(char*)buf,bufsiz);

    if(rc==0)
	return CE_EOF;
    
    if(rc==CE_ERROR)
	return CE_ERROR;
    return CE_OK;	
}

ce_int_t ce_pcfg_openfile(ce_configfile_t **ret_cfg,ce_pool_t *p, const char *name)
{
    struct stat sb;
    ce_configfile_t *new_cfg;
    ce_file_t *file = NULL;
    ce_int_t status;

    if (name == NULL) {
        return CE_ERROR;
    }
    
    file = (ce_file_t*)ce_palloc(p,sizeof(ce_file_t));
  
    file->fd = ce_open_file(name,CE_FILE_RDONLY,CE_FILE_OPEN,CE_FILE_DEFAULT_ACCESS);
    if(file->fd<=CE_INVALID_FILE){
	return CE_ERROR;
     }	
    file->sys_offset = 0;
    file->offset = 0;


    status = stat(name,&sb);
    if (status != 0)
        return CE_ERROR;

    if (!ce_is_file(&sb) &&
        strcmp(name, "/dev/null") != 0) {
        ce_close_file(file->fd);
        return CE_ERROR;
    }

    new_cfg = ce_palloc(p, sizeof(ce_configfile_t));
    new_cfg->param = (void*)file;
    new_cfg->name = ce_pstrdup(p, name);
    new_cfg->getch = cfg_getch;
    new_cfg->getstr = cfg_getstr;
    new_cfg->close = cfg_close;
    new_cfg->line_number = 0;
    *ret_cfg = new_cfg;
    return CE_OK;
}

/* Allocate a ce_configfile_t handle with user defined functions and params */
ce_configfile_t * ce_pcfg_open_custom(
            ce_pool_t *p, const char *descr, void *param,
            ce_int_t (*getc_func) (char *ch, void *param),
            ce_int_t (*gets_func) (void *buf, size_t bufsize, void *param),
            ce_int_t (*close_func) (void *param))
{
    ce_configfile_t *new_cfg = ce_palloc(p, sizeof(*new_cfg));
    new_cfg->param = param;
    new_cfg->name = descr;
    new_cfg->getch = getc_func;
    new_cfg->getstr = gets_func;
    new_cfg->close = close_func;
    new_cfg->line_number = 0;
    return new_cfg;
}

ce_int_t ce_cfg_getc(char *ch, ce_configfile_t *cfp){

    ssize_t rc = cfp->getch(ch, cfp->param);
    if (rc !=CE_ERROR && *ch == '\n')
        ++cfp->line_number;
    return rc;
}
/* Read one line from open ce_configfile_t, strip LF, increase line number */
/* If custom handler does not define a getstr() function, read char by char */
static ce_int_t ce_cfg_getline_core(char *buf, size_t bufsize,
                                        ce_configfile_t *cfp)
{
    ssize_t rc;
    /* If a "get string" function is defined, use it */
    if (cfp->getstr != NULL) {
        char *cp;
        char *cbuf = buf;
        size_t cbufsize = bufsize;

        while (1) {
            ++cfp->line_number;
            rc = cfp->getstr(cbuf, cbufsize, cfp->param);
            if (rc == CE_EOF) {
                if (cbuf != buf) {
                    *cbuf = '\0';
                    break;
                }

                else {
                    return CE_EOF;
                }
            }
            if (rc != CE_OK) {
                return rc;
            }

            /*
             *  check for line continuation,
             *  i.e. match [^\\]\\[\r]\n only
             */
            cp = cbuf;
            cp += strlen(cp);
            if (cp > cbuf && cp[-1] == '\n') {
                cp--;
                if (cp > cbuf && cp[-1] == '\r')
                    cp--;
                if (cp > cbuf && cp[-1] == '\\') {
                    cp--;
                    /*
                     * line continuation requested -
                     * then remove baccelash and continue
                     */
                    cbufsize -= (cp-cbuf);
                    cbuf = cp;
                    continue;
                }
            }
            else if (cp - buf >= bufsize - 1) {
                return CE_ENOSPC;
            }
            break;
        }
    } else {
        /* No "get string" function defined; read character by character */
        size_t i = 0;

        if (bufsize < 2) {
            /* too small, assume caller is crazy */
            return CE_EINVAL;
        }
        buf[0] = '\0';

        while (1) {
            char c;
            rc = cfp->getch(&c, cfp->param);
            if (rc == CE_EOF) {
                if (i > 0)
                    break;
                else
                    return CE_EOF;
            }
            if (rc != CE_OK)
                return rc;
            if (c == '\n') {
                ++cfp->line_number;
                /* check for line continuation */
                if (i > 0 && buf[i-1] == '\\') {
                    i--;
                    continue;
                }
                else {
                    break;
                }
            }
            else if (i >= bufsize - 2) {
                return CE_ENOSPC;
            }
            buf[i] = c;
            ++i;
        }
        buf[i] = '\0';
    }
    return CE_OK;
}

static int cfg_trim_line(char *buf)
{
    char *start, *end;
    /*
     * Leading and trailing white space is eliminated completely
     */
    start = buf;
    while (ce_isspace(*start))
        ++start;
    /* blast trailing whitespace */
    end = &start[strlen(start)];
    while (--end >= start && ce_isspace(*end))
        *end = '\0';
    /* Zap leading whitespace by shifting */
    if (start != buf)
        memmove(buf, start, end - start + 2);
    return end - start + 1;
}

/* Read one line from open ce_configfile_t, strip LF, increase line number */
/* If custom handler does not define a getstr() function, read char by char */
ce_int_t ce_cfg_getline(char *buf, size_t bufsize,
                                        ce_configfile_t *cfp)
{
    ce_int_t rc = ce_cfg_getline_core(buf, bufsize, cfp);
    if (rc == CE_OK)
        cfg_trim_line(buf);
    return rc;
}

ce_int_t ce_varbuf_cfg_getline(struct ce_varbuf *vb,
                                               ce_configfile_t *cfp,
                                               size_t max_len)
{
    ce_int_t rc;
    size_t new_len;
    vb->strlen = 0;
    *vb->buf = '\0';

    if (vb->strlen == CE_VARBUF_UNKNOWN)
        vb->strlen = strlen(vb->buf);
    if (vb->avail - vb->strlen < 3) {
        new_len = vb->avail * 2;
        if (new_len > max_len)
            new_len = max_len;
        else if (new_len < 3)
            new_len = 3;
        ce_varbuf_grow(vb, new_len);
    }

    for (;;) {
        rc = ce_cfg_getline_core(vb->buf + vb->strlen, vb->avail - vb->strlen, cfp);
        if (rc == CE_ENOSPC || rc == CE_OK)
            vb->strlen += strlen(vb->buf + vb->strlen);
        if (rc != CE_ENOSPC)
            break;
        if (vb->avail >= max_len)
            return CE_ENOSPC;
        new_len = vb->avail * 2;
        if (new_len > max_len)
            new_len = max_len;
        ce_varbuf_grow(vb, new_len);
        --cfp->line_number;
    }
    if (vb->strlen > max_len)
        return CE_ENOSPC;
    if (rc == CE_OK)
        vb->strlen = cfg_trim_line(vb->buf);
    return rc;
}

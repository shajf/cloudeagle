/*by shajianfeng*/
#include "ce_varbuf.h"


const char nul = '\0';
static char * const varbuf_empty = (char *)&nul;

#define VARBUF_SMALL_SIZE 2048
#define VARBUF_MAX_SIZE   (CE_SIZE_MAX - 1)

void ce_varbuf_init(ce_pool_t *pool, struct ce_varbuf *vb,
                                size_t init_size){

    vb->buf = varbuf_empty;
    vb->avail = 0;
    vb->strlen = CE_VARBUF_UNKNOWN;
    vb->pool = pool;

    ce_varbuf_grow(vb, init_size);
}

ce_int_t ce_varbuf_grow(struct ce_varbuf *vb, size_t new_len)
{

    char *new;

    if(vb->strlen != CE_VARBUF_UNKNOWN && vb->avail < vb->strlen)
    {
	return CE_EINVAL;
    }

    if (new_len <= vb->avail)
        return CE_EINVAL;

    if (new_len < 2 * vb->avail && vb->avail < VARBUF_MAX_SIZE/2) {
        /* at least double the size, to avoid repeated reallocations */
        new_len = 2 * vb->avail;
    }
    else if (new_len > VARBUF_MAX_SIZE) {
        return CE_ENOMEM;
    }

    new_len++;  /* add space for trailing \0 */
    if (1) {
        new_len = CE_ALIGN_DEFAULT(new_len);
        new = ce_palloc(vb->pool, new_len);
        if (vb->avail && vb->strlen != 0) {
            
		if (new == vb->buf + vb->avail + 1) {
                /* We are lucky: the new memory lies directly after our old
                 * buffer, we can now use both.
                 */
                vb->avail += new_len;
                return CE_OK;
            }
            else {
                /* copy up to vb->strlen + 1 bytes */
                memcpy(new, vb->buf, vb->strlen == CE_VARBUF_UNKNOWN ?
                                     vb->avail + 1 : vb->strlen + 1);
            }
        }
        else {
            *new = '\0';
        }
        vb->avail = new_len - 1;
        vb->buf = new;
        return CE_OK;
    }

}

void ce_varbuf_free(struct ce_varbuf *vb){
	
	if(vb->pool){
		ce_destroy_pool(vb->pool);
	}
}

void ce_varbuf_strmemcat(struct ce_varbuf *vb, const char *str,
                                     int len)
{

    if (len == 0)
        return;
    
    if (!vb->avail) {
        ce_varbuf_grow(vb, len);
        memcpy(vb->buf, str, len);
        vb->buf[len] = '\0';
        vb->strlen = len;
        return;
    }

    if (vb->strlen == CE_VARBUF_UNKNOWN)
        vb->strlen = strlen(vb->buf);

    ce_varbuf_grow(vb, vb->strlen + len);
    memcpy(vb->buf + vb->strlen, str, len);
    vb->strlen += len;
    vb->buf[vb->strlen] = '\0';
}

char * ce_varbuf_pdup(ce_pool_t *p, struct ce_varbuf *vb,
                                  const char *prepend, size_t prepend_len,
                                  const char *append, size_t append_len,
                                  size_t *new_len)
{

    size_t i = 0;
    struct iovec vec[3];

    if (prepend) {
        vec[i].iov_base = (void *)prepend;
        vec[i].iov_len = prepend_len;
        i++;
    }
    if (vb->avail && vb->strlen) {
        if (vb->strlen == CE_VARBUF_UNKNOWN)
            vb->strlen = strlen(vb->buf);
        vec[i].iov_base = (void *)vb->buf;
        vec[i].iov_len = vb->strlen;
        i++;
    }
    if (append) {
        vec[i].iov_base = (void *)append;
        vec[i].iov_len = append_len;
        i++;
    }
    if (i)
        return ce_pstrcatv(p, vec, i, new_len);

    if (new_len)
        *new_len = 0;
    return "";

}

/* This function substitutes for $0-$9, filling in regular expression
 * submatches. Pass it the same nmatch and pmatch arguments that you
 * passed ce_regexec(). pmatch should not be greater than the maximum number
 * of subexpressions - i.e. one more than the re_nsub member of ce_regex_t.
 *
 * nmatch must be <=CE_MAX_REG_MATCH (10).
 *
 * input should be the string with the $-expressions, source should be the
 * string that was matched against.
 *
 * It returns the substituted string, or NULL if a vbuf is used.
 * On errors, returns the orig string.
 *
 * Parts of this code are based on Henry Spencer's regsub(), from his
 * AT&T V8 regexp package.
 */

static ce_int_t regsub_core(ce_pool_t *p, char **result,
                                struct ce_varbuf *vb, const char *input,
                                const char *source, size_t nmatch,
                                ce_regmatch_t pmatch[], size_t maxlen)
{
    const char *src = input;
    char *dst;
    char c;
    size_t no;
    size_t len = 0;

    assert((result && p && !vb) || (vb && !p && !result));

    if (!source || nmatch>CE_MAX_REG_MATCH)
        return CE_EINVAL;
    if (!nmatch) {
        len = strlen(src);
        if (maxlen > 0 && len >= maxlen)
            return CE_ENOMEM;
        if (!vb) {
            *result = ce_pstrmemdup(p, src, len);
            return CE_OK;
        }
        else {
            ce_varbuf_strmemcat(vb, src, len);
            return CE_OK;
        }
    }

    /* First pass, find the size */
    while ((c = *src++) != '\0') {
        if (c == '$' && ce_isdigit(*src))
            no = *src++ - '0';
        else
            no = CE_MAX_REG_MATCH;

        if (no >= CE_MAX_REG_MATCH) {  /* Ordinary character. */
            if (c == '\\' && *src)
                src++;
            len++;
        }
        else if (no < nmatch && pmatch[no].rm_so < pmatch[no].rm_eo) {
            if (CE_SIZE_MAX - len <= pmatch[no].rm_eo - pmatch[no].rm_so)
                return CE_ENOMEM;
            len += pmatch[no].rm_eo - pmatch[no].rm_so;
        }

    }

    if (len >= maxlen && maxlen > 0)
        return CE_ENOMEM;

    if (!vb) {
        *result = dst = ce_palloc(p, len + 1);
    }
    else {
        if (vb->strlen == CE_VARBUF_UNKNOWN)
            vb->strlen = strlen(vb->buf);
        ce_varbuf_grow(vb, vb->strlen + len);
        dst = vb->buf + vb->strlen;
        vb->strlen += len;
    }

    /* Now actually fill in the string */

    src = input;

    while ((c = *src++) != '\0') {
        if (c == '$' && ce_isdigit(*src))
            no = *src++ - '0';
        else
            no = CE_MAX_REG_MATCH;

        if (no >= CE_MAX_REG_MATCH) {  /* Ordinary character. */
            if (c == '\\' && *src)
                c = *src++;
            *dst++ = c;
        }
        else if (no < nmatch && pmatch[no].rm_so < pmatch[no].rm_eo) {
            len = pmatch[no].rm_eo - pmatch[no].rm_so;
            memcpy(dst, source + pmatch[no].rm_so, len);
            dst += len;
        }

    }
    *dst = '\0';

    return CE_OK;
}


ce_int_t ce_varbuf_regsub(struct ce_varbuf *vb,
                                          const char *input,
                                          const char *source,
                                          size_t nmatch,
                                          ce_regmatch_t pmatch[],
                                          size_t maxlen)
{
	return regsub_core(NULL, NULL, vb, input, source, nmatch, pmatch, maxlen);
}

ce_int_t ce_varbuf_cfg_getline(struct ce_varbuf *vb,
                                               ce_configfile_t *cfp,
                                               size_t max_len);

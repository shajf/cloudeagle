/*
 * =====================================================================================
 *
 *       Filename:  ce_regex.c
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

#include "ce_regex.h"
#include "ce_string.h"

#ifndef POSIX_MALLOC_THRESHOLD
#define POSIX_MALLOC_THRESHOLD (10)
#endif

/* Table of error strings corresponding to POSIX error codes; must be
 * kept in synch with include/ce_regex.h's CE_REG_E* definitions.
 */

static const char *const pstring[] = {
    "",                         /* Dummy for value 0 */
    "internal error",           /* CE_REG_ASSERT */
    "failed to get memory",     /* CE_REG_ESPACE */
    "bad argument",             /* CE_REG_INVARG */
    "match failed"              /* CE_REG_NOMATCH */
};

size_t ce_regerror(int errcode, const ce_regex_t *preg,
                                   char *errbuf, size_t errbuf_size)
{
    const char *message, *addmessage;
    size_t length, addlength;

    message = (errcode >= (int)(sizeof(pstring) / sizeof(char *))) ?
              "unknown error code" : pstring[errcode];
    length = strlen(message) + 1;

    addmessage = " at offset ";
    addlength = (preg != NULL && (int)preg->re_erroffset != -1) ?
                strlen(addmessage) + 6 : 0;

    if (errbuf_size > 0) {
        if (addlength > 0 && errbuf_size >= length + addlength)
            ce_snprintf(errbuf, errbuf_size, "%s%s%-6d", message, addmessage,
                         (int)preg->re_erroffset);
        else
            ce_cpystrn(errbuf, message, errbuf_size);
    }

    return length + addlength;
}




/*************************************************
 *           Free store held by a regex          *
 *************************************************/

void ce_regfree(ce_regex_t *preg)
{
    (pcre_free)(preg->re_pcre);
}




/*************************************************
 *            Compile a regular expression       *
 *************************************************/

/*
 * Arguments:
 *  preg        points to a structure for recording the compiled expression
 *  pattern     the pattern to compile
 *  cflags      compilation flags
 *
 * Returns:      0 on success
 *               various non-zero codes on failure
*/
int ce_regcomp(ce_regex_t * preg, const char *pattern, int cflags)
{
    const char *errorptr;
    int erroffset;
    int options = 0;

    if ((cflags & CE_REG_ICASE) != 0)
        options |= PCRE_CASELESS;
    if ((cflags & CE_REG_NEWLINE) != 0)
        options |= PCRE_MULTILINE;
    if ((cflags & CE_REG_DOTALL) != 0)
        options |= PCRE_DOTALL;

    preg->re_pcre =
        pcre_compile(pattern, options, &errorptr, &erroffset, NULL);
    preg->re_erroffset = erroffset;

    if (preg->re_pcre == NULL)
        return CE_REG_INVARG;

    pcre_fullinfo((const pcre *)preg->re_pcre, NULL,
                   PCRE_INFO_CAPTURECOUNT, &(preg->re_nsub));
    return 0;
}




/*************************************************
 *              Match a regular expression       *
 *************************************************/

/* Unfortunately, PCRE requires 3 ints of working space for each ccetured
 * substring, so we have to get and release working store instead of just using
 * the POSIX structures as was done in earlier releases when PCRE needed only 2
 * ints. However, if the number of possible cceturing brackets is small, use a
 * block of store on the stack, to reduce the use of malloc/free. The threshold
 * is in a macro that can be changed at configure time.
 */
int ce_regexec(const ce_regex_t *preg, const char *string,
                           size_t nmatch, ce_regmatch_t *pmatch,
                           int eflags)
{
    return ce_regexec_len(preg, string, strlen(string), nmatch, pmatch,
                          eflags);
}

int ce_regexec_len(const ce_regex_t *preg, const char *buff,
                               size_t len, size_t nmatch,
                               ce_regmatch_t *pmatch, int eflags)
{
    int rc;
    int options = 0;
    int *ovector = NULL;
    int small_ovector[POSIX_MALLOC_THRESHOLD * 3];
    int allocated_ovector = 0;

    if ((eflags & CE_REG_NOTBOL) != 0)
        options |= PCRE_NOTBOL;
    if ((eflags & CE_REG_NOTEOL) != 0)
        options |= PCRE_NOTEOL;

    ((ce_regex_t *)preg)->re_erroffset = (size_t)(-1);    /* Only has meaning after compile */

    if (nmatch > 0) {
        if (nmatch <= POSIX_MALLOC_THRESHOLD) {
            ovector = &(small_ovector[0]);
        }
        else {
            ovector = (int *)malloc(sizeof(int) * nmatch * 3);
            if (ovector == NULL)
                return CE_REG_ESPACE;
            allocated_ovector = 1;
        }
    }

    rc = pcre_exec((const pcre *)preg->re_pcre, NULL, buff, (int)len,
                   0, options, ovector, nmatch * 3);

    if (rc == 0)
        rc = nmatch;            /* All ccetured slots were filled in */

    if (rc >= 0) {
        size_t i;
        for (i = 0; i < (size_t)rc; i++) {
            pmatch[i].rm_so = ovector[i * 2];
            pmatch[i].rm_eo = ovector[i * 2 + 1];
        }
        if (allocated_ovector)
            free(ovector);
        for (; i < nmatch; i++)
            pmatch[i].rm_so = pmatch[i].rm_eo = -1;
        return 0;
    }

    else {
        if (allocated_ovector)
            free(ovector);
        switch (rc) {
        case PCRE_ERROR_NOMATCH:
            return CE_REG_NOMATCH;
        case PCRE_ERROR_NULL:
            return CE_REG_INVARG;
        case PCRE_ERROR_BADOPTION:
            return CE_REG_INVARG;
        case PCRE_ERROR_BADMAGIC:
            return CE_REG_INVARG;
        case PCRE_ERROR_UNKNOWN_NODE:
            return CE_REG_ASSERT;
        case PCRE_ERROR_NOMEMORY:
            return CE_REG_ESPACE;
#ifdef PCRE_ERROR_MATCHLIMIT
        case PCRE_ERROR_MATCHLIMIT:
            return CE_REG_ESPACE;
#endif
#ifdef PCRE_ERROR_BADUTF8
        case PCRE_ERROR_BADUTF8:
            return CE_REG_INVARG;
#endif
#ifdef PCRE_ERROR_BADUTF8_OFFSET
        case PCRE_ERROR_BADUTF8_OFFSET:
            return CE_REG_INVARG;
#endif
        default:
            return CE_REG_ASSERT;
        }
    }
}

static ce_int_t rxplus_cleanup(void *preg)
{
    ce_regfree((ce_regex_t *) preg);
    return CE_OK;
}

ce_rxplus_t* ce_rxplus_compile(ce_pool_t *pool,
                                           const char *pattern)
{
    /* perl style patterns
     * add support for more as and when wanted
     * substitute: s/rx/subs/
     * match: m/rx/ or just /rx/
     */

    /* allow any nonalnum delimiter as first or second char.
     * If we ever use this with non-string pattern we'll need an extra check
     */
    const char *endp = 0;
    const char *str = pattern;
    const char *rxstr;
    ce_rxplus_t *ret = ce_pcalloc(pool, sizeof(ce_rxplus_t));
    char delim = 0;
    enum { SUBSTITUTE = 's', MATCH = 'm'} action = MATCH;
    if (!ce_isalnum(pattern[0])) {
        delim = *str++;
    }
    else if (pattern[0] == 's' && !ce_isalnum(pattern[1])) {
        action = SUBSTITUTE;
        delim = pattern[1];
        str += 2;
    }
    else if (pattern[0] == 'm' && !ce_isalnum(pattern[1])) {
        delim = pattern[1];
        str += 2;
    }
    /* TODO: support perl's after/before */
    /* FIXME: fix these simplminded delims */

    /* we think there's a delimiter.  Allow for it not to be if unmatched */
    if (delim) {
        endp = ce_strchr(str, delim);
    }
    if (!endp) { /* there's no delim  or flags */
        if (ce_regcomp(&ret->rx, pattern, 0) == 0) {
            //ce_pool_cleanup_register(pool, &ret->rx, rxplus_cleanup,
              //                        ce_pool_cleanup_null);
            return ret;
        }
        else {
            return NULL;
        }
    }

    /* We have a delimiter.  Use it to extract the regexp */
    rxstr = ce_pstrldup(pool, str, endp-str);

    /* If it's a substitution, we need the replacement string
     * TODO: possible future enhancement - support other parsing
     * in the replacement string.
     */
    if (action == SUBSTITUTE) {
        str = endp+1;
        if (!*str || (endp = ce_strchr(str, delim), !endp)) {
            /* missing replacement string is an error */
            return NULL;
        }
        ret->subs = ce_pstrldup(pool, str, (endp-str));
    }

    /* anything after the current delimiter is flags */
    while (*++endp) {
        switch (*endp) {
        case 'i': ret->flags |= CE_REG_ICASE; break;
        case 'm': ret->flags |= CE_REG_NEWLINE; break;
        case 'n': ret->flags |= CE_REG_NOMEM; break;
        case 'g': ret->flags |= CE_REG_MULTI; break;
        case 's': ret->flags |= CE_REG_DOTALL; break;
        case '^': ret->flags |= CE_REG_NOTBOL; break;
        case '$': ret->flags |= CE_REG_NOTEOL; break;
        default: break; /* we should probably be stricter here */
        }
    }
    if (ce_regcomp(&ret->rx, rxstr, ret->flags) != 0) {
        return NULL;
        /*ce_pool_cleanup_register(pool, &ret->rx, rxplus_cleanup,
                                  ce_pool_cleanup_null);*/
    }
    if (!(ret->flags & CE_REG_NOMEM)) {
        /* count size of memory required, starting at 1 for the whole-match
         * Simpleminded should be fine 'cos regcomp already checked syntax
         */
        ret->nmatch = 1;
        while (*rxstr) {
            switch (*rxstr++) {
            case '\\':  /* next char is escaped - skip it */
                if (*rxstr != 0) {
                    ++rxstr;
                }
                break;
            case '(':   /* unescaped bracket implies memory */
                ++ret->nmatch;
                break;
            default:
                break;
            }
        }
        ret->pmatch = ce_palloc(pool, ret->nmatch*sizeof(ce_regmatch_t));
    }
    return ret;
}

static ce_int_t _regsub_core(ce_pool_t *p, char **result,
                                const char *input,
                                const char *source, size_t nmatch,
                                ce_regmatch_t pmatch[], size_t maxlen)
{
    const char *src = input;
    char *dst;
    char c;
    size_t no;
    size_t len = 0;


    if (!source || nmatch>CE_MAX_REG_MATCH)
        return CE_ERROR;

    if (!nmatch) {
        len = strlen(src);
        if (maxlen > 0 && len >= maxlen)
            return CE_ERROR;
            
	    *result = ce_pstrmemdup(p, src, len);
            return CE_OK;
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
                return CE_ERROR;
            len += pmatch[no].rm_eo - pmatch[no].rm_so;
        }

    }

    if (len >= maxlen && maxlen > 0)
        return CE_ERROR;

    *result = dst = ce_palloc(p, len + 1);

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
static char * _ce_pregsub(ce_pool_t *p, const char *input,
                              const char *source, size_t nmatch,
                              ce_regmatch_t pmatch[])
{
    char *result;
    ce_int_t rc = _regsub_core(p, &result, input, source, nmatch,
                                  pmatch, CE_PREGSUB_MAXLEN);
    if (rc != CE_OK)
        result = NULL;
    return result;
}

int ce_rxplus_exec(ce_pool_t *pool, ce_rxplus_t *rx,
                               const char *pattern, char **newpattern)
{
    int ret = 1;
    int startl, oldl, newl, diffsz;
    const char *remainder;
    char *subs;
/* snrf process_regexp from mod_headers */
    if (ce_regexec(&rx->rx, pattern, rx->nmatch, rx->pmatch, rx->flags) != 0) {
        rx->match = NULL;
        return 0; /* no match, nothing to do */
    }
    rx->match = pattern;
    if (rx->subs) {
        *newpattern = _ce_pregsub(pool, rx->subs, pattern,
                                 rx->nmatch, rx->pmatch);
        if (!*newpattern) {
            return 0; /* FIXME - should we do more to handle error? */
        }
        startl = rx->pmatch[0].rm_so;
        oldl = rx->pmatch[0].rm_eo - startl;
        newl = strlen(*newpattern);
        diffsz = newl - oldl;
        remainder = pattern + startl + oldl;
        if (rx->flags & CE_REG_MULTI) {
            /* recurse to do any further matches */
            char *subs;
            ret += ce_rxplus_exec(pool, rx, remainder, &subs);
            if (ret > 1) {
                /* a further substitution happened */
                diffsz += strlen(subs) - strlen(remainder);
                remainder = subs;
            }
        }
        subs  = ce_palloc(pool, strlen(pattern) + 1 + diffsz);
        memcpy(subs, pattern, startl);
        memcpy(subs+startl, *newpattern, newl);
        strcpy(subs+startl+newl, remainder);
        *newpattern = subs;
    }
    return ret;
}

/* If this blows up on you, see the notes in the header/apidoc
 * rx->match is a pointer and it's your responsibility to ensure
 * it hasn't gone out-of-scope since the last ce_rxplus_exec
 */
void ce_rxplus_match(ce_rxplus_t *rx, int n, int *len,
                                 const char **match)
{
    if (n >= 0 && n < ce_rxplus_nmatch(rx)) {
        *match = rx->match + rx->pmatch[n].rm_so;
        *len = rx->pmatch[n].rm_eo - rx->pmatch[n].rm_so;
    }
    else {
        *len = -1;
        *match = NULL;
    }
}

char* ce_rxplus_pmatch(ce_pool_t *pool, ce_rxplus_t *rx, int n)
{
    int len;
    const char *match;
    ce_rxplus_match(rx, n, &len, &match);
    return (match != NULL) ? ce_pstrldup(pool, match, len) : NULL;
}


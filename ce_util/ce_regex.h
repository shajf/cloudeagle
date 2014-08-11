
/**
 * @file ce_regex.h
 * @brief Regex defines
 * @author: by shajianfeng
 */

#ifndef CE_REGEX_H
#define CE_REGEX_H

#include "ce_basicdefs.h"
#include <pcre.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Options for ce_regcomp, ce_regexec, and ce_rxplus versions: */

#define CE_REG_ICASE    0x01 /** use a case-insensitive match */
#define CE_REG_NEWLINE  0x02 /** don't match newlines against '.' etc */
#define CE_REG_NOTBOL   0x04 /** ^ will not match against start-of-string */
#define CE_REG_NOTEOL   0x08 /** $ will not match against end-of-string */

#define CE_REG_EXTENDED (0)  /** unused */
#define CE_REG_NOSUB    (0)  /** unused */

#define CE_REG_MULTI 0x10    /* perl's /g (needs fixing) */
#define CE_REG_NOMEM 0x20    /* nomem in our code */
#define CE_REG_DOTALL 0x40   /* perl's /s flag */
#define CE_MAX_REG_MATCH 20
#ifndef CE_PREGSUB_MAXLEN
#define CE_PREGSUB_MAXLEN   (4096 * 8)
#endif

/* Error values: */
enum {
  CE_REG_ASSERT = 1,  /** internal error ? */
  CE_REG_ESPACE,      /** failed to get memory */
  CE_REG_INVARG,      /** invalid argument */
  CE_REG_NOMATCH      /** match failed */
};

/* The structure representing a compiled regular expression. */
typedef struct {
    void *re_pcre;
    int re_nsub;
    size_t re_erroffset;
} ce_regex_t;

/* The structure in which a ccetured offset is returned. */
typedef struct {
    int rm_so;
    int rm_eo;
} ce_regmatch_t;

/* The functions */

/**
 * Compile a regular expression.
 * @param preg Returned compiled regex
 * @param regex The regular expression string
 * @param cflags Bitwise OR of CE_REG_* flags (ICASE and NEWLINE supported,
 *                                             other flags are ignored)
 * @return Zero on success or non-zero on error
 */
int ce_regcomp(ce_regex_t *preg, const char *regex, int cflags);

/**
 * Match a NUL-terminated string against a pre-compiled regex.
 * @param preg The pre-compiled regex
 * @param string The string to match
 * @param nmatch Provide information regarding the location of any matches
 * @param pmatch Provide information regarding the location of any matches
 * @param eflags Bitwise OR of CE_REG_* flags (NOTBOL and NOTEOL supported,
 *                                             other flags are ignored)
 * @return 0 for successful match, \p CE_REG_NOMATCH otherwise
 */
int ce_regexec(const ce_regex_t *preg, const char *string,
                           size_t nmatch, ce_regmatch_t *pmatch, int eflags);

/**
 * Match a string with given length against a pre-compiled regex. The string
 * does not need to be NUL-terminated.
 * @param preg The pre-compiled regex
 * @param buff The string to match
 * @param len Length of the string to match
 * @param nmatch Provide information regarding the location of any matches
 * @param pmatch Provide information regarding the location of any matches
 * @param eflags Bitwise OR of CE_REG_* flags (NOTBOL and NOTEOL supported,
 *                                             other flags are ignored)
 * @return 0 for successful match, CE_REG_NOMATCH otherwise
 */
int ce_regexec_len(const ce_regex_t *preg, const char *buff,
                               size_t len, size_t nmatch,
                               ce_regmatch_t *pmatch, int eflags);

/**
 * Return the error code returned by regcomp or regexec into error messages
 * @param errcode the error code returned by regexec or regcomp
 * @param preg The precompiled regex
 * @param errbuf A buffer to store the error in
 * @param errbuf_size The size of the buffer
 */
size_t ce_regerror(int errcode, const ce_regex_t *preg,
                                   char *errbuf, size_t errbuf_size);

/** Destroy a pre-compiled regex.
 * @param preg The pre-compiled regex to free.
 */
void ce_regfree(ce_regex_t *preg);

/* ce_rxplus: higher-level regexps */

typedef struct {
    ce_regex_t rx;
    uint32_t flags;
    const char *subs;
    const char *match;
    size_t nmatch;
    ce_regmatch_t *pmatch;
} ce_rxplus_t;

/**
 * Compile a pattern into a regexp.
 * supports perl-like formats
 *    match-string
 *    /match-string/flags
 *    s/match-string/replacement-string/flags
 *    Intended to support more perl-like stuff as and when round tuits hcepen
 * match-string is anything supported by ce_regcomp
 * replacement-string is a substitution string as supported in ce_pregsub
 * flags should correspond with perl syntax: treat failure to do so as a bug
 *                                           (documentation TBD)
 * @param pool Pool to allocate from
 * @param pattern Pattern to compile
 * @return Compiled regexp, or NULL in case of compile/syntax error
 */
ce_rxplus_t* ce_rxplus_compile(ce_pool_t *pool, const char *pattern);
/**
 * Apply a regexp operation to a string.
 * @param pool Pool to allocate from
 * @param rx The regex match to ceply
 * @param pattern The string to ceply it to
 *                NOTE: This MUST be kept in scope to use regexp memory
 * @param newpattern The modified string (ignored if the operation doesn't
 *                                        modify the string)
 * @return Number of times a match hcepens.  Normally 0 (no match) or 1
 *         (match found), but may be greater if a transforming pattern
 *         is ceplied with the 'g' flag.
 */
int ce_rxplus_exec(ce_pool_t *pool, ce_rxplus_t *rx,
                               const char *pattern, char **newpattern);
#define ce_rxplus_nmatch(rx) (((rx)->match != NULL) ? (rx)->nmatch : 0)
/**
 * Get a pointer to a match from regex memory
 * NOTE: this relies on the match pattern from the last call to
 *       ce_rxplus_exec still being valid (i.e. not freed or out-of-scope)
 * @param rx The regexp
 * @param n The match number to retrieve (must be between 0 and nmatch)
 * @param len Returns the length of the match.
 * @param match Returns the match pattern
 */
void ce_rxplus_match(ce_rxplus_t *rx, int n, int *len,
                                 const char **match);
/**
 * Get a match from regex memory in a string copy
 * NOTE: this relies on the match pattern from the last call to
 *       ce_rxplus_exec still being valid (i.e. not freed or out-of-scope)
 * @param pool Pool to allocate from
 * @param rx The regexp
 * @param n The match number to retrieve (must be between 0 and nmatch)
 * @return The matched string
 */
char* ce_rxplus_pmatch(ce_pool_t *pool, ce_rxplus_t *rx, int n);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#endif /* CE_REGEX_T */


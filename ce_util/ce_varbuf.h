
/*by shajianfeng*/

#ifndef CE_VARBUF_H
#define CE_VARBUF_H

#include "ce_basicdefs.h"
#include "ce_palloc.h"
#include "ce_string.h"
#include "ce_regex.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CE_VARBUF_UNKNOWN CE_SIZE_MAX

/** A resizable buffer */
struct ce_varbuf {
    /** the actual buffer; will point to a const '\0' if avail == 0 and
     *  to memory of the same lifetime as the pool otherwise */
    char *buf;

    /** allocated size of the buffer (minus one for the final \0);
     *  must only be changed using ce_varbuf_grow() */
    size_t avail;

    /** length of string in buffer, or CE_VARBUF_UNKNOWN. This determines how
     *  much memory is copied by ce_varbuf_grow() and where
     *  ce_varbuf_strmemcat() will append to the buffer. */
    size_t strlen;

    /** the pool for memory allocations and for registering the cleanup;
     *  the buffer memory will be released when this pool is cleared */
    ce_pool_t *pool;

};

/** initialize a resizable buffer. It is safe to re-initialize a prevously
 *  used ce_varbuf. The old buffer will be released when the corresponding
 *  pool is cleared. The buffer remains usable until the pool is cleared,
 *  even if the ce_varbuf was located on the stack and has gone out of scope.
 * @param pool the pool to allocate small buffers from and to register the
 *        cleanup with
 * @param vb pointer to the ce_varbuf struct
 * @param init_size the initial size of the buffer (see ce_varbuf_grow() for details)
 */
void ce_varbuf_init(ce_pool_t *pool, struct ce_varbuf *vb,
                                size_t init_size);

/** grow a resizable buffer. If the vb->buf cannot be grown in place, it will
 *  be reallocated and the first vb->strlen + 1 bytes of memory will be copied
 *  to the new location. If vb->strlen == CE_VARBUF_UNKNOWN, the whole buffer
 *  is copied.
 * @param vb pointer to the ce_varbuf struct
 * @param new_size the minimum new size of the buffer
 * @note ce_varbuf_grow() will usually at least double vb->buf's size with
 *       every invocation in order to reduce reallications.
 * @note ce_varbuf_grow() will use pool memory for small and allocator
 *       mem nodes for larger allocations.
 * @note ce_varbuf_grow() will call vb->pool's abort function if out of memory.
 */
ce_int_t ce_varbuf_grow(struct ce_varbuf *vb, size_t new_size);

/** Release memory from a ce_varbuf immediately, if possible.
 *  This allows to free large buffers before the corresponding pool is
 *  cleared. Only larger allocations using mem nodes will be freed.
 * @param vb pointer to the ce_varbuf struct
 * @note After ce_varbuf_free(), vb must not be used unless ce_varbuf_init()
 *       is called again.
 */
void ce_varbuf_free(struct ce_varbuf *vb);

/** Concatenate a string to an ce_varbuf. vb->strlen determines where
 * the string is appended in the buffer. If vb->strlen == CE_VARBUF_UNKNOWN,
 * the string will be appended at the first NUL byte in the buffer.
 * If len == 0, ce_varbuf_strmemcat() does nothing.
 * @param vb pointer to the ce_varbuf struct
 * @param str the string to append; must be at least len bytes long
 * @param len the number of characters of *str to concatenate to the buf
 * @note vb->strlen will be set to the length of the new string
 * @note if len != 0, vb->buf will always be NUL-terminated
 */
void ce_varbuf_strmemcat(struct ce_varbuf *vb, const char *str,
                                     int len);

/** Duplicate an ce_varbuf's content into pool memory
 * @param p the pool to allocate from
 * @param vb the ce_varbuf to copy from
 * @param prepend an optional buffer to prepend (may be NULL)
 * @param prepend_len length of prepend
 * @param append an optional buffer to append (may be NULL)
 * @param append_len length of append
 * @param new_len where to store the length of the resulting string
 *        (may be NULL)
 * @return the new string
 * @note ce_varbuf_pdup() uses vb->strlen to determine how much memory to
 *       copy. It worce even if 0-bytes are embedded in vb->buf, prepend, or
 *       append.
 * @note If vb->strlen equals CE_VARBUF_UNKNOWN, it will be set to
 *       strlen(vb->buf).
 */
char * ce_varbuf_pdup(ce_pool_t *p, struct ce_varbuf *vb,
                                  const char *prepend, size_t prepend_len,
                                  const char *append, size_t append_len,
                                  size_t *new_len);


/** Concatenate a string to an ce_varbuf
 * @param vb pointer to the ce_varbuf struct
 * @param str the string to append
 * @note vb->strlen will be set to the length of the new string
 */
#define ce_varbuf_strcat(vb, str) ce_varbuf_strmemcat(vb, str, strlen(str))

/** Perform string substitutions based on regexp match, using an ce_varbuf.
 * This function behaves like ce_pregsub(), but appends to an ce_varbuf
 * instead of allocating the result from a pool.
 * @param vb The ce_varbuf to which the string will be appended
 * @param input An arbitrary string containing $1 through $9.  These are
 *              replaced with the corresponding matched sub-expressions
 * @param source The string that was originally matched to the regex
 * @param nmatch the nmatch returned from ce_pregex
 * @param pmatch the pmatch array returned from ce_pregex
 * @param maxlen the maximum string length to append to vb, 0 for unlimited
 * @return APR_SUCCESS if successful
 * @note Just like ce_pregsub(), this function does not copy the part of
 *       *source before the matching part (i.e. the first pmatch[0].rm_so
 *       characters).
 * @note If vb->strlen equals CE_VARBUF_UNKNOWN, it will be set to
 *       strlen(vb->buf) first.
 */
ce_int_t ce_varbuf_regsub(struct ce_varbuf *vb,
                                          const char *input,
                                          const char *source,
                                          size_t nmatch,
                                          ce_regmatch_t pmatch[],
                                          size_t maxlen);

/** Read a line from an ce_configfile_t and append it to an ce_varbuf.
 * @param vb pointer to the ce_varbuf struct
 * @param cfg pointer to the ce_configfile_t
 * @param max_len maximum line length, including leading/trailing whitespace
 * @return see ce_cfg_getline()
 * @note vb->strlen will be set to the length of the line
 * @note If vb->strlen equals CE_VARBUF_UNKNOWN, it will be set to
 *       strlen(vb->buf) first.
 */
#include "ce_line_conf_file.h"
ce_int_t ce_varbuf_cfg_getline(struct ce_varbuf *vb,
                                               ce_configfile_t *cfp,
                                               size_t max_len);

#ifdef __cplusplus
}
#endif

#endif  /* !CE_VARBUF_H */
/** @} */

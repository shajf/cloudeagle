/*
 * =====================================================================================
 *
 *       Filename:  ce_string.h
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

#ifndef _CE_STRING_H
#define _CE_STRING_H

#include "ce_basicdefs.h"
#include "ce_palloc.h"

typedef struct 
{
    size_t      len;
    u_char     *data;
}ce_str_t;

#define ce_string(str)     		{ sizeof(str) - 1, (u_char *) str}
#define ce_null_string     		{ 0, NULL }
#define ce_str_set(str, text) 	(str)->len = sizeof(text) - 1; (str)->data = (u_char *) text
#define ce_str_null(str)   		(str)->len = 0; (str)->data = NULL

#define ce_tolower(c)      		(u_char) ((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define ce_toupper(c)      		(u_char) ((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

#define ce_strncmp(s1, s2, n) 	 strncmp(( char *) s1, ( char *) s2, n)
#define ce_strcmp(s1, s2)	         strcmp(( char *) s1, ( char *) s2)

#define ce_strstr(s1, s2)  		 strstr(( char *) s1, ( char *) s2)
#define ce_strlen(s)       		 strlen((const char*) s)

#define ce_strchr(s1, c)   		 strchr(( char *) s1, (int) c)
#define ce_strrchr(s, c)  		strrchr(s, c)

#define ce_isspace(c) (isspace(((unsigned char)(c))))
#define ce_isalnum(c) (isalnum(((unsigned char)(c))))
/*inline u_char *ce_strlchr(u_char *p, u_char *last, u_char c)
{
    while (p < last) {

        if (*p == c) {
            return p;
        }

        p++;
    }

    return NULL;
}
*/
#define ce_memzero(buf, n)       	   (void) memset(buf, 0, n)
#define ce_memset(buf, c, n)             (void) memset(buf, c, n)

#define ce_memcpy(dst, src, n)           (void) memcpy(dst, src, n)
#define ce_cpymem(dst, src, n)           (((u_char *) memcpy(dst, src, n)) + (n))

#define ce_memmove(dst, src, n)   	   (void) memmove(dst, src, n)
#define ce_movemem(dst, src, n)          (((u_char *) memmove(dst, src, n)) + (n))
#define ce_memcmp(s1, s2, n)              memcmp(( char *) s1, ( char *) s2, n)

#define ce_base64_encoded_length(len)  (((len + 2) / 3) * 4)
#define ce_base64_decoded_length(len)  (((len + 3) / 4) * 3)
#define ce_isdigit(ch) ((ch)>='0'&&(ch)<='9')
#define ce_isslash(ch) ((ch)=='/')

#ifdef __cplusplus
extern "c" {
#endif

extern void   ce_strlow(u_char *dst,u_char *src,size_t n); 

extern u_char *ce_cpystrn(u_char *dst, u_char *src, size_t n);

extern u_char *ce_cdecl ce_sprintf(u_char *buf,  const char *fmt, ...);

extern u_char *ce_cdecl ce_snprintf(u_char *buf, size_t max,  const char *fmt, ...);

extern u_char *ce_cdecl ce_slprintf(u_char *buf, u_char *last, const char *fmt, ...);

extern u_char *ce_vslprintf(u_char *buf, u_char *last, const char *fmt, va_list args);

#define ce_vsnprintf(buf, max, fmt, args) ce_vslprintf(buf, buf + (max), fmt, args)

extern ce_int_t ce_strcasecmp(u_char *s1, u_char *s2);

extern ce_int_t ce_strncasecmp(u_char *s1, u_char *s2, size_t n);

extern u_char *ce_strnstr(u_char *s1, char *s2, size_t n);

extern u_char *ce_strstrn(u_char *s1, char *s2, size_t n);

extern u_char *ce_strcasestrn(u_char *s1, char *s2, size_t n);

extern char* ce_strcasestr(const char* s1,const char* s2);

extern const char * ce_stripprefix(const char *bigstring, const char *prefix);

extern char * ce_field_noparam(ce_pool_t *p, const char *intype);
extern int ce_strcmp_match(const char *str, const char *expected);
extern int ce_strcasecmp_match(const char *str, const char *expected);
extern int ce_os_is_path_absolute(ce_pool_t *p, const char *dir);
extern int ce_is_matchexp(const char *str);
 
extern u_char *ce_strlcasestrn(u_char *s1, u_char *last, u_char *s2, size_t n);

extern ce_int_t ce_rstrncmp(u_char *s1, u_char *s2, size_t n);

extern ce_int_t ce_rstrncasecmp(u_char *s1, u_char *s2, size_t n);

extern ce_int_t ce_memn2cmp(u_char *s1, u_char *s2, size_t n1, size_t n2);

extern ce_int_t ce_dns_strcmp(u_char *s1, u_char *s2);

extern ce_int_t ce_atoi(u_char *line, size_t n);

extern ce_int_t ce_atofp(u_char *line, size_t n, size_t point);

extern ssize_t ce_atosz(u_char *line, size_t n);

extern off_t ce_atoof(u_char *line, size_t n);

extern time_t ce_atotm(u_char *line, size_t n);

extern ce_int_t ce_hextoi(u_char *line, size_t n);

extern u_char *ce_hex_dump(u_char *dst, u_char *src, size_t len);

extern void ce_encode_base64(ce_str_t *dst, ce_str_t *src);

extern ce_int_t ce_decode_base64(ce_str_t *dst, ce_str_t *src);

extern ce_int_t ce_decode_base64url(ce_str_t *dst, ce_str_t *src);

extern uint32_t ce_utf8_decode(u_char **p, size_t n);

extern size_t ce_utf8_length(u_char *p, size_t n);

extern u_char *ce_utf8_cpystrn(u_char *dst, u_char *src, size_t n, size_t len);

extern void ce_sort(void *base, size_t n, size_t size,
    ce_int_t (*cmp)( void *,  void *));

extern char *ce_pstrdup(ce_pool_t *pool, const char *src);
extern char *ce_pstrldup(ce_pool_t *pool, char *src,size_t len);
extern char *ce_pstrmemdup(ce_pool_t *pool,const char* src,size_t len); 
extern  u_char * ce_pstrcat(ce_pool_t *a, ...);
extern size_t ce_itoa(u_char* buf,int val);
extern u_char* ce_make_dir(ce_pool_t *pool,...);
extern u_char *ce_make_dir2(u_char *dir,...);
extern size_t ce_get_dir_len(u_char *prefix,...);
extern u_char *ce_make_full_path(u_char *buf,
				       size_t buf_size,
				       u_char *file_name,...);

extern u_char *ce_make_full_path2(ce_pool_t *pool,
				   u_char *file_name,...);

extern int ce_ipstrtoint( char *ip,size_t len);

extern void ip_int2_dostring(unsigned long ip,u_char *ip_b,size_t len);
extern size_t ce_get_full_path_len(u_char *file_name,...);

extern void ce_getparents(char *name);
extern void ce_no2slash(char *name);
extern char * ce_make_dirstr_prefix(char *d, const char *s, int n);
extern char * ce_make_dirstr_parent(ce_pool_t *p, const char *s);
extern int ce_count_dirs(const char *path);
extern char*  ce_getword(ce_pool_t *atrans, const char **line, char stop);
extern char * ce_getword_nc(ce_pool_t *atrans, char **line, char stop);
extern char * ce_getword_white(ce_pool_t *atrans, const char **line);
extern char *ce_getword_white_nc(ce_pool_t *atrans, char **line);
extern char * ce_getword_nulls(ce_pool_t *atrans, const char **line,char stop);
extern char * ce_getword_nulls_nc(ce_pool_t *atrans, char **line,char stop);
extern char * ce_getword_conf(ce_pool_t *p, const char **line);
extern char * ce_getword_conf_nc(ce_pool_t *p, char **line);
extern char * ce_pstrcatv(ce_pool_t *a, const struct iovec *vec,size_t nvec, size_t *nbytes);
#ifdef __cplusplus
}
#endif

#define ce_qsort             qsort
#endif /* _CE_STRING_H*/

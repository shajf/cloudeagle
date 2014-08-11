/*
 * =====================================================================================
 *
 *       Filename:  ce_string.c
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

#include "ce_string.h"
#include "ce_times.h"
#include "ce_alloc.h"
#include "ce_file.h"
#define MAX_SAVED_LENGTHS 6
static u_char *ce_sprintf_num(u_char *buf, u_char *last, uint64_t ui64,
    u_char zero, ce_uint_t hexadecimal, ce_uint_t width);
static ce_int_t ce_decode_base64_internal(ce_str_t *dst, ce_str_t *src,
     u_char *basis);

void ce_strlow(u_char *dst, u_char *src, size_t n)
{
	while (n) 
	{
		*dst = ce_tolower(*src);
		dst++;
		src++;
		n--;
	}
}

u_char *ce_cpystrn(u_char *dst, u_char *src, size_t n)
{
	if (n == 0) 
	{
		return dst;
	}

	while (n--) 
	{
		*dst = *src;

	       	if (*dst == '\0') 
		{
			return dst;
		}
		dst++;
		src++;
	}

	*dst = '\0';

	return dst;
}

/*
 * supported formats:
 *    %[0][width][x][X]O        off_t
 *    %[0][width]T              time_t
 *    %[0][width][u][x|X]z      ssize_t/size_t
 *    %[0][width][u][x|X]d      int/u_int
 *    %[0][width][u][x|X]l      long
 *    %[0][width|m][u][x|X]i    ce_int_t/ce_uint_t
 *    %[0][width][u][x|X]D      int32_t/uint32_t
 *    %[0][width][u][x|X]L      int64_t/uint64_t
 *    %[0][width][.width]f      double, max valid number fits to %18.15f
 *    %P                        ce_pid_t
 *    %M                        ce_msec_t
 *    %r                        rlim_t
 *    %p                        void *
 *    %V                        ce_str_t *
 *    %s                        null-terminated string
 *    %*s                       length and string
 *    %Z                        '\0'
 *    %N                        '\n'
 *    %c                        char
 *    %%                        %
 *
 *  reserved:
 *    %t                        ptrdiff_t
 *    %S                        null-terminated wchar string
 *    %C                        wchar
 */


u_char * ce_cdecl ce_sprintf(u_char *buf, const char *fmt, ...)
{
	u_char   *p;
	va_list   args;

	va_start(args, fmt);
	p = ce_vslprintf(buf, (u_char*) -1, fmt, args);
	va_end(args);
	return p;
}


u_char * ce_cdecl ce_snprintf(u_char *buf, size_t max, const char *fmt, ...)
{
	u_char   *p;
	va_list   args;

	va_start(args, fmt);
	p = ce_vslprintf(buf, buf + max, fmt, args);
	va_end(args);

	return p;
}


u_char * ce_cdecl ce_slprintf(u_char *buf, u_char *last,  const char *fmt, ...)
{
	u_char   *p;
	va_list   args;

	va_start(args, fmt);
	p = ce_vslprintf(buf, last, fmt, args);
	va_end(args);

	return p;
}


u_char *ce_vslprintf(u_char *buf, u_char *last,  const char *fmt, va_list args)
{
	u_char                *p, zero;
	int                    d;
	double                 f, scale;
	size_t                 len, slen;
	int64_t                i64;
	uint64_t               ui64;
	ce_uint_t             width, sign, hex, max_width, frac_width, n;
	ce_str_t             *v;

	while (*fmt && buf < last) 
	{

        /*
         * "buf < last" means that we could copy at least one character:
         * the plain character, "%%", "%c", and minus without the checking
         */

		if (*fmt == '%') 
		{

			i64 = 0;
			ui64 = 0;

			zero = (u_char) ((*++fmt == '0') ? '0' : ' ');
			width = 0;
			sign = 1;
		       	hex = 0;
			max_width = 0;
		       	frac_width = 0;
			slen = (size_t) -1;

			while (*fmt >= '0' && *fmt <= '9') 
			{
			       	width = width * 10 + *fmt++ - '0';
			}

			for ( ;; ) 
			{
				switch (*fmt) 
				{

					case 'u':
						sign = 0;
						fmt++;
						continue;

					case 'm':
						max_width = 1;
						fmt++;
						continue;

				       	case 'X':
						hex = 2;
						sign = 0;
						fmt++;
						continue;

				       	case 'x':
						hex = 1;
						sign = 0;
						fmt++;
						continue;

					case '.':
						fmt++;

						while (*fmt >= '0' && *fmt <= '9') 
						{
							frac_width = frac_width * 10 + *fmt++ - '0';
						}

				       	break;

					case '*':
						slen = va_arg(args, size_t);
						fmt++;
						continue;

				       	default:
					break;
			}//switch

                break;
            }//for


	switch (*fmt) 
	{

		case 'V':
		      	v = va_arg(args, ce_str_t *);

			len = ce_min(((size_t) (last - buf)), v->len);
			buf = ce_cpymem(buf, v->data, len);
			fmt++;

			continue;
		
		case 's':
			p = va_arg(args, u_char *);

		       	if (slen == (size_t) -1) 
			{
				while (*p && buf < last) 
				{
					*buf++ = *p++;
				}

			} 
		
			else 
			{
				len = ce_min(((size_t) (last - buf)), slen);
				buf = ce_cpymem(buf, p, len);
			}

			fmt++;

			continue;

	       	case 'O':
			i64 = (int64_t) va_arg(args, off_t);
			sign = 1;
			break;

	   	case 'P':
			i64 = (int64_t) va_arg(args, ce_pid_t);
			sign = 1;
		 	break;

            case 'T':
                i64 = (int64_t) va_arg(args, time_t);
                sign = 1;
                break;
	
	    #if 0
            case 'M':
		ms = (ce_msec_t) va_arg(args, ce_msec_t);
                if ((ce_msec_int_t) ms == -1) 
		{
		    	sign = 1;
			i64 = -1;
                } 
		
		else 
		{
		    	sign = 0;
			ui64 = (uint64_t) ms;
                }
                break;
	    #endif

            case 'z':
	     	if (sign) 
		{
		      	i64 = (int64_t) va_arg(args, ssize_t);
                } 
		
		else 
		{
		    	ui64 = (uint64_t) va_arg(args, size_t);
                }
                break;

            case 'i':
		if (sign) 
		{
		    	i64 = (int64_t) va_arg(args, ce_int_t);
                } 
		
		else 
		{
		     	ui64 = (uint64_t) va_arg(args, ce_uint_t);
                }

                if (max_width) 
		{
			width = CE_INT64_LEN;
                }

                break;

            case 'd':
                if (sign) 
		{
		    	i64 = (int64_t) va_arg(args, int);
                } 
		
		else 
		{
		    	ui64 = (uint64_t) va_arg(args, u_int);
                }
                
		break;

            case 'l':
                if (sign) 
		{
		    	i64 = (int64_t) va_arg(args, long);
                } 
		
		else 
		{
		    	ui64 = (uint64_t) va_arg(args, u_long);
                }
                break;

            case 'D':
                if (sign)
		{
		     	i64 = (int64_t) va_arg(args, int32_t);
                } 
		
		else 
		{
		    	ui64 = (uint64_t) va_arg(args, uint32_t);
                }
                break;

            case 'L':
                if (sign)
		{
		    	i64 = va_arg(args, int64_t);
                }
		
		else 
		{
		    	ui64 = va_arg(args, uint64_t);
                }
                break;


            case 'f':
                f = va_arg(args, double);

                if (f < 0) 
		{
		    	*buf++ = '-';
		       	f = -f;
                }

                ui64 = (int64_t) f;

                buf = ce_sprintf_num(buf, last, ui64, zero, 0, width);

                if (frac_width)
		{

                    if (buf < last) 
		    {
                        *buf++ = '.';
                    }

                    scale = 1.0;

                    for (n = frac_width; n; n--) 
		    {
                        scale *= 10.0;
                    }

                    /*
                     * (int64_t) cast is required for msvc6:
                     * it can not convert uint64_t to double
                     */
                    ui64 = (uint64_t) ((f - (int64_t) ui64) * scale + 0.5);

                    buf = ce_sprintf_num(buf, last, ui64, '0', 0, frac_width);
                }

                fmt++;

                continue;

            case 'r':
                i64 = (int64_t) va_arg(args, rlim_t);
                sign = 1;
                break;

            case 'p':
                ui64 = (uintptr_t) va_arg(args, void *);
                hex = 2;
                sign = 0;
                zero = '0';
                width = CE_PTR_SIZE * 2;
                break;

            case 'c':
                d = va_arg(args, int);
                *buf++ = (u_char) (d & 0xff);
                fmt++;

                continue;

            case 'Z':
                *buf++ = '\0';
                fmt++;

                continue;

            case 'N':

                *buf++ = LF;
                fmt++;
		continue;

            case '%':
                *buf++ = '%';
                 fmt++;

                continue;

            default:
                *buf++ = *fmt++;

                continue;
            }

           if (sign) 
	   {
                if (i64 < 0)
		{
                    *buf++ = '-';
                     ui64 = (uint64_t) -i64;

                }
		
		else 
		{
                    ui64 = (uint64_t) i64;
                }
            }

            buf = ce_sprintf_num(buf, last, ui64, zero, hex, width);

            fmt++;

        } else {
            *buf++ = *fmt++;
        }
    }

    return buf;
}


static u_char *ce_sprintf_num(u_char *buf, u_char *last, uint64_t ui64, u_char zero,  ce_uint_t hexadecimal, ce_uint_t width)
{
	u_char         *p, temp[CE_INT64_LEN + 1];
                       /*
                        * we need temp[CE_INT64_LEN] only,
                        * but icc issues the warning
                        */
	size_t          len;
       	uint32_t        ui32;
       	static u_char   hex[] = "0123456789abcdef";
	static u_char   HEX[] = "0123456789ABCDEF";

       	p = temp + CE_INT64_LEN;

	if (hexadecimal == 0) 
	{

		if (ui64 <= CE_MAX_UINT32_VALUE) 
		{

            /*
             * To divide 64-bit numbers and to find remainders
             * on the x86 platform gcc and icc call the libc functions
             * [u]divdi3() and [u]moddi3(), they call another function
             * in its turn.  On FreeBSD it is the qdivrem() function,
             * its source code is about 170 lines of the code.
             * The glibc counterpart is about 150 lines of the code.
             *
             * For 32-bit numbers and some divisors gcc and icc use
             * a inlined multiplication and shifts.  For example,
             * unsigned "i32 / 10" is compiled to
             *
             *     (i32 * 0xCCCCCCCD) >> 35
             */

			ui32 = (uint32_t) ui64;

			do 
			{
				*--p = (u_char) (ui32 % 10 + '0');
			} while (ui32 /= 10);

		} 
		
		else 
		{
			do {
			      	*--p = (u_char) (ui64 % 10 + '0');
		           } while (ui64 /= 10);
		}

	}
	else if (hexadecimal == 1) 
	{

		do
		{

            /* the "(uint32_t)" cast disables the BCC's warning */
			*--p = hex[(uint32_t) (ui64 & 0xf)];

	   	} while (ui64 >>= 4);

       	}
	
	else 
	{ /* hexadecimal == 2 */

		do
		{

            /* the "(uint32_t)" cast disables the BCC's warning */
			*--p = HEX[(uint32_t) (ui64 & 0xf)];

		} 
		while (ui64 >>= 4);
	}

    /* zero or space padding */

	len = (temp + CE_INT64_LEN) - p;

       	while (len++ < width && buf < last)
	{
		*buf++ = zero;
	}

    /* number safe copy */

    len = (temp + CE_INT64_LEN) - p;

    if (buf + len > last)
    {
	len = last - buf;
    }
    return ce_cpymem(buf, p, len);
}


/*
 * We use ce_strcasecmp()/ce_strncasecmp() for 7-bit ASCII strings only,
 * and implement our own ce_strcasecmp()/ce_strncasecmp()
 * to avoid libc locale overhead.  Besides, we use the ce_uint_t's
 * instead of the u_char's, because they are slightly faster.
 */

ce_int_t ce_strcasecmp(u_char *s1, u_char *s2)
{
	ce_uint_t  c1, c2;

       	for ( ;; ) 
	{
		c1 = (ce_uint_t) *s1++;
		c2 = (ce_uint_t) *s2++;

		c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
		c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

		if (c1 == c2) 
		{

			if (c1) 
			{
				continue;
			}

	       	return 0;
		}

	       	return c1 - c2;
	}
}


ce_int_t ce_strncasecmp(u_char *s1, u_char *s2, size_t n)
{
    ce_uint_t  c1, c2;

    while (n)
    {
        c1 = (ce_uint_t) *s1++;
        c2 = (ce_uint_t) *s2++;

        c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
        c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

        if (c1 == c2) 
	{

            if (c1) 
	    {
                n--;
                continue;
            }

            return 0;
        }

        return c1 - c2;
    }

    return 0;
}


u_char *ce_strnstr(u_char *s1, char *s2, size_t len)
{
    u_char  c1, c2;
    size_t  n;
    c2 = *(u_char *) s2++;
    n = ce_strlen(s2);

    do {
	do 
	{
            if (len-- == 0) 
	    {
                return NULL;
            }

            c1 = *s1++;

            if (c1 == 0) 
	    {
                return NULL;
            }

        } while (c1 != c2);

        if (n > len) 
	{
            return NULL;
        }

    } while (ce_strncmp(s1, (u_char *) s2, n) != 0);

    return --s1;
}


/*
 * ce_strstrn() and ce_strcasestrn() are intended to search for static
 * substring with known length in null-terminated string. The argument n
 * must be length of the second substring - 1.
 */

u_char *ce_strstrn(u_char *s1, char *s2, size_t n)
{
    u_char  c1, c2;

    c2 = *(u_char *) s2++;

    do {
        do 
	{
            c1 = *s1++;

            if (c1 == 0) 
	    {
                return NULL;
            }

        } while (c1 != c2);

    } while (ce_strncmp(s1, (u_char *) s2, n) != 0);

    return --s1;
}


u_char *ce_strcasestrn(u_char *s1, char *s2, size_t n)
{
    ce_uint_t  c1, c2;

    c2 = (ce_uint_t) *s2++;
    c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

    do {
        do 
	{
            c1 = (ce_uint_t) *s1++;

            if (c1 == 0) {
                return NULL;
            }

            c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;

        } while (c1 != c2);

    } while (ce_strncasecmp(s1, (u_char *) s2, n) != 0);

    return --s1;
}


/*
 * ce_strlcasestrn() is intended to search for static substring
 * with known length in string until the argument last. The argument n
 * must be length of the second substring - 1.
 */

u_char *ce_strlcasestrn(u_char *s1, u_char *last, u_char *s2, size_t n)
{
    ce_uint_t  c1, c2;

    c2 = (ce_uint_t) *s2++;
    c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;
    last -= n;

    do {
        do 
	{
            if (s1 >= last)
	    {
                return NULL;
            }

            c1 = (ce_uint_t) *s1++;

            c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;

        } while (c1 != c2);

    } while (ce_strncasecmp(s1, s2, n) != 0);

    return --s1;
}


ce_int_t ce_rstrncmp(u_char *s1, u_char *s2, size_t n)
{
    if (n == 0) 
    {
        return 0;
    }

    n--;

    for ( ;; ) 
    {
        if (s1[n] != s2[n]) {
            return s1[n] - s2[n];
        }

        if (n == 0) {
            return 0;
        }

        n--;
    }
}


ce_int_t ce_rstrncasecmp(u_char *s1, u_char *s2, size_t n)
{
    u_char  c1, c2;

    if (n == 0) 
    {
        return 0;
    }

    n--;

    for ( ;; ) 
    {
        c1 = s1[n];
        if (c1 >= 'a' && c1 <= 'z')
	{
            c1 -= 'a' - 'A';
        }

        c2 = s2[n];
        if (c2 >= 'a' && c2 <= 'z')
	{
            c2 -= 'a' - 'A';
        }

        if (c1 != c2)
	{
            return c1 - c2;
        }

        if (n == 0)
	{
            return 0;
        }

        n--;
    }
}


ce_int_t ce_memn2cmp(u_char *s1, u_char *s2, size_t n1, size_t n2)
{
    size_t     n;
    ce_int_t  m, z;

    if (n1 <= n2)
    {
        n = n1;
        z = -1;

    }
    
    else 
    {
        n = n2;
        z = 1;
    }

    m = ce_memcmp(s1, s2, n);

    if (m || n1 == n2) 
    {
        return m;
    }

    return z;
}


ce_int_t ce_dns_strcmp(u_char *s1, u_char *s2)
{
    ce_uint_t  c1, c2;

    for ( ;; ) 
    {
        c1 = (ce_uint_t) *s1++;
        c2 = (ce_uint_t) *s2++;

        c1 = (c1 >= 'A' && c1 <= 'Z') ? (c1 | 0x20) : c1;
        c2 = (c2 >= 'A' && c2 <= 'Z') ? (c2 | 0x20) : c2;

        if (c1 == c2) 
	{

            if (c1)
	    {
                continue;
            }

            return 0;
        }

        /* in ASCII '.' > '-', but we need '.' to be the lowest character */

        c1 = (c1 == '.') ? ' ' : c1;
        c2 = (c2 == '.') ? ' ' : c2;

        return c1 - c2;
    }
}


ce_int_t ce_atoi(u_char *line, size_t n)
{
    ce_int_t  value;

    if (n == 0) 
    {
        return CE_ERROR;
    }

    for (value = 0; n--; line++) 
    {
        if (*line < '0' || *line > '9') 
	{
            return CE_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) {
        return CE_ERROR;

    } else {
        return value;
    }
}


/* parse a fixed point number, e.g., ce_atofp("10.5", 4, 2) returns 1050 */

ce_int_t ce_atofp(u_char *line, size_t n, size_t point)
{
    ce_int_t   value;
    ce_uint_t  dot;

    if (n == 0) 
    {
        return CE_ERROR;
    }

    dot = 0;

    for (value = 0; n--; line++)
    {

        if (point == 0)
	{
            return CE_ERROR;
        }

        if (*line == '.')
	{
            if (dot) 
	    {
                return CE_ERROR;
            }

            dot = 1;
            continue;
        }

        if (*line < '0' || *line > '9') {
            return CE_ERROR;
        }

        value = value * 10 + (*line - '0');
        point -= dot;
    }

    while (point--) {
        value = value * 10;
    }

    if (value < 0) {
        return CE_ERROR;

    } 
    
    else 
    {
        return value;
    }
}


ssize_t ce_atosz(u_char *line, size_t n)
{
    ssize_t  value;

    if (n == 0) 
    {
        return CE_ERROR;
    }

    for (value = 0; n--; line++)
    {
        if (*line < '0' || *line > '9')
	{
            return CE_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) 
    {
        return CE_ERROR;

    }
    
    else 
    {
        return value;
    }
}


off_t ce_atoof(u_char *line, size_t n)
{
    off_t  value;

    if (n == 0) 
    {
        return CE_ERROR;
    }

    for (value = 0; n--; line++) 
    {
        if (*line < '0' || *line > '9') 
	{
            return CE_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) 
    {
        return CE_ERROR;

    }
   
   else 
   {
        return value;
    }
}


time_t ce_atotm(u_char *line, size_t n)
{
    time_t  value;

    if (n == 0) 
    {
        return CE_ERROR;
    }

    for (value = 0; n--; line++) 
    {
        if (*line < '0' || *line > '9')
	{
            return CE_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) 
    {
        return CE_ERROR;

    } 
    
    else
    {
        return value;
    }
}


ce_int_t ce_hextoi(u_char *line, size_t n)
{
    u_char     c, ch;
    ce_int_t  value;

    if (n == 0) 
    {
        return CE_ERROR;
    }

    for (value = 0; n--; line++) 
    {
        ch = *line;

        if (ch >= '0' && ch <= '9') 
	{
            value = value * 16 + (ch - '0');
            continue;
        }

        c = (u_char) (ch | 0x20);

        if (c >= 'a' && c <= 'f') 
	{
            value = value * 16 + (c - 'a' + 10);
            continue;
        }

        return CE_ERROR;
    }

    if (value < 0) 
    {
        return CE_ERROR;

    } 
    
    else 
    {
        return value;
    }
}


u_char *ce_hex_dump(u_char *dst, u_char *src, size_t len)
{
    static u_char  hex[] = "0123456789abcdef";

    while (len--) 
    {
        *dst++ = hex[*src >> 4];
        *dst++ = hex[*src++ & 0xf];
    }

    return dst;
}


void ce_encode_base64(ce_str_t *dst, ce_str_t *src)
{
    u_char         *d, *s;
    size_t          len;
    static u_char   basis64[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    len = src->len;
    s = src->data;
    d = dst->data;

    while (len > 2) 
    {
        *d++ = basis64[(s[0] >> 2) & 0x3f];
        *d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
        *d++ = basis64[((s[1] & 0x0f) << 2) | (s[2] >> 6)];
        *d++ = basis64[s[2] & 0x3f];

        s += 3;
        len -= 3;
    }

    if (len) {
        *d++ = basis64[(s[0] >> 2) & 0x3f];

        if (len == 1) {
            *d++ = basis64[(s[0] & 3) << 4];
            *d++ = '=';

        } else {
            *d++ = basis64[((s[0] & 3) << 4) | (s[1] >> 4)];
            *d++ = basis64[(s[1] & 0x0f) << 2];
        }

        *d++ = '=';
    }

    dst->len = d - dst->data;
}


ce_int_t ce_decode_base64(ce_str_t *dst, ce_str_t *src)
{
    static u_char   basis64[] = {
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77, 77, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
        77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 77,
        77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
    };

    return ce_decode_base64_internal(dst, src, basis64);
}

ce_int_t ce_decode_base64url(ce_str_t *dst, ce_str_t *src)
{
    static u_char   basis64[] = {
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 62, 77, 77,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 77, 77, 77, 77, 77, 77,
        77,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 77, 77, 77, 77, 63,
        77, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 77, 77, 77, 77, 77,

        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77,
        77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77, 77
    };

    return ce_decode_base64_internal(dst, src, basis64);
}


static ce_int_t ce_decode_base64_internal(ce_str_t *dst, ce_str_t *src,  u_char *basis)
{
    size_t          len;
    u_char         *d, *s;

    for (len = 0; len < src->len; len++) 
    {
        if (src->data[len] == '=') 
	{
            break;
        }

        if (basis[src->data[len]] == 77) 
	{
            return CE_ERROR;
        }
    }

    if (len % 4 == 1) 
    {
        return CE_ERROR;
    }

    s = src->data;
    d = dst->data;

    while (len > 3) {
        *d++ = (u_char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
        *d++ = (u_char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
        *d++ = (u_char) (basis[s[2]] << 6 | basis[s[3]]);

        s += 4;
        len -= 4;
    }

    if (len > 1) {
        *d++ = (u_char) (basis[s[0]] << 2 | basis[s[1]] >> 4);
    }

    if (len > 2) {
        *d++ = (u_char) (basis[s[1]] << 4 | basis[s[2]] >> 2);
    }

    dst->len = d - dst->data;

    return CE_OK;
}


/*
 * ce_utf8_decode() decodes two and more bytes UTF sequences only
 * the return values:
 *    0x80 - 0x10ffff         valid character
 *    0x110000 - 0xfffffffd   invalid sequence
 *    0xfffffffe              incomplete sequence
 *    0xffffffff              error
 */

uint32_t ce_utf8_decode(u_char **p, size_t n)
{
    size_t    len;
    uint32_t  u, i, valid;

    u = **p;

    if (u > 0xf0) 
    {

        u &= 0x07;
        valid = 0xffff;
        len = 3;

    } 
    
    else if (u > 0xe0) 
    {

        u &= 0x0f;
        valid = 0x7ff;
        len = 2;

    } 
    
    else if (u > 0xc0) 
    {

        u &= 0x1f;
        valid = 0x7f;
        len = 1;

    } 
    
    else 
    {
        (*p)++;
        return 0xffffffff;
    }

    if (n - 1 < len) 
    {
        return 0xfffffffe;
    }

    (*p)++;

    while (len) 
    {
        i = *(*p)++;

        if (i < 0x80) 
	{
            return 0xffffffff;
        }

        u = (u << 6) | (i & 0x3f);

        len--;
    }

    if (u > valid) 
    {
        return u;
    }

    return 0xffffffff;
}


size_t ce_utf8_length(u_char *p, size_t n)
{
    u_char  c, *last;
    size_t  len;

    last = p + n;

    for (len = 0; p < last; len++) 
    {

        c = *p;

        if (c < 0x80) 
	{
            p++;
            continue;
        }

        if (ce_utf8_decode(&p, n) > 0x10ffff) 
	{
            /* invalid UTF-8 */
            return n;
        }
    }

    return len;
}


u_char *ce_utf8_cpystrn(u_char *dst, u_char *src, size_t n, size_t len)
{
    u_char  c, *next;

    if (n == 0) 
    {
        return dst;
    }

    while (--n) 
    {

        c = *src;
        *dst = c;

        if (c < 0x80) 
	{

            if (c != '\0') 
	    {
                dst++;
                src++;
                len--;

                continue;
            }

            return dst;
        }

        next = src;

        if (ce_utf8_decode(&next, len) > 0x10ffff)
	{
            /* invalid UTF-8 */
            break;
        }

        while (src < next) {
            *dst++ = *src++;
            len--;
        }
    }

    *dst = '\0';

    return dst;
}

/* ce_sort() is implemented as insertion sort because we need stable sort */

void ce_sort(void *base, size_t n, size_t size,
    ce_int_t (*cmp)( void *,  void *))
{
    u_char  *p1, *p2, *p;

    p =(u_char*)ce_alloc(size);
    if (p == NULL) 
    {
        return;
    }

    for (p1 = (u_char *) base + size;
         p1 < (u_char *) base + n * size;
         p1 += size)
    {
        ce_memcpy(p, p1, size);

        for (p2 = p1;
             p2 > (u_char *) base && cmp(p2 - size, p) > 0;
             p2 -= size)
        {
            ce_memcpy(p2, p2 - size, size);
        }

        ce_memcpy(p2, p, size);
    }

    ce_free(p);
}

char *ce_pstrdup(ce_pool_t *pool, const char *src)
{
    char  *dst;
    size_t len=strlen(src);

    dst = (char*)ce_pcalloc(pool, len+1);
    if (dst == NULL) {
        return NULL;
    }

    ce_memcpy(dst, src, len);
    //dst[len]='\0';
    return dst;
}

char *
ce_pstrldup(ce_pool_t *pool, char *src,size_t len)
{
	char *dst;
	
	dst=(char*)ce_pcalloc(pool, 
			        len+1);
	if(dst==NULL)
	{
		return NULL;
	}

	ce_memcpy(dst,src,len);

	return dst;
}

u_char * ce_pstrcat(ce_pool_t *a, ...)
{
	 u_char *cp, *argp, *res;

	 int nargs = 0;
	  /* Pass one --- find length of required 
	   * string */
	  size_t len = 0;
	  va_list adummy; 

	  va_start(adummy, a);
	  while ((cp = va_arg(adummy, 
	           u_char *)) != NULL)
	 { 
		size_t cplen = ce_strlen(cp);
		len += cplen;
	 }
	 va_end(adummy); 
	 /* Allocate the required string */
	 res = (u_char *)ce_pcalloc(a, len + 1);
	 cp = res;

	 /* Pass two --- copy the argument 
	  * strings into the result space */
	 va_start(adummy, a);

	 nargs = 0;    
	 while ((argp = va_arg(adummy, 
		               u_char *)) != NULL)
	{
		len = ce_strlen(argp);
		 ce_memcpy(cp, argp, len);
		 cp += len;
	}
	va_end(adummy);
	 /* Return the result string */
	 //*cp = '\0';
	 return res; 
}

size_t ce_itoa(u_char *buf,int val)
{
	 size_t radix = 10;
	u_char* p;
	size_t a; //every digit
	int len;
	u_char* b; //start of the digit char
	u_char temp;
	size_t u;
	p=buf;
	if (val<0)
	{
		*p++ = '-';
		val = 0 - val;
	}

	u = (size_t)val;
	b = p;
	do
	{
		a = u % radix;
		u /= radix;
		*p++ = a + '0';
	}while(u>0);
	
	len = (size_t)(p - buf);
	*p-- = 0;
	//swap
	do
	{
		temp = *p;
		*p = *b;
		*b = temp;
		--p;
		++b;
	}while(b < p);

	return len;
}

u_char* ce_make_dir(ce_pool_t *pool,...)
{

	va_list dir_list;
	size_t len=0;
	u_char *cp;
	u_char *res,*tmp_res;
	va_start(dir_list,pool);
	while((cp=va_arg(dir_list,u_char*))!=NULL)
	{
		len+=ce_strlen(cp);
		if(cp[len-1]!='/')
			len+=1;
	}
	
	va_end(dir_list);

	res=(u_char*)ce_pcalloc(pool,len+16);
	tmp_res=res;

	if(res==NULL)
	{
		return NULL;
	}
	va_start(dir_list,pool);
	while((cp=va_arg(dir_list,u_char*))!=NULL)
	{
		len=ce_strlen(cp);

		ce_memcpy(res,cp,len);

		if(res[len-1]!='/')
		{
			ce_memcpy(res+len,"/",1);
			len+=1;
		}
		res+=len;
	}
	va_end(dir_list);
	return tmp_res;
}

size_t ce_get_dir_len(u_char *prefix,...)
{
	
	va_list dir_list;
	size_t len=0;
	u_char *cp;
	va_start(dir_list,prefix);
	if(prefix==NULL)
	{
		len=0;
	}
	else
	{
		len+=ce_strlen(prefix);
		if(prefix[ce_strlen(prefix)-1]!='/')
			len+=1;
	}
	while((cp=va_arg(dir_list,u_char*))!=NULL)
	{
		len+=ce_strlen(cp);
		if(cp[len-1]!='/')
			len+=1;
	}
	
	
	va_end(dir_list);
	return len;

}

u_char *ce_make_dir2(u_char *dir,...)
{

	va_list dir_list;
	size_t len=0;
	u_char *cp;
	u_char *res;
	va_start(dir_list,dir);
	while((cp=va_arg(dir_list,u_char*))!=NULL)
	{
		len+=ce_strlen(cp);
		if(cp[len-1]!='/')
			len+=1;
	}
	
	va_end(dir_list);

	res=dir;

	if(res==NULL)
	{
		return NULL;
	}
	va_start(dir_list,dir);
	while((cp=va_arg(dir_list,u_char*))!=NULL)
	{
		len=ce_strlen(cp);

		ce_memcpy(res,cp,len);

		if(res[len-1]!='/')
		{
			ce_memcpy(res+len,"/",1);
			len+=1;
		}
		res+=len;
	}
	va_end(dir_list);
	return dir;
}



int ce_ipstrtoint( char *ip,size_t len)
{
	int   result = 0;
	int   tmp = 0;
	int   shift = 24;
	 char *pEnd = ip;
	 char *pStart = ip;
        size_t	i=0;
	while((i++)<=len)
	{
		while(*pEnd != '.' && (i++)<=len)
			pEnd++;
		tmp = 0;

		while(pStart < pEnd)
		{
			tmp = tmp * 10 + (*pStart - '0');
			pStart++;
		}
		
		result += (tmp << shift);
		shift -= 8;
		if (len<=0)
			break;
		pStart = pEnd + 1;
		pEnd++;
	}

	return result;
}

u_char *ce_make_full_path(u_char *buf,
				       size_t buf_size,
				       u_char *file_name,...)
{
	if(buf==NULL||buf_size<=0){return NULL;}
	va_list dir_list;
	size_t len=0;

	u_char *cp;
	u_char *res;
	va_start(dir_list,file_name);

	while((cp=va_arg(dir_list,u_char*))!=NULL)
	{
		len+=ce_strlen(cp);
		if(cp[len-1]!='/')
			len+=1;
	}
	len+=ce_strlen(file_name)+1;

	va_end(dir_list);
	
	if(len>buf_size)
	{
		return NULL;
	}
	ce_memzero(buf,buf_size);

	res=buf;

	va_start(dir_list,file_name);

	while((cp=va_arg(dir_list,u_char*))!=NULL)
	{
		len=ce_strlen(cp);

		ce_memcpy(res,cp,len);

		if(res[len-1]!='/')
		{
			ce_memcpy(res+len,"/",1);
			len+=1;
		}
		res+=len;
	}
	va_end(dir_list);
	ce_memcpy(res,file_name,ce_strlen(file_name));
	return buf;
}

u_char *ce_make_full_path2(ce_pool_t *pool,
				   u_char *file_name,...)
{

	if(pool==NULL){return NULL;}

	va_list dir_list;
	size_t len=0;

	u_char *cp;
	u_char *res,*tmp_res;
	va_start(dir_list,file_name);

	while((cp=va_arg(dir_list,u_char*))!=NULL)
	{
		len+=ce_strlen(cp);
		if(cp[len-1]!='/')
			len+=1;
	}

	len+=ce_strlen(file_name)+1;

	va_end(dir_list);
	
	res=(u_char*)ce_pcalloc(pool,len);
	tmp_res=res;


	va_start(dir_list,file_name);

	while((cp=va_arg(dir_list,u_char*))!=NULL)
	{
		len=ce_strlen(cp);

		ce_memcpy(res,cp,len);

		if(res[len-1]!='/')
		{
			ce_memcpy(res+len,"/",1);
			len+=1;
		}
		res+=len;
	}
	va_end(dir_list);
	ce_memcpy(res,file_name,ce_strlen(file_name));
	return tmp_res;

}

size_t 
ce_get_full_path_len(u_char *file_name,...)
{


	va_list dir_list;
	size_t len=0;

	u_char *cp;
	va_start(dir_list,file_name);

	while((cp=va_arg(dir_list,u_char*))!=NULL)
	{
		len+=ce_strlen(cp);
		if(cp[len-1]!='/')
			len+=1;
	}

	len+=ce_strlen(file_name);

	va_end(dir_list);
	return len;
	
}

void ip_int2_dostring(unsigned long ip,u_char *ip_b,size_t len)
{
	struct in_addr s;
	s.s_addr=ip;
	inet_ntop(AF_INET, (void *)&s, (char*)ip_b, len); 
}

char* ce_strcasestr(const char* s1,const char* s2)
{

    char *p1, *p2;
    if (*s2 == '\0') {
        /* an empty s2 */
        return((char *)s1);
    }
    while(1) {
        for ( ; (*s1 != '\0') && (ce_tolower(*s1) != ce_tolower(*s2)); s1++);
        if (*s1 == '\0') {
            return(NULL);
        }
        /* found first character of s2, see if the rest matches */
        p1 = (char *)s1;
        p2 = (char *)s2;
        for (++p1, ++p2; ce_tolower(*p1) == ce_tolower(*p2); ++p1, ++p2) {
            if (*p1 == '\0') {
                /* both strings ended together */
                return((char *)s1);
            }
        }
        if (*p2 == '\0') {
            /* second string ended, a match */
            break;
        }
        /* didn't find a match here, try starting at next character in s1 */
        s1++;
    }
    return((char *)s1);
}

/*
 * Returns an offsetted pointer in bigstring immediately after
 * prefix. Returns bigstring if bigstring doesn't start with
 * prefix or if prefix is longer than bigstring while still matching.
 * NOTE: pointer returned is relative to bigstring, so we
 * can use standard pointer comparisons in the calling function
 * (eg: test if ap_stripprefix(a,b) == a)
 */
const char * ce_stripprefix(const char *bigstring,
                                        const char *prefix)
{
    const char *p1;

    if (*prefix == '\0')
        return bigstring;

    p1 = bigstring;
    while (*p1 && *prefix) {
        if (*p1++ != *prefix++)
            return bigstring;
    }
    if (*prefix == '\0')
        return p1;

    /* hit the end of bigstring! */
    return bigstring;
}

/*
 * Examine a field value (such as a media-/content-type) string and return
 * it sans any parameters; e.g., strip off any ';charset=foo' and the like.
 */

char * ce_field_noparam(ce_pool_t *p, const char *intype)
{
    const char *semi;

    if (intype == NULL) return NULL;

    semi = ce_strchr(intype, ';');
    if (semi == NULL) {
        return ce_pstrdup(p, intype);
    }
    else {
        while ((semi > intype) && ce_isspace(semi[-1])) {
            semi--;
        }
        return ce_pstrldup(p, intype, semi - intype);
    }
}


/* Match = 0, NoMatch = 1, Abort = -1
 */
int ce_strcmp_match(const char *str, const char *expected)
{
    int x, y;

    for (x = 0, y = 0; expected[y]; ++y, ++x) {
        if ((!str[x]) && (expected[y] != '*'))
            return -1;
        if (expected[y] == '*') {
            while (expected[++y] == '*');
            if (!expected[y])
                return 0;
            while (str[x]) {
                int ret;
                if ((ret = ce_strcmp_match(&str[x++], &expected[y])) != 1)
                    return ret;
            }
            return -1;
        }
        else if ((expected[y] != '?') && (str[x] != expected[y]))
            return 1;
    }
    return (str[x] != '\0');
}

int ce_strcasecmp_match(const char *str, const char *expected)
{
    int x, y;

    for (x = 0, y = 0; expected[y]; ++y, ++x) {
        if (!str[x] && expected[y] != '*')
            return -1;
        if (expected[y] == '*') {
            while (expected[++y] == '*');
            if (!expected[y])
                return 0;
            while (str[x]) {
                int ret;
                if ((ret = ce_strcasecmp_match(&str[x++], &expected[y])) != 1)
                    return ret;
            }
            return -1;
        }
        else if (expected[y] != '?'
                 && ce_tolower(str[x]) != ce_tolower(expected[y]))
            return 1;
    }
    return (str[x] != '\0');
}

/* We actually compare the canonical root to this root, (but we don't
 * waste time checking the case), since every use of this function in
 * httpd-2.1 tests if the path is 'proper', meaning we've already passed
 * it through apr_filepath_merge, or we haven't.
 */

int ce_os_is_path_absolute(ce_pool_t *p, const char *dir)
{
    const char *newpath;
    const char *ourdir = dir;
    if (ce_filepath_root(&newpath, &dir, 0, p) != CE_OK
            || strncmp(newpath, ourdir, strlen(newpath)) != 0) {
        return 0;
    }
    return 1;
}


int ce_is_matchexp(const char *str)
{
    register int x;

    for (x = 0; str[x]; x++)
        if ((str[x] == '*') || (str[x] == '?'))
            return 1;
    return 0;
}
char *ce_pstrmemdup(ce_pool_t *pool,const char* src,size_t len)
{
	assert(pool&&src&&len>0);
	char *res = (char*)ce_palloc(pool,len);
	memcpy(res,src,len);
	return res;
}

/*
 * Parse .. so we don't compromise security
 */
void ce_getparents(char *name)
{
    char *next;
    int l, w, first_dot;

    /* Four paseses, as per RFC 1808 */
    /* a) remove ./ path segments */
    for (next = name; *next && (*next != '.'); next++) {
    }

    l = w = first_dot = next - name;
    while (name[l] != '\0') {
        if (name[l] == '.' && ce_isslash(name[l + 1])
            && (l == 0 || ce_isslash(name[l - 1])))
            l += 2;
        else
            name[w++] = name[l++];
    }

    /* b) remove trailing . path, segment */
    if (w == 1 && name[0] == '.')
        w--;
    else if (w > 1 && name[w - 1] == '.' && ce_isslash(name[w - 2]))
        w--;
    name[w] = '\0';

    /* c) remove all xx/../ segments. (including leading ../ and /../) */
    l = first_dot;

    while (name[l] != '\0') {
        if (name[l] == '.' && name[l + 1] == '.' && ce_isslash(name[l + 2])
            && (l == 0 || ce_isslash(name[l - 1]))) {
            register int m = l + 3, n;

            l = l - 2;
            if (l >= 0) {
                while (l >= 0 && !ce_isslash(name[l]))
                    l--;
                l++;
            }
            else
                l = 0;
            n = l;
            while ((name[n] = name[m]))
                (++n, ++m);
        }
        else
            ++l;
    }

    /* d) remove trailing xx/.. segment. */
    if (l == 2 && name[0] == '.' && name[1] == '.')
        name[0] = '\0';
    else if (l > 2 && name[l - 1] == '.' && name[l - 2] == '.'
             && ce_isslash(name[l - 3])) {
        l = l - 4;
        if (l >= 0) {
            while (l >= 0 && !ce_isslash(name[l]))
                l--;
            l++;
        }
        else
            l = 0;
        name[l] = '\0';
    }
}

void ce_no2slash(char *name)
{
    char *d, *s;

    s = d = name;

    while (*s) {
        if ((*d++ = *s) == '/') {
            do {
                ++s;
            } while (*s == '/');
        }
        else {
            ++s;
        }
    }
    *d = '\0';
}

/*
 * copy at most n leading directories of s into d
 * d should be at least as large as s plus 1 extra byte
 * assumes n > 0
 * the return value is the ever useful pointer to the trailing \0 of d
 *
 * MODIFIED FOR HAVE_DRIVE_LETTERS and NETWARE environments,
 * so that if n == 0, "/" is returned in d with n == 1
 * and s == "e:/test.html", "e:/" is returned in d
 * *** See also directory_walk in modules/http/http_request.c

 * examples:
 *    /a/b, 0  ==> /  (true for all platforms)
 *    /a/b, 1  ==> /
 *    /a/b, 2  ==> /a/
 *    /a/b, 3  ==> /a/b/
 *    /a/b, 4  ==> /a/b/
 *
 *    c:/a/b 0 ==> /
 *    c:/a/b 1 ==> c:/
 *    c:/a/b 2 ==> c:/a/
 *    c:/a/b 3 ==> c:/a/b
 *    c:/a/b 4 ==> c:/a/b
 */
char * ce_make_dirstr_prefix(char *d, const char *s, int n)
{
    if (n < 1) {
        *d = '/';
        *++d = '\0';
        return (d);
    }

    for (;;) {
        if (*s == '\0' || (*s == '/' && (--n) == 0)) {
            *d = '/';
            break;
        }
        *d++ = *s++;
    }
    *++d = 0;
    return (d);
}

/*
 * return the parent directory name including trailing / of the file s
 */
char * ce_make_dirstr_parent(ce_pool_t *p, const char *s)
{
    const char *last_slash = ce_strrchr(s, '/');
    char *d;
    int l;

    if (last_slash == NULL) {
        return ce_pstrdup(p, "");
    }
    l = (last_slash - s) + 1;
    d = ce_pstrmemdup(p, s, l);

    return (d);
}


int ce_count_dirs(const char *path)
{
    register int x, n;

    for (x = 0, n = 0; path[x]; x++)
        if (path[x] == '/')
            n++;
    return n;
}

char * ce_getword(ce_pool_t *atrans, const char **line, char stop)
{
    const char *pos = *line;
    int len;
    char *res;

    while ((*pos != stop) && *pos) {
        ++pos;
    }

    len = pos - *line;
    res = ce_pstrmemdup(atrans, *line, len);

    if (stop) {
        while (*pos == stop) {
            ++pos;
        }
    }
    *line = pos;

    return res;
}

char * ce_getword_nc(ce_pool_t *atrans, char **line, char stop)
{
    return ce_getword(atrans, (const char **) line, stop);
}

char * ce_getword_white(ce_pool_t *atrans, const char **line)
{
    const char *pos = *line;
    int len;
    char *res;

    while (!ce_isspace(*pos) && *pos) {
        ++pos;
    }

    len = pos - *line;
    res = ce_pstrmemdup(atrans, *line, len);

    while (ce_isspace(*pos)) {
        ++pos;
    }

    *line = pos;

    return res;
}

char * ce_getword_white_nc(ce_pool_t *atrans, char **line)
{
    return ce_getword_white(atrans, (const char **) line);
}

char * ce_getword_nulls(ce_pool_t *atrans, const char **line,
                                    char stop)
{
    const char *pos = ce_strchr(*line, stop);
    char *res;

    if (!pos) {
        size_t len = strlen(*line);
        res = ce_pstrmemdup(atrans, *line, len);
        *line += len;
        return res;
    }

    res = ce_pstrldup(atrans, *line, pos - *line);

    ++pos;

    *line = pos;

    return res;
}

char * ce_getword_nulls_nc(ce_pool_t *atrans, char **line,
                                       char stop)
{
    return ce_getword_nulls(atrans, (const char **) line, stop);
}

/* Get a word, (new) config-file style --- quoted strings and baccelashes
 * all honored
 */

static char *substring_conf(ce_pool_t *p, const char *start, int len,
                            char quote)
{
    char *result = ce_palloc(p, len + 1);
    char *resp = result;
    int i;

    for (i = 0; i < len; ++i) {
        if (start[i] == '\\' && (start[i + 1] == '\\'
                                 || (quote && start[i + 1] == quote)))
            *resp++ = start[++i];
        else
            *resp++ = start[i];
    }

    *resp++ = '\0';
    return result;
}

char * ce_getword_conf(ce_pool_t *p, const char **line)
{
    const char *str = *line, *strend;
    char *res;
    char quote;

    while (ce_isspace(*str))
        ++str;

    if (!*str) {
        *line = str;
        return "";
    }

    if ((quote = *str) == '"' || quote == '\'') {
        strend = str + 1;
        while (*strend && *strend != quote) {
            if (*strend == '\\' && strend[1] &&
                (strend[1] == quote || strend[1] == '\\')) {
                strend += 2;
            }
            else {
                ++strend;
            }
        }
        res = substring_conf(p, str + 1, strend - str - 1, quote);

        if (*strend == quote)
            ++strend;
    }
    else {
        strend = str;
        while (*strend && !ce_isspace(*strend))
            ++strend;

        res = substring_conf(p, str, strend - str, 0);
    }

    while (ce_isspace(*strend))
        ++strend;
    *line = strend;
    return res;
}

char * ce_getword_conf_nc(ce_pool_t *p, char **line)
{
    return ce_getword_conf(p, (const char **) line);
}

char * ce_pstrcatv(ce_pool_t *a, const struct iovec *vec,
                                 size_t nvec, size_t *nbytes)
{
    size_t i;
    size_t len;
    const struct iovec *src;
    char *res;
    char *dst;

    /* Pass one --- find length of required string */
    len = 0;
    src = vec;
    for (i = nvec; i; i--) {
        len += src->iov_len;
        src++;
    }
    if (nbytes) {
        *nbytes = len;
    }

    /* Allocate the required string */
    res = (char *) ce_palloc(a, len + 1);

    /* Pass two --- copy the argument strings into the result space */
    src = vec;
    dst = res;
    for (i = nvec; i; i--) {
        memcpy(dst, src->iov_base, src->iov_len);
        dst += src->iov_len;
        src++;
    }

    /* Return the result string */
    *dst = '\0';

    return res;
}

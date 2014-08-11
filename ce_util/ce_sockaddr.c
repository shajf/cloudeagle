/*
 * =====================================================================================
 *
 *       Filename:  ce_sockaddr.c
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

#include "ce_network_io.h"
#include "ce_string.h"

#define GETHOSTBYNAME_BUFLEN 512

static ce_int_t get_local_addr(ce_socket_t *sock)
{
	sock->local_addr->salen = sizeof(sock->local_addr->sa);
	if (getsockname(sock->socketdes,
		    (struct sockaddr *)&sock->local_addr->sa,
                    &sock->local_addr->salen) < 0) 
	{
		return CE_ERROR;
	}
    
	else 
	{
		sock->local_port_unknown =0;
		sock->local_interface_unknown = 0;
		/* XXX assumes sin_port and sin6_port 
		 * at same offset */
	       	sock->local_addr->port = \
		ntohs(sock->local_addr->sa.sin.sin_port);
        return CE_OK;
    }
}

static ce_int_t get_remote_addr(ce_socket_t *sock)
{
	sock->remote_addr->salen = sizeof(sock->remote_addr->sa);
	if (getpeername(sock->socketdes,
			(struct sockaddr *)&sock->remote_addr->sa,
			&sock->remote_addr->salen) < 0) 
	{
		return CE_ERROR;
	}

	else 
	{
		sock->remote_addr_unknown = 0;
		/* XXX assumes sin_port and sin6_port 
		 * at same offset */
		sock->remote_addr->port = \
		ntohs(sock->remote_addr->sa.sin.sin_port);
        return CE_OK;
    }
}

ce_int_t ce_sockaddr_ip_getbuf(char *buf, 
				   size_t buflen,
                                   ce_sockaddr_t *sockaddr)
{
    if (!ce_inet_ntop(sockaddr->family,
			sockaddr->ipaddr_ptr, 
			buf, 
			buflen)) 
     {
		return CE_ERROR;
     }

#if CE_HAVE_IPV6
    if (sockaddr->family == AF_INET6 
        && IN6_IS_ADDR_V4MAPPED((struct in6_addr *)sockaddr->ipaddr_ptr)
        && buflen > strlen("::ffff:")) {
        /* This is an IPv4-mapped IPv6 address; drop the leading
         * part of the address string so we're left with the familiar
         * IPv4 format.
         */
        memmove(buf, buf + strlen("::ffff:"),
                strlen(buf + strlen("::ffff:"))+1);
    }
#endif
    /* ensure NUL termination if the buffer is too short */
    buf[buflen-1] = '\0';
    return CE_OK;
}

ce_int_t ce_sockaddr_ip_get(char **addr,
				ce_sockaddr_t *sockaddr)
{
	*addr = (char*)ce_palloc(sockaddr->pool,
			    sockaddr->addr_str_len);
	return ce_sockaddr_ip_getbuf(*addr, 
					sockaddr->addr_str_len,
					sockaddr);
}

void ce_sockaddr_vars_set(ce_sockaddr_t *addr, 
			    int family,
			    ce_port_t port)
{
    addr->family = family;
    addr->sa.sin.sin_family = family;
    if (port) {
        /* XXX IPv6: assumes sin_port and sin6_port at same offset */
        addr->sa.sin.sin_port = htons(port);
        addr->port = port;
    }

    if (family == CE_INET) {
        addr->salen = sizeof(struct sockaddr_in);
        addr->addr_str_len = 16;
        addr->ipaddr_ptr = &(addr->sa.sin.sin_addr);
        addr->ipaddr_len = sizeof(struct in_addr);
    }
#if CE_HAVE_IPV6
    else if (family == CE_INET6) {
        addr->salen = sizeof(struct sockaddr_in6);
        addr->addr_str_len = 46;
        addr->ipaddr_ptr = &(addr->sa.sin6.sin6_addr);
        addr->ipaddr_len = sizeof(struct in6_addr);
    }
#endif
}

ce_int_t ce_socket_addr_get(ce_sockaddr_t **sa,
				ce_interface_e which,
				ce_socket_t *sock)
{
    if (which == CE_LOCAL) {
        if (sock->local_interface_unknown || sock->local_port_unknown) {
            ce_int_t rv = get_local_addr(sock);

            if (rv != CE_OK) {
                return rv;
            }
        }
        *sa = sock->local_addr;
    }
    else if (which == CE_REMOTE) {
        if (sock->remote_addr_unknown) {
            ce_int_t rv = get_remote_addr(sock);

            if (rv != CE_OK) {
                return rv;
            }
        }
        *sa = sock->remote_addr;
    }
    else {
        *sa = NULL;
        return CE_ERROR;
    }
    return CE_OK;
}

ce_int_t ce_parse_addr_port(char **addr,
		     		char **scope_id,
       				ce_port_t *port,
				const char *str,
				ce_pool_t *p)
{
    const char *ch, *lastchar;
    int big_port;
    size_t addrlen;

    *addr = NULL;         /* assume not specified */
    *scope_id = NULL;     /* assume not specified */
    *port = 0;            /* assume not specified */

    /* First handle the optional port number.  That may be all that
     * is specified in the string.
     */
    ch = lastchar = str + strlen(str) - 1;
    while (ch >= str && ce_isdigit(*ch)) {
        --ch;
    }

    if (ch < str) {       /* Entire string is the port. */
        big_port = atoi(str);
        if (big_port < 1 || big_port > 65535) {
            return CE_EINVAL;
        }
        *port = big_port;
        return CE_OK;
    }

    if (*ch == ':' && ch < lastchar) { /* host and port number specified */
        if (ch == str) {               /* string starts with ':' -- bad */
            return CE_EINVAL;
        }
        big_port = atoi(ch + 1);
        if (big_port < 1 || big_port > 65535) {
            return CE_EINVAL;
        }
        *port = big_port;
        lastchar = ch - 1;
    }

    /* now handle the hostname */
    addrlen = lastchar - str + 1;

/* XXX we don't really have to require CE_HAVE_IPV6 for this; 
 * just pass char[] for ipaddr (so we don't depend on struct in6_addr)
 * and always define CE_INET6 
 */
#if CE_HAVE_IPV6
    if (*str == '[') {
        const char *end_bracket = memchr(str, ']', addrlen);
        struct in6_addr ipaddr;
        const char *scope_delim;

        if (!end_bracket || end_bracket != lastchar) {
            *port = 0;
            return CE_EINVAL;
        }

        /* handle scope id; this is the only context where it is allowed */
        scope_delim = memchr(str, '%', addrlen);
        if (scope_delim) {
            if (scope_delim == end_bracket - 1) { /* '%' without scope id */
                *port = 0;
                return CE_EINVAL;
            }
            addrlen = scope_delim - str - 1;
            *scope_id = ce_palloc(p, end_bracket - scope_delim);
            memcpy(*scope_id, scope_delim + 1, end_bracket - scope_delim - 1);
            (*scope_id)[end_bracket - scope_delim - 1] = '\0';
        }
        else {
            addrlen = addrlen - 2; /* minus 2 for '[' and ']' */
        }

        *addr = (char*)ce_palloc(p, addrlen + 1);
        memcpy(*addr,
               str + 1,
               addrlen);
        (*addr)[addrlen] = '\0';
        if (ce_inet_pton(AF_INET6, *addr, &ipaddr) != 1) {
            *addr = NULL;
            *scope_id = NULL;
            *port = 0;
            return CE_EINVAL;
        }
    }
    else 
#endif
    {
        /* XXX If '%' is not a valid char in a DNS name, we *could* check 
         *     for bogus scope ids first.
         */
        *addr = (char*)ce_palloc(p, addrlen + 1);
        memcpy(*addr, str, addrlen);
        (*addr)[addrlen] = '\0';
    }
    return CE_OK;
}


static ce_int_t call_resolver(ce_sockaddr_t **sa,
                                  char *hostname, 
				  ce_int32_t family,
                                  ce_port_t port, 
				  ce_int32_t flags, 
                                  ce_pool_t *p)
{
    flags=flags;
    struct addrinfo hints, *ai, *ai_list;
    ce_sockaddr_t *prev_sa;
    int error;
    char *servname = NULL; 

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = family;
    hints.ai_socktype = SOCK_STREAM;
    if(hostname == NULL) {
        //servname = ce_itoa(p, port);
    }
    
    error = getaddrinfo(hostname, servname, &hints, &ai_list);
    
    if (error) 
    {
            return errno;
        
    }

    prev_sa = NULL;
    ai = ai_list;
    while (ai) 
    { /* while more addresses to report */
        ce_sockaddr_t *new_sa;

        /* Ignore anything bogus: getaddrinfo in some old versions of
         * glibc will return AF_UNIX entries for CE_UNSPEC+AI_PASSIVE
         * lookups. */
#if CE_HAVE_IPV6
        if (ai->ai_family != AF_INET && ai->ai_family != AF_INET6) {
#else
        if (ai->ai_family != AF_INET) {
#endif
            ai = ai->ai_next;
            continue;
        }

        new_sa = (ce_sockaddr_t*)ce_pcalloc(p, sizeof(ce_sockaddr_t));

        new_sa->pool = p;
        memcpy(&new_sa->sa, ai->ai_addr, ai->ai_addrlen);
        ce_sockaddr_vars_set(new_sa, ai->ai_family, port);

        if (!prev_sa) { /* first element in new list */
            if (hostname) {
                new_sa->hostname = ce_pstrdup(p, hostname);
            }
            *sa = new_sa;
        }
        else {
            new_sa->hostname = prev_sa->hostname;
            prev_sa->next = new_sa;
        }

        prev_sa = new_sa;
        ai = ai->ai_next;
    }
    freeaddrinfo(ai_list);
    return CE_OK;
}

static ce_int_t find_addresses(ce_sockaddr_t **sa, 
                                    char *hostname,
				   ce_int32_t family,
                                   ce_port_t port,
				   ce_int32_t flags, 
                                   ce_pool_t *p)
{
    if (flags & CE_IPV4_ADDR_OK) {
        ce_int_t error = call_resolver(sa, hostname, AF_INET, port, flags, p);

#if CE_HAVE_IPV6
        if (error) {
            family = AF_INET6; /* try again */
        }
        else
#endif
        return error;
    }
#if CE_HAVE_IPV6
    else if (flags & CE_IPV6_ADDR_OK) {
        ce_int_t error = call_resolver(sa, hostname, AF_INET6, port, flags, p);

        if (error) {
            family = AF_INET; /* try again */
        }
        else {
            return CE_OK;
        }
    }
#endif

    return call_resolver(sa, hostname, family, port, flags, p);
}

ce_int_t ce_sockaddr_info_get(ce_sockaddr_t **sa,
				   char *hostname, 
			  	  ce_int32_t family,
				  ce_port_t port,
		    		  ce_int32_t flags,
				  ce_pool_t *p)
{
	ce_int32_t masked;
	*sa = NULL;

    if ((masked = flags & (CE_IPV4_ADDR_OK | CE_IPV6_ADDR_OK))) {
        if (!hostname ||
            family != CE_UNSPEC ||
            masked == (CE_IPV4_ADDR_OK | CE_IPV6_ADDR_OK)) {
            return CE_EINVAL;
        }
#if !CE_HAVE_IPV6
        if (flags & CE_IPV6_ADDR_OK) {
            return CE_ERROR;
        }
#endif
    }
#if !CE_HAVE_IPV6
    /* What may happen is that CE is not IPv6-enabled, but we're still
     * going to call getaddrinfo(), so we have to tell the OS we only
     * want IPv4 addresses back since we won't know what to do with
     * IPv6 addresses.
     */
    if (family == CE_UNSPEC) {
        family = CE_INET;
    }
#endif

    return find_addresses(sa, hostname, family, port, flags, p);
}





    
#define V4MAPPED_EQUAL(a,b)                                   \
((a)->sa.sin.sin_family == AF_INET &&                         \
 (b)->sa.sin.sin_family == AF_INET6 &&                        \
 IN6_IS_ADDR_V4MAPPED((struct in6_addr *)(b)->ipaddr_ptr) &&  \
 !memcmp((a)->ipaddr_ptr,                                     \
         &((struct in6_addr *)(b)->ipaddr_ptr)->s6_addr[12],  \
         (a)->ipaddr_len))

int ce_sockaddr_equal(const ce_sockaddr_t *addr1,
			const ce_sockaddr_t *addr2)
{
    if (addr1->ipaddr_len == addr2->ipaddr_len &&
        !memcmp(addr1->ipaddr_ptr, addr2->ipaddr_ptr, addr1->ipaddr_len)){
        return 1;
    }
#if CE_HAVE_IPV6
    if (V4MAPPED_EQUAL(addr1, addr2)) {
        return 1;
    }
    if (V4MAPPED_EQUAL(addr2, addr1)) {
        return 1;
    }
#endif
    return 0; /* not equal */
}


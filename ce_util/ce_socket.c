/*
 * =====================================================================================
 *
 *       Filename:  ce_socket.c
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
#include "ce_poll.h"

static char generic_inaddr_any[16] = {0}; /* big enough for IPv4 
					   *or IPv6 */

static ce_int_t 
socket_cleanup(ce_socket_t *sock)
{
	ce_socket_t *thesocket = sock;
	int sd = thesocket->socketdes;

	/* Set socket descriptor to -1 before close(),
	 * so that there is no
	* chance of returning an already closed FD 
	* from ce_os_sock_get().
	*/
	thesocket->socketdes = -1;

	if (close(sd) == 0) 
	{
		return CE_OK;
	}
    
	else 
	{
		/* Restore, close() was not successful. */
	       	thesocket->socketdes = sd;

		return errno;
	}
}

static void 
set_socket_vars(ce_socket_t *sock, 
		int family,
		int type, 
		int protocol)
{
	sock->type = type;
	sock->protocol = protocol;
	ce_sockaddr_vars_set(sock->local_addr,
	     			family,
	     			0);

	ce_sockaddr_vars_set(sock->remote_addr,
	     			family,
	     			0);
	sock->options = 0;

}

static void
alloc_socket(ce_socket_t **new_instance, 
	     ce_pool_t *p)
{
	*new_instance = (ce_socket_t *)ce_pcalloc(p,
					CE_SOCKET_T_SIZE);
	(*new_instance)->pool = p;

	(*new_instance)->local_addr = (ce_sockaddr_t *)\
       	ce_pcalloc((*new_instance)->pool,
			CE_SOCKADDR_T_SIZE);

       	(*new_instance)->local_addr->pool = p;
       	(*new_instance)->remote_addr = (ce_sockaddr_t *)\
	ce_pcalloc((*new_instance)->pool,
			CE_SOCKADDR_T_SIZE);

	(*new_instance)->remote_addr->pool = p;
	(*new_instance)->remote_addr_unknown = 1;
}

ce_int_t
ce_socket_protocol_get(ce_socket_t *sock,
			int *protocol)
{
	*protocol = sock->protocol;
	return CE_OK;
}

ce_int_t
ce_socket_create(ce_socket_t **new_instance,
		   int ofamily, 
		   int type,
    		   int protocol, 
		   ce_pool_t *cont)
{
	int family = ofamily, flags = 0;


	if (family == CE_UNSPEC) 
	{
		#if CE_HAVE_IPV6
		family = CE_INET6;
		#else
	       	family = CE_INET;
		#endif
	}

      	alloc_socket(new_instance, cont);

	switch (protocol) 
	{
		case 0:
	       	(*new_instance)->socketdes =socket(family, 
					  type|flags,
					  0);
		break;
		
		case CE_PROTO_TCP:
		(*new_instance)->socketdes = socket(family, 
					   type|flags,
					   IPPROTO_TCP);
	       	break;
		case CE_PROTO_UDP:
		(*new_instance)->socketdes = socket(family,
		     			type|flags,
		     			IPPROTO_UDP);
		break;

		case CE_PROTO_SCTP:
		default:
		errno = EPROTONOSUPPORT;
	       	(*new_instance)->socketdes = -1;
		break;
	}

#if CE_HAVE_IPV6
    	if ((*new_instance)->socketdes < 0 && ofamily == CE_UNSPEC) 
	{
		family = CE_INET;
		(*new_instance)->socketdes = socket(family, 
					   type|flags,
					   protocol);
    }
#endif

	if ((*new_instance)->socketdes < 0) 
	{
		return errno;
       	}

	set_socket_vars(*new_instance, 
			family,
			type, 
			protocol);

#ifndef HAVE_SOCK_CLOEXEC
    {
        int flags1;

        if ((flags1 = fcntl((*new_instance)->socketdes, F_GETFD)) == -1)
            return errno;

        flags1 |= FD_CLOEXEC;
        if (fcntl((*new_instance)->socketdes, F_SETFD, flags1) == -1)
            return errno;
    }
#endif

	(*new_instance)->timeout = -1;
       	(*new_instance)->inherit = 0;

     	return CE_OK;
} 

ce_int_t 
ce_socket_shutdown(ce_socket_t *thesocket, 
			ce_shutdown_how_e how)
{
       	return (shutdown(thesocket->socketdes,
   			how) == -1) ? errno : CE_OK;
}

ce_int_t 
ce_socket_close(ce_socket_t *thesocket)
{
	return socket_cleanup(thesocket);
}

ce_int_t 
ce_socket_bind(ce_socket_t *sock,
		ce_sockaddr_t *sa)
{
	if (bind(sock->socketdes, 
                 (struct sockaddr *)&sa->sa, 
		 sa->salen) == -1) 
	{
		return errno;
	}

	else 
	{
		sock->local_addr = sa;
        /* XXX IPv6 - this assumes sin_port and sin6_port 
	 * at same offset */
        
		if (sock->local_addr->sa.sin.sin_port == 0)
		{ /* no need for ntohs() when comparing w/ 0 */
			sock->local_port_unknown = 1; /* kernel got us an ephemeral port */
		}
		return CE_OK;
	}
}

ce_int_t 
ce_socket_listen(ce_socket_t *sock,
		   ce_int32_t backlog)
{
	if (listen(sock->socketdes, backlog) == -1)
		return errno;
	else
		return CE_OK;
}

ce_int_t 
ce_socket_accept(ce_socket_t **new_instance, 
		   ce_socket_t *sock,
		   ce_pool_t *connection_context)
{
	int s;
	ce_sockaddr_t sa;

       	sa.salen = sizeof(sa.sa);

	s = accept(sock->socketdes, 
		   (struct sockaddr *)&sa.sa,
		   &sa.salen);

    	if (s < 0) 
	{
		return errno;
	}
    
	alloc_socket(new_instance,
		     connection_context);

    /* Set up socket variables -- note that it may be possible for
     * *new_instance to be an AF_INET socket when sock is AF_INET6 in some
     * dual-stack configurations, so ensure that the remote_/local_addr
     * structures are adjusted for the family of the accepted
     * socket: */
       	set_socket_vars(*new_instance, 
			sa.sa.sin.sin_family, 
	    		SOCK_STREAM,
	    		sock->protocol);

	(*new_instance)->connected = 1;
	(*new_instance)->timeout = -1;

	(*new_instance)->remote_addr_unknown = 0;

	(*new_instance)->socketdes = s;

       	/* Copy in peer's address. */
	(*new_instance)->remote_addr->sa = sa.sa;
	(*new_instance)->remote_addr->salen = sa.salen;

	*(*new_instance)->local_addr = *sock->local_addr;

       	/* The above assignment just overwrote the pool entry.
	 * Setting the local_addr 
         * pool for the accepted socket back to what it should be.
	 * Otherwise all 
         * allocations for this socket will come from 
	 * a server pool that is not freed until the process 
	 * goes down.*/
       	(*new_instance)->local_addr->pool = connection_context;

	/* fix up any pointers which are no longer valid */
	if (sock->local_addr->sa.sin.sin_family == AF_INET) 
	{
		(*new_instance)->local_addr->ipaddr_ptr =\
		&(*new_instance)->local_addr->sa.sin.sin_addr;
       	}
	#if CE_HAVE_IPV6
	else if (sock->local_addr->sa.sin.sin_family == AF_INET6)
	{

	       	(*new_instance)->local_addr->ipaddr_ptr =\
		&(*new_instance)->local_addr->sa.sin6.sin6_addr;
	}
	#endif
      	(*new_instance)->remote_addr->port = \
	ntohs((*new_instance)->remote_addr->sa.sin.sin_port);
	
	if (sock->local_port_unknown) 
	{
		/* not likely for a listening socket,
	       	* but theoretically possible :) */
	       	(*new_instance)->local_port_unknown = 1;
	}

	if (ce_is_option_set(sock, CE_SO_NONBLOCK) == 1) 
	{
		ce_set_option(*new_instance, CE_SO_NONBLOCK, 1);
	}

    	if (sock->local_interface_unknown ||
	    	!memcmp(sock->local_addr->ipaddr_ptr,
		    	generic_inaddr_any,
			sock->local_addr->ipaddr_len))
	{
        /* If the interface address inside
	 * the listening socket's local_addr wasn't 
         * up-to-date, we don't know local interface of 
	 * the connected socket either.
         *
         * If the listening socket was not bound to 
	 * a specific interface, we
         * don't know the local_addr of the connected socket.
         */
		(*new_instance)->local_interface_unknown = 1;
	}


    return CE_OK;
}

ce_int_t 
ce_socket_connect(ce_socket_t *sock,
		    ce_sockaddr_t *sa)
{
    int rc;        

    do 
    {
	rc = connect(sock->socketdes,
                     (const struct sockaddr *)&sa->sa.sin,
                     sa->salen);

    } while (rc == -1 && errno == EINTR);

    /* we can see EINPROGRESS the first time connect 
     * is called on a non-blocking
     * socket; if called again, we can see EALREADY
     */
    if ((rc == -1) && (errno == EINPROGRESS || errno == EALREADY)
                   && (sock->timeout > 0)) 
	{
	       	rc = ce_wait_for_io_or_timeout(NULL, sock, 0);
		if (rc != CE_OK)
		{
			return rc;
	       	}

       	}


    if (memcmp(sa->ipaddr_ptr, 
	       generic_inaddr_any,
	       sa->ipaddr_len))
	{
        /* A real remote address was passed in.  If the unspecified
         * address was used, the actual remote addr will have to be
         * determined using getpeername() if required. */
	       	sock->remote_addr_unknown = 0;

        /* Copy the address structure details in. */
		sock->remote_addr->sa = sa->sa;
		sock->remote_addr->salen = sa->salen;
        /* Adjust ipaddr_ptr et al. */
		ce_sockaddr_vars_set(sock->remote_addr,
				       sa->family,
				       sa->port);
	}

    if (sock->local_addr->port == 0) 
    {
        /* connect() got us an ephemeral port */
        sock->local_port_unknown = 1;
    }
    
    if (!memcmp(sock->local_addr->ipaddr_ptr,
                generic_inaddr_any,
                sock->local_addr->ipaddr_len)) 
	{
        /* not bound to specific local interface;
	 * connect() had to assign
         * one for the socket
         */
	       	sock->local_interface_unknown = 1;
	}

	if (rc == -1 && errno != EISCONN) 
	{
		return errno;
	}

	sock->connected=1;
	return CE_OK;
}

ce_int_t
ce_socket_type_get(ce_socket_t *sock,
		     int *type)
{
    *type = sock->type;
    return CE_OK;
}

ce_int_t
ce_socket_data_get(void **data,
		     const char *key, 
		     ce_socket_t *sock)
{
	sock_userdata_t *cur = sock->userdata;

	*data = NULL;

	while (cur)
	{
		if (!strcmp(cur->key, key)) 
	{
	    	*data = cur->data;
		break;
        }
	cur = cur->next;
	}

	return CE_OK;
}

ce_int_t 
ce_socket_data_set(ce_socket_t *sock,
		     void *data,
		     char *key)
{
	sock_userdata_t *new_instance = (sock_userdata_t *)ce_palloc(sock->pool, 
					sizeof(sock_userdata_t));

       	new_instance->key = ce_pstrdup(sock->pool, key);
	new_instance->data = data;
	new_instance->next = sock->userdata;
	sock->userdata = new_instance;


    	return CE_OK;
}

ce_int_t 
ce_sockdes_get(int *thesock,
		ce_socket_t *sock)
{
    *thesock = sock->socketdes;
    return CE_OK;
}


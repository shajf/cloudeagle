/*
 * =====================================================================================
 *
 *       Filename:  ce_listen.c
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
#include "ce_listen.h"
#include "ce_connection.h"

ce_listen_rec_t *ce_listeners;
static ce_listen_rec_t *old_listeners;

static int ce_listenbacklog;
static int send_buffer_size;
static int receive_buffer_size;

static ce_int_t make_sock(ce_pool_t *p, ce_listen_rec_t *server)
{
	p=p;
    ce_socket_t *s = server->sd;
    int one = 1;
    int __attribute__((unused)) v6only_setting = 0;
    ce_int_t st;

    st = ce_socket_opt_set(s, CE_SO_REUSEADDR, one);
    if (st != CE_OK && st != CE_ENOTIMPL) {
        ce_socket_close(s);
        return st;
    }

    st = ce_socket_opt_set(s, CE_SO_KEEPALIVE, one);
    if (st != CE_OK && st != CE_ENOTIMPL) {
        ce_socket_close(s);
        return st;
    }

#if CE_HAVE_IPV6
    if (server->bind_addr->family == CE_INET6) {
        st = ce_socket_opt_set(s, CE_IPV6_V6ONLY, v6only_setting);
        if (st != CE_OK && st != CE_ENOTIMPL) {
            ce_socket_close(s);
            return st;
        }
    }
#endif

    /*
     * To send data over high bandwidth-delay connections at full
     * speed we must force the TCP window to open wide enough to keep the
     * pipe full.  The default window size on many systems
     * is only 4kB.  Cross-country WAN connections of 100ms
     * at 1Mb/s are not impossible for well connected sites.
     * If we assume 100ms cross-country latency,
     * a 4kB buffer limits throughput to 40kB/s.
     *
     * To avoid this problem I've added the SendBufferSize directive
     * to allow the web master to configure send buffer size.
     *
     * The trade-off of larger buffers is that more kernel memory
     * is consumed.  YMMV, know your customers and your network!
     *
     * -John Heidemann <johnh@isi.edu> 25-Oct-96
     *
     * If no size is specified, use the kernel default.
     */
    if (send_buffer_size) {
        st = ce_socket_opt_set(s, CE_SO_SNDBUF,  send_buffer_size);
        if (st != CE_OK && st != CE_ENOTIMPL) {
            /* not a fatal error */
        }
    }
    if (receive_buffer_size) {
        st = ce_socket_opt_set(s, CE_SO_RCVBUF, receive_buffer_size);
        if (st != CE_OK && st != CE_ENOTIMPL) {
            /* not a fatal error */
        }
    }


    if ((st = ce_socket_bind(s, server->bind_addr)) != CE_OK) {
        ce_socket_close(s);
        return st;
    }

    if ((st = ce_socket_listen(s, ce_listenbacklog)) != CE_OK) {
        ce_socket_close(s);
        return st;
    }


    server->sd = s;
    server->active = 1;

    server->accept_func = NULL;

    return CE_OK;
}



ce_listen_rec_t *ce_open_listener(ce_pool_t *pool,
				  char *addr,
                                  int port)
{
    ce_listen_rec_t **walk, *last;
    ce_int_t status;
    ce_sockaddr_t *sa;
    int found_listener = 0;

    /* see if we've got an old listener for this address:port */
    for (walk = &old_listeners; *walk;) {
        sa = (*walk)->bind_addr;
        /* Some listeners are not real so they will not have a bind_addr. */
        if (sa) {
            ce_listen_rec_t *new_instance;
            int oldport;

            oldport = sa->port;
            /* If both ports are equivalent, then if their names are equivalent,
             * then we will re-use the existing record.
             */
            if (port == oldport &&
                ((!addr && !sa->hostname) ||
                 ((addr && sa->hostname) && !strcmp(sa->hostname, addr)))) {
                new_instance = *walk;
                *walk = new_instance->next;
                new_instance->next = ce_listeners;
                ce_listeners = new_instance;
                found_listener = 1;
                continue;
            }
        }

        walk = &(*walk)->next;
    }

    if (found_listener) 
    {
        return NULL;
    }

    if ((status = ce_sockaddr_info_get(&sa, addr, CE_UNSPEC, port, 0,
                                        pool))
        != CE_OK) {
        return NULL;
    }

    /* Initialize to our last configured ce_listener. */
    last = ce_listeners;
    while (last && last->next) {
        last = last->next;
    }

    
        ce_listen_rec_t *new_instance;

        /* this has to survive restarts */
        new_instance =(ce_listen_rec_t*)ce_palloc(pool, sizeof(ce_listen_rec_t));
        new_instance->active = 0;
        new_instance->next = 0;
        new_instance->bind_addr = sa;

        /* Go to the next sockaddr. */
        sa = sa->next;

        status = ce_socket_create(&new_instance->sd, new_instance->bind_addr->family,
                                    SOCK_STREAM, 0,pool);


        if (status != CE_OK) 
	{
            return NULL;
        }

	status=make_sock(pool,new_instance);
	if(status != CE_OK)
	{
		return NULL;
	}

        /* We need to preserve the order returned by getaddrinfo() */
        if (last == NULL) {
            ce_listeners = last = new_instance;
        } else {
            last->next = new_instance;
            last = new_instance;
        }
   

    return new_instance;
}

void ce_close_listeners(void)
{
    ce_listen_rec_t *lr;

    for (lr = ce_listeners; lr; lr = lr->next) {
        ce_socket_close(lr->sd);
        lr->active = 0;
    }
}

void ce_listen_pre_config(void)
{
    old_listeners = ce_listeners;
    ce_listeners = NULL;
    ce_listenbacklog =4096;
}


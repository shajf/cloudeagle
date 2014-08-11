/*
 * =====================================================================================
 *
 *       Filename:  ce_sockopt.h
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

static ce_int_t soblock(int sd)
{
    int fd_flags;

    fd_flags = fcntl(sd, F_GETFL, 0);
#if defined(O_NONBLOCK)
    fd_flags &= ~O_NONBLOCK;
#elif defined(O_NDELAY)
    fd_flags &= ~O_NDELAY;
#elif defined(FNDELAY)
    fd_flags &= ~FNDELAY;
#else
#error Please teach CE how to make sockets blocking on your platform.
#endif
    if (fcntl(sd, F_SETFL, fd_flags) == -1) 
    {
        return errno;
    }

    return CE_OK;
}

static ce_int_t sononblock(int sd)
{
    int fd_flags;

    fd_flags = fcntl(sd, F_GETFL, 0);
#if defined(O_NONBLOCK)
    fd_flags |= O_NONBLOCK;
#elif defined(O_NDELAY)
    fd_flags |= O_NDELAY;
#elif defined(FNDELAY)
    fd_flags |= FNDELAY;
#else
#error Please teach CE how to make sockets non-blocking on your platform.
#endif
    if (fcntl(sd, F_SETFL, fd_flags) == -1) {
        return errno;
    }
    return CE_OK;
}


ce_int_t 
ce_socket_timeout_set(ce_socket_t *sock,
			ce_interval_time_t t)
{
    ce_int_t st;

    /* If our new timeout is non-negative and our old timeout was
     * negative, then we need to ensure that we are non-blocking.
     * Conversely, if our new timeout is negative and we had
     * non-negative timeout, we must make sure our socket is blocking.
     * We want to avoid calling fcntl more than necessary on the
     * socket.
     */
    if (t >= 0 && sock->timeout < 0) {
        if (ce_is_option_set(sock, CE_SO_NONBLOCK) != 1) {
            if ((st = sononblock(sock->socketdes)) != CE_OK) {
                return st;
            }
            ce_set_option(sock, CE_SO_NONBLOCK, 1);
        }
    } 
    else if (t < 0 && sock->timeout >= 0) {
        if (ce_is_option_set(sock, CE_SO_NONBLOCK) != 0) { 
            if ((st = soblock(sock->socketdes)) != CE_OK) { 
                return st; 
            }
            ce_set_option(sock, CE_SO_NONBLOCK, 0);
        } 
    }
    /* must disable the incomplete read support if we disable
     * a timeout
     */
    if (t <= 0) {
        sock->options &= ~CE_INCOMPLETE_READ;
    }
    sock->timeout = t;
    return CE_OK;
}


ce_int_t 
ce_socket_opt_set(ce_socket_t *sock, 
                    ce_int32_t opt,
		    ce_int32_t on)
{
    int one;
    ce_int_t rv;

    if (on)
        one = 1;
    else
        one = 0;
    switch(opt) {
    case CE_SO_KEEPALIVE:
#ifdef SO_KEEPALIVE
        if (on != ce_is_option_set(sock, CE_SO_KEEPALIVE)) {
            if (setsockopt(sock->socketdes, SOL_SOCKET, SO_KEEPALIVE, (void *)&one, sizeof(int)) == -1) {
                return errno;
            }
            ce_set_option(sock, CE_SO_KEEPALIVE, on);
        }
#else
        return CE_ENOTIMPL;
#endif
        break;
    case CE_SO_DEBUG:
        if (on != ce_is_option_set(sock, CE_SO_DEBUG)) {
            if (setsockopt(sock->socketdes, SOL_SOCKET, SO_DEBUG, (void *)&one, sizeof(int)) == -1) {
                return errno;
            }
            ce_set_option(sock, CE_SO_DEBUG, on);
        }
        break;
    case CE_SO_REUSEADDR:
        if (on != ce_is_option_set(sock, CE_SO_REUSEADDR)) {
            if (setsockopt(sock->socketdes, SOL_SOCKET, SO_REUSEADDR, (void *)&one, sizeof(int)) == -1) {
                return errno;
            }
            ce_set_option(sock, CE_SO_REUSEADDR, on);
        }
        break;
    case CE_SO_SNDBUF:
#ifdef SO_SNDBUF
        if (setsockopt(sock->socketdes, SOL_SOCKET, SO_SNDBUF, (void *)&on, sizeof(int)) == -1) {
            return errno;
        }
#else
        return CE_ENOTIMPL;
#endif
        break;
    case CE_SO_RCVBUF:
#ifdef SO_RCVBUF
        if (setsockopt(sock->socketdes, SOL_SOCKET, SO_RCVBUF, (void *)&on, sizeof(int)) == -1) {
            return errno;
        }
#else
        return CE_ENOTIMPL;
#endif
        break;
    case CE_SO_NONBLOCK:
        if (ce_is_option_set(sock, CE_SO_NONBLOCK) != on) {
            if (on) {
                if ((rv = sononblock(sock->socketdes)) != CE_OK) 
                    return rv;
            }
            else {
                if ((rv = soblock(sock->socketdes)) != CE_OK)
                    return rv;
            }
            ce_set_option(sock, CE_SO_NONBLOCK, on);
        }
        break;
    case CE_SO_LINGER:
#ifdef SO_LINGER
        if (ce_is_option_set(sock, CE_SO_LINGER) != on) {
            struct linger li;
            li.l_onoff = on;
            li.l_linger = CE_MAX_SECS_TO_LINGER;
            if (setsockopt(sock->socketdes, SOL_SOCKET, SO_LINGER, (char *) &li, sizeof(struct linger)) == -1) {
                return errno;
            }
            ce_set_option(sock, CE_SO_LINGER, on);
        }
#else
        return CE_ENOTIMPL;
#endif
        break;
    case CE_TCP_DEFER_ACCEPT:
#if defined(TCP_DEFER_ACCEPT)
        if (ce_is_option_set(sock, CE_TCP_DEFER_ACCEPT) != on) {
            int optlevel = IPPROTO_TCP;
            int optname = TCP_DEFER_ACCEPT;

            if (setsockopt(sock->socketdes, optlevel, optname, 
                           (void *)&on, sizeof(int)) == -1) {
                return errno;
            }
            ce_set_option(sock, CE_TCP_DEFER_ACCEPT, on);
        }
#else
        return CE_ENOTIMPL;
#endif
        break;
    case CE_TCP_NODELAY:
#if defined(TCP_NODELAY)
        if (ce_is_option_set(sock, CE_TCP_NODELAY) != on) {
            int optlevel = IPPROTO_TCP;
            int optname = TCP_NODELAY;

#if CE_HAVE_SCTP
            if (sock->protocol == IPPROTO_SCTP) {
                optlevel = IPPROTO_SCTP;
                optname = SCTP_NODELAY;
            }
#endif
            if (setsockopt(sock->socketdes, optlevel, optname, (void *)&on, sizeof(int)) == -1) {
                return errno;
            }
            ce_set_option(sock, CE_TCP_NODELAY, on);
        }
#else
        /* BeOS pre-BONE has TCP_NODELAY set by default.
         * As it can't be turned off we might as well check if they're asking
         * for it to be turned on!
         */
#ifdef BEOS
        if (on == 1)
            return CE_OK;
        else
#endif
        return CE_ENOTIMPL;
#endif
        break;
    case CE_TCP_NOPUSH:
#if CE_TCP_NOPUSH_FLAG
        /* TCP_NODELAY and TCP_CORK are mutually exclusive on Linux
         * kernels < 2.6; on newer kernels they can be used together
         * and TCP_CORK takes preference, which is the desired
         * behaviour.  On older kernels, TCP_NODELAY must be toggled
         * to "off" whilst TCP_CORK is in effect. */
        if (ce_is_option_set(sock, CE_TCP_NOPUSH) != on) {
#ifndef HAVE_TCP_NODELAY_WITH_CORK
            int optlevel = IPPROTO_TCP;
            int optname = TCP_NODELAY;

#if CE_HAVE_SCTP
            if (sock->protocol == IPPROTO_SCTP) {
                optlevel = IPPROTO_SCTP;
                optname = SCTP_NODELAY;
            }
#endif
            /* OK we're going to change some settings here... */
            if (ce_is_option_set(sock, CE_TCP_NODELAY) == 1 && on) {
                /* Now toggle TCP_NODELAY to off, if TCP_CORK is being
                 * turned on: */
                int tmpflag = 0;
                if (setsockopt(sock->socketdes, optlevel, optname,
                               (void*)&tmpflag, sizeof(int)) == -1) {
                    return errno;
                }
                ce_set_option(sock, CE_RESET_NODELAY, 1);
                ce_set_option(sock, CE_TCP_NODELAY, 0);
            } else if (on) {
                ce_set_option(sock, CE_RESET_NODELAY, 0);
            }
#endif /* HAVE_TCP_NODELAY_WITH_CORK */

            /* OK, now we can just set the TCP_NOPUSH flag accordingly...*/
            if (setsockopt(sock->socketdes, IPPROTO_TCP, CE_TCP_NOPUSH_FLAG,
                           (void*)&on, sizeof(int)) == -1) {
                return errno;
            }
            ce_set_option(sock, CE_TCP_NOPUSH, on);
#ifndef HAVE_TCP_NODELAY_WITH_CORK
            if (!on && ce_is_option_set(sock, CE_RESET_NODELAY)) {
                /* Now, if TCP_CORK was just turned off, turn
                 * TCP_NODELAY back on again if it was earlier toggled
                 * to off: */
                int tmpflag = 1;
                if (setsockopt(sock->socketdes, optlevel, optname,
                               (void*)&tmpflag, sizeof(int)) == -1) {
                    return errno;
                }
                ce_set_option(sock, CE_RESET_NODELAY,0);
                ce_set_option(sock, CE_TCP_NODELAY, 1);
            }
#endif /* HAVE_TCP_NODELAY_WITH_CORK */
        }
#endif
        break;
    case CE_INCOMPLETE_READ:
        ce_set_option(sock, CE_INCOMPLETE_READ, on);
        break;
    case CE_IPV6_V6ONLY:
#if CE_HAVE_IPV6 && defined(IPV6_V6ONLY)
        /* we don't know the initial setting of this option,
         * so don't check sock->options since that optimization
         * won't work
         */
        if (setsockopt(sock->socketdes, IPPROTO_IPV6, IPV6_V6ONLY,
                       (void *)&on, sizeof(int)) == -1) {
            return errno;
        }
        ce_set_option(sock, CE_IPV6_V6ONLY, on);
#endif
        break;
    default:
        return CE_EINVAL;
    }

    return CE_OK; 
}         


ce_int_t ce_socket_timeout_get(ce_socket_t *sock, ce_interval_time_t *t)
{
    *t = sock->timeout;
    return CE_OK;
}


ce_int_t ce_socket_opt_get(ce_socket_t *sock, 
                                ce_int32_t opt, ce_int32_t *on)
{
    switch(opt) {
        default:
            *on = ce_is_option_set(sock, opt);
    }
    return CE_OK;
}


ce_int_t ce_socket_atmark(ce_socket_t *sock, int *atmark)
{
    int oobmark;

    if (ioctl(sock->socketdes, SIOCATMARK, (void*) &oobmark) < 0)
        return CE_ERROR;

    *atmark = (oobmark != 0);

    return CE_OK;
}

ce_int_t ce_gethostname(char *buf, size_t len, ce_pool_t *cont)
{
    cont=cont;
    if (gethostname(buf, len) != 0) {
        buf[0] = '\0';
        return errno;
    }
    else if (!memchr(buf, '\0', len)) { /* buffer too small */
        /* note... most platforms just truncate in this condition
         *         linux+glibc return an error
         */
        buf[0] = '\0';
        return CE_ENAMETOOLONG;
    }
    return CE_OK;
}


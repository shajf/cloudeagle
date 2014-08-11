
/*
 * =====================================================================================
 *
 *       Filename: ce_sendrecv.c 
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
#include "ce_basicdefs.h"
#include "ce_poll.h"
#include "ce_file.h"


ce_int_t ce_socket_send(ce_socket_t *sock, const char *buf, 
                             size_t *len)
{
   ssize_t rv;
    
    if (sock->options & CE_INCOMPLETE_WRITE) {
        sock->options &= ~CE_INCOMPLETE_WRITE;
        goto do_select;
    }

    do {
        rv = write(sock->socketdes, buf, (*len));
    } while (rv == -1 && errno == EINTR);

    while (rv == -1 && (errno == EAGAIN || errno == EWOULDBLOCK) 
                    && (sock->timeout > 0)) {
        ce_int_t arv;
do_select:
        arv = ce_wait_for_io_or_timeout(NULL, sock, 0);
        if (arv != CE_OK) {
            *len = 0;
            return arv;
        }
        else {
            do {
                rv = write(sock->socketdes, buf, (*len));
            } while (rv == -1 && errno == EINTR);
        }
    }
    if (rv == -1) {
        *len = 0;
        return errno;
    }
    if ((sock->timeout > 0) && (rv < (ssize_t)(*len))) {
        sock->options |= CE_INCOMPLETE_WRITE;
    }
    (*len) = rv;
    return CE_OK;
}

ce_int_t ce_socket_recv(ce_socket_t *sock, char *buf, size_t *len)
{
    ssize_t rv;
    ce_int_t arv;

    if (sock->options & CE_INCOMPLETE_READ) {
        sock->options &= ~CE_INCOMPLETE_READ;
        goto do_select;
    }

    do {
        rv = read(sock->socketdes, buf, (*len));
    } while (rv == -1 && errno == EINTR);

    while ((rv == -1) && (errno == EAGAIN || errno == EWOULDBLOCK)
                      && (sock->timeout > 0)) {
do_select:
        arv = ce_wait_for_io_or_timeout(NULL, sock, 1);
        if (arv != CE_OK) {
            *len = 0;
            return arv;
        }
        else {
            do {
                rv = read(sock->socketdes, buf, (*len));
            } while (rv == -1 && errno == EINTR);
        }
    }
    if (rv == -1) {
        (*len) = 0;
        return errno;
    }
    if ((sock->timeout > 0) && (rv < (ssize_t)*len)) {
        sock->options |= CE_INCOMPLETE_READ;
    }
    (*len) = rv;
    if (rv == 0) {
        return CE_EOF;
    }
    return CE_OK;
}

ce_int_t ce_socket_sendto(ce_socket_t *sock, ce_sockaddr_t *where,
                               ce_int32_t flags, const char *buf,
                               size_t *len)
{
    ssize_t rv;

    do {
        rv = sendto(sock->socketdes, buf, (*len), flags, 
                    (const struct sockaddr*)&where->sa, 
                    where->salen);
    } while (rv == -1 && errno == EINTR);

    while ((rv == -1) && (errno == EAGAIN || errno == EWOULDBLOCK)
                      && (sock->timeout > 0)) {
        ce_int_t arv = ce_wait_for_io_or_timeout(NULL, sock, 0);
        if (arv != CE_OK) {
            *len = 0;
            return arv;
        } else {
            do {
                rv = sendto(sock->socketdes, buf, (*len), flags,
                            (const struct sockaddr*)&where->sa,
                            where->salen);
            } while (rv == -1 && errno == EINTR);
        }
    }
    if (rv == -1) {
        *len = 0;
        return errno;
    }
    *len = rv;
    return CE_OK;
}

ce_int_t ce_socket_recvfrom(ce_sockaddr_t *from, ce_socket_t *sock,
                                 ce_int32_t flags, char *buf, 
                                 size_t *len)
{
    ssize_t rv;
    
    from->salen = sizeof(from->sa);

    do {
        rv = recvfrom(sock->socketdes, buf, (*len), flags, 
                      (struct sockaddr*)&from->sa, &from->salen);
    } while (rv == -1 && errno == EINTR);

    while ((rv == -1) && (errno == EAGAIN || errno == EWOULDBLOCK)
                      && (sock->timeout > 0)) {
        ce_int_t arv = ce_wait_for_io_or_timeout(NULL, sock, 1);
        if (arv != CE_OK) {
            *len = 0;
            return arv;
        } else {
            do {
                rv = recvfrom(sock->socketdes, buf, (*len), flags,
                              (struct sockaddr*)&from->sa, &from->salen);
            } while (rv == -1 && errno == EINTR);
        }
    }
    if (rv == -1) {
        (*len) = 0;
        return errno;
    }

    ce_sockaddr_vars_set(from, from->sa.sin.sin_family, ntohs(from->sa.sin.sin_port));

    (*len) = rv;
    if (rv == 0 && sock->type == SOCK_STREAM) {
        return CE_EOF;
    }

    return CE_OK;
}

ce_int_t ce_socket_sendv(ce_socket_t * sock, const struct iovec *vec,
                              ce_int32_t nvec, size_t *len)
{
    ssize_t rv;
    size_t requested_len = 0;
    ce_int32_t i;

    for (i = 0; i < nvec; i++) {
        requested_len += vec[i].iov_len;
    }

    if (sock->options & CE_INCOMPLETE_WRITE) {
        sock->options &= ~CE_INCOMPLETE_WRITE;
        goto do_select;
    }

    do {
        rv = writev(sock->socketdes, vec, nvec);
    } while (rv == -1 && errno == EINTR);

    while ((rv == -1) && (errno == EAGAIN || errno == EWOULDBLOCK) 
                      && (sock->timeout > 0)) {
        ce_int_t arv;
do_select:
        arv = ce_wait_for_io_or_timeout(NULL, sock, 0);
        if (arv != CE_OK) {
            *len = 0;
            return arv;
        }
        else {
            do {
                rv = writev(sock->socketdes, vec, nvec);
            } while (rv == -1 && errno == EINTR);
        }
    }
    if (rv == -1) {
        *len = 0;
        return errno;
    }
    if ((sock->timeout > 0) && (rv < (ssize_t)requested_len)) {
        sock->options |= CE_INCOMPLETE_WRITE;
    }
    (*len) = rv;
    return CE_OK;
}


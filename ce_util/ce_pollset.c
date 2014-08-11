/*
 * =====================================================================================
 *
 *       Filename:  ce_pollset.c
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

#include "ce_poll.h"
#include "ce_time.h"
#include "ce_file.h"
#include "ce_network_io.h"

#define CE_EINIT	-4

static ce_pollset_method_e pollset_default_method = POLLSET_DEFAULT_METHOD;


/* Create a dummy wakeup pipe for interrupting the poller
 */
/*static ce_int_t create_wakeup_pipe(ce_pollset_t *pollset)
{
    ce_int_t rv;

    if ((rv = ce_file_pipe_create(&pollset->wakeup_pipe[0],
                                   &pollset->wakeup_pipe[1],
                                   pollset->pool)) != CE_OK)
        return rv;

    pollset->wakeup_pfd.p = pollset->pool;
    pollset->wakeup_pfd.reqevents = CE_POLLIN;
    pollset->wakeup_pfd.desc_type = CE_POLL_FILE;
    pollset->wakeup_pfd.desc.f = pollset->wakeup_pipe[0];

    {
        int flags;

        if ((flags = fcntl(pollset->wakeup_pipe[0]->fd, F_GETFD)) == -1)
            return errno;

    }
    {
        int flags;

        if ((flags = fcntl(pollset->wakeup_pipe[1]->fd, F_GETFD)) == -1)
            return errno;

    }

    return ce_pollset_add(pollset, &pollset->wakeup_pfd);
}*/


static ce_int_t pollset_cleanup(void *p)
{
    ce_pollset_t *pollset = (ce_pollset_t *) p;
    if (pollset->flags & CE_POLLSET_WAKEABLE) {
        /* Close both sides of the wakeup pipe */
        if (pollset->wakeup_pipe[0]) {
            ce_close_file((int)pollset->wakeup_pipe[0]);
            pollset->wakeup_pipe[0] = NULL;
        }
        if (pollset->wakeup_pipe[1]) {
            ce_close_file((int)pollset->wakeup_pipe[1]);
            pollset->wakeup_pipe[1] = NULL;
        }
    }

    return CE_OK;
}

extern ce_pollset_provider_t *ce_pollset_provider_epoll;

static ce_pollset_provider_t *pollset_provider(ce_pollset_method_e method)
{
	     method=method;
             return ce_pollset_provider_epoll;
}

ce_int_t ce_pollset_create_ex(ce_pollset_t **ret_pollset,
                                                ce_uint32_t size,
                                                ce_pool_t *p,
                                                ce_uint32_t flags,
                                                ce_pollset_method_e method)
{
    ce_int_t rv;
    ce_pollset_t *pollset;
    ce_pollset_provider_t *provider = NULL;

    *ret_pollset = NULL;


    if (method == CE_POLLSET_DEFAULT)
        method = pollset_default_method;
    while (provider == NULL) {
        provider = pollset_provider(method);
        if (!provider) {
            if ((flags & CE_POLLSET_NODEFAULT) == CE_POLLSET_NODEFAULT)
                return CE_ENOTIMPL;
            if (method == pollset_default_method)
                return CE_ENOTIMPL;
            method = pollset_default_method;
        }
    }
    if (flags & CE_POLLSET_WAKEABLE) {
        /* Add room for wakeup descriptor */
        size++;
    }

    pollset =(ce_pollset_t *)ce_palloc(p, sizeof(*pollset));
    pollset->nelts = 0;
    pollset->nalloc = size;
    pollset->pool = p;
    pollset->flags = flags;
    pollset->provider = provider;

    rv = (*provider->create)(pollset, size, p, flags);
    if (rv == CE_ENOTIMPL) {
        if (method == pollset_default_method) {
            return rv;
        }
        provider = pollset_provider(pollset_default_method);
        if (!provider) {
            return CE_ENOTIMPL;
        }
        rv = (*provider->create)(pollset, size, p, flags);
        if (rv != CE_OK) {
            return rv;
        }
        pollset->provider = provider;
    }
    else if (rv != CE_OK) {
        return rv;
    }
    if (flags & CE_POLLSET_WAKEABLE) {
        /* Create wakeup pipe */
        /*if ((rv = create_wakeup_pipe(pollset)) != CE_OK) {
            return rv;
        }*/
    }

    *ret_pollset = pollset;
    return CE_OK;
}

 char * ce_pollset_method_name(ce_pollset_t *pollset)
{
    return pollset->provider->name;
}

 char * ce_poll_method_defname()
{
    ce_pollset_provider_t *provider = NULL;

    provider = pollset_provider(pollset_default_method);
    if (provider)
        return provider->name;
    else
        return "unknown";
}

ce_int_t ce_pollset_create(ce_pollset_t **pollset,
                                             ce_uint32_t size,
                                             ce_pool_t *p,
                                             ce_uint32_t flags)
{
    ce_pollset_method_e method = CE_POLLSET_DEFAULT;
    return ce_pollset_create_ex(pollset, size, p, flags, method);
}

ce_int_t ce_pollset_destroy(ce_pollset_t * pollset)
{
	pollset_cleanup(pollset);
        return CE_OK;
}

/*ce_int_t ce_pollset_wakeup(ce_pollset_t *pollset)
{
    if (pollset->flags & CE_POLLSET_WAKEABLE)
        return ce_file_putc(1, pollset->wakeup_pipe[1]);
    else
        return CE_EINIT;
}*/

ce_int_t ce_pollset_add(ce_pollset_t *pollset,
                                           ce_pollfd_t *descriptor)
{
    return (*pollset->provider->add)(pollset, descriptor);
}

ce_int_t ce_pollset_remove(ce_pollset_t *pollset,
                                              ce_pollfd_t *descriptor)
{
    return (*pollset->provider->remove)(pollset, descriptor);
}

ce_int_t ce_pollset_poll(ce_pollset_t *pollset,
                                           ce_interval_time_t timeout,
                                           ce_int32_t *num,
                                            ce_pollfd_t **descriptors)
{
    return (*pollset->provider->poll)(pollset, timeout, num, descriptors);
}

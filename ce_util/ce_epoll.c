/*
 * =====================================================================================
 *
 *       Filename:  ce_epoll.c
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
#include <poll.h>

#define CE_NOTFOUND -1
#define CE_TIMEUP -2

static ce_int16_t get_epoll_event(ce_int16_t event)
{
    ce_int16_t rv = 0;

    if (event & CE_POLLIN)
        rv |= EPOLLIN;
    if (event & CE_POLLPRI)
        rv |= EPOLLPRI;
    if (event & CE_POLLOUT)
        rv |= EPOLLOUT;
    /* CE_POLLNVAL is not handled by epoll.  EPOLLERR and EPOLLHUP are return-only */

    return rv;
}

static ce_int16_t get_epoll_revent(ce_int16_t event)
{
    ce_int16_t rv = 0;

    if (event & EPOLLIN)
        rv |= CE_POLLIN;
    if (event & EPOLLPRI)
        rv |= CE_POLLPRI;
    if (event & EPOLLOUT)
        rv |= CE_POLLOUT;
    if (event & EPOLLERR)
        rv |= CE_POLLERR;
    if (event & EPOLLHUP)
        rv |= CE_POLLHUP;
    /* CE_POLLNVAL is not handled by epoll. */

    return rv;
}

struct ce_pollset_private_t
{
    int epoll_fd;
    struct epoll_event *pollset;
    ce_pollfd_t *result_set;
    /* A thread mutex to protect operations on the rings */
    ce_thread_mutex_t *ring_lock;
    /* A ring coceining all of the pollfd_t that are active */
    CE_RING_HEAD(pfd_query_ring_t, pfd_elem_t) query_ring;
    /* A ring of pollfd_t that have been used, and then _remove()'d */
    CE_RING_HEAD(pfd_free_ring_t, pfd_elem_t) free_ring; 
    /* A ring of pollfd_t where rings that have been _remove()`ed but
        might still be inside a _poll() */
    CE_RING_HEAD(pfd_dead_ring_t, pfd_elem_t) dead_ring;
};

#if 0
static ce_int_t impl_pollset_cleanup(ce_pollset_t *pollset) __attribute__((unused))
{
    close(pollset->p->epoll_fd);
    return CE_OK;
}
#endif

 ce_int_t ce_pollfd_create(ce_pool_t *pool,
				     ce_pollfd_t **pfd,
				     ce_datatype_e fd_type,
				     ce_int16_t reqevents,
				     ce_int16_t rtnevents,
				     void *user_data,
				     void* fd_ptr)
{
	if(pool==NULL)
	{
		return CE_ERROR;
	}

	*pfd=(ce_pollfd_t*)ce_palloc(pool,CE_POLLFD_T_SIZE);

	if(*pfd==NULL)
	{
		return CE_ERROR;
	}
	(*pfd)->p=pool;
	(*pfd)->desc_type=fd_type;
	(*pfd)->reqevents=reqevents;
	(*pfd)->rtnevents=rtnevents;
	(*pfd)->client_data=user_data;
	if((*pfd)->desc_type==CE_POLL_SOCKET)
	{
		(*pfd)->desc.s=(ce_socket_t*)fd_ptr;
	//	(*pfd)->desc.f=NULL;
	}
	else if((*pfd)->desc_type==CE_POLL_FILE)
	{
		(*pfd)->desc.f=(ce_file_t*)fd_ptr;
	//	(*pfd)->desc.s=NULL;
	}
	else
	{
		return CE_ERROR;
	}
	(*pfd)->accepted=0;
	(*pfd)->read=0;
	(*pfd)->write=0;

	return CE_OK;
}

static ce_int_t impl_pollset_create(ce_pollset_t *pollset,
                                        ce_uint32_t size,
                                        ce_pool_t *p,
                                        ce_uint32_t flags)
{
    
	ce_int_t rv;
	int fd;
	
       	fd = epoll_create(size);
    
	if (fd < 0) 
	{
		pollset->p = NULL;
		return CE_ERROR ;
	}


	{
        int flags1;

        if ((flags1 = fcntl(fd, F_GETFD)) == -1)
            return errno;
	}


       	pollset->p = (ce_pollset_private_t*)ce_palloc(p, 
				sizeof(ce_pollset_private_t));

        if((rv = ce_thread_mutex_create(&pollset->p->ring_lock,
                                       0,
                                       p)) != CE_OK) 
	{
		pollset->p = NULL;
		return rv;
       	}
	
	pollset->p->epoll_fd = fd;
	pollset->p->pollset = (struct epoll_event*)ce_palloc(p,
				size * sizeof(struct epoll_event));

	pollset->p->result_set =(ce_pollfd_t *)ce_palloc(p, size * sizeof(ce_pollfd_t));

    if (!(flags & CE_POLLSET_NOCOPY)) {
        CE_RING_INIT(&pollset->p->query_ring, pfd_elem_t, link);
        CE_RING_INIT(&pollset->p->free_ring, pfd_elem_t, link);
        CE_RING_INIT(&pollset->p->dead_ring, pfd_elem_t, link);
    }
    return CE_OK;
}

static ce_int_t impl_pollset_add(ce_pollset_t *pollset,
                                      ce_pollfd_t *descriptor)
{
    struct epoll_event ev;
    memset(&ev,0,sizeof(struct epoll_event));
    int ret = -1;
    pfd_elem_t *elem = NULL;
    ce_int_t rv = CE_OK;

    ev.events = get_epoll_event(descriptor->reqevents);

    if (pollset->flags & CE_POLLSET_NOCOPY) {
        ev.data.ptr=(void*)descriptor;
    }
    else {
        pollset_lock_rings();

        if (!CE_RING_EMPTY(&(pollset->p->free_ring), pfd_elem_t, link)) {
            elem = CE_RING_FIRST(&(pollset->p->free_ring));
            CE_RING_REMOVE(elem, link);
        }
        else {
            elem = (pfd_elem_t *) ce_palloc(pollset->pool, sizeof(pfd_elem_t));
            CE_RING_ELEM_INIT(elem, link);
        }
        elem->pfd = *descriptor;
        ev.data.ptr = elem;
    }
    if (descriptor->desc_type == CE_POLL_SOCKET) {
        ret = epoll_ctl(pollset->p->epoll_fd, EPOLL_CTL_ADD,
                        descriptor->desc.s->socketdes, &ev);
    }
    else {
        ret = epoll_ctl(pollset->p->epoll_fd, EPOLL_CTL_ADD,
                        descriptor->desc.f->fd, &ev);
    }

    if (0 != ret) {
        rv = CE_ERROR;
    }

    if (!(pollset->flags & CE_POLLSET_NOCOPY)) {
        if (rv != CE_OK) {
            CE_RING_INSERT_TAIL(&(pollset->p->free_ring), elem, pfd_elem_t, link);
        }
        else {
            CE_RING_INSERT_TAIL(&(pollset->p->query_ring), elem, pfd_elem_t, link);
        }
        pollset_unlock_rings();
    }

    return rv;
}

static ce_int_t impl_pollset_remove(ce_pollset_t *pollset,
                                         ce_pollfd_t *descriptor)
{
    pfd_elem_t *ep;
    ce_int_t rv = CE_OK;
    struct epoll_event ev; /* ignored, but must be passed with
                                  * kernel < 2.6.9
                                  */
    memset(&ev,0,sizeof(struct epoll_event));
    int ret = -1;

    if (descriptor->desc_type == CE_POLL_SOCKET) {
        ret = epoll_ctl(pollset->p->epoll_fd, EPOLL_CTL_DEL,
                        descriptor->desc.s->socketdes, &ev);
    }
    else {
        ret = epoll_ctl(pollset->p->epoll_fd, EPOLL_CTL_DEL,
                        descriptor->desc.f->fd, &ev);
    }
    if (ret < 0) {
        rv = CE_NOTFOUND;
    }

    if (!(pollset->flags & CE_POLLSET_NOCOPY)) {
        pollset_lock_rings();

        for (ep = CE_RING_FIRST(&(pollset->p->query_ring));
             ep != CE_RING_SENTINEL(&(pollset->p->query_ring),
                                     pfd_elem_t, link);
             ep = CE_RING_NEXT(ep, link)) {
                
            if (descriptor->desc.s == ep->pfd.desc.s) {
                CE_RING_REMOVE(ep, link);
                CE_RING_INSERT_TAIL(&(pollset->p->dead_ring),
                                     ep, pfd_elem_t, link);
                break;
            }
        }

        pollset_unlock_rings();
    }

    return rv;
}

static ce_int_t impl_pollset_poll(ce_pollset_t *pollset,
                                           ce_interval_time_t timeout,
                                           ce_int32_t *num,
                                            ce_pollfd_t **descriptors)
{
    int ret, i, j;
    ce_int_t rv = CE_OK;
    ce_pollfd_t *fdptr;

    if (timeout > 0) {
        timeout /= 1000;
    }

    ret = epoll_wait(pollset->p->epoll_fd, pollset->p->pollset, pollset->nalloc,
                     timeout);
    (*num) = ret;

    if (ret < 0) {
        rv = CE_ERROR;
    }
    else if (ret == 0) {
        rv = CE_TIMEUP;
    }
    else {
        for (i = 0, j = 0; i < ret; i++) {
            if (pollset->flags & CE_POLLSET_NOCOPY) {
                fdptr = (ce_pollfd_t *)(pollset->p->pollset[i].data.ptr);
            }
            else {
                fdptr = &(((pfd_elem_t *) (pollset->p->pollset[i].data.ptr))->pfd);
            }
            /* Check if the polled descriptor is our
             * wakeup pipe. In that case do not put it result set.
             */
            if ((pollset->flags & CE_POLLSET_WAKEABLE) &&
                fdptr->desc_type == CE_POLL_FILE &&
                fdptr->desc.f == pollset->wakeup_pipe[0]) {
              //  ce_pollset_drain_wakeup_pipe(pollset);
                rv = CE_EINTR;
            }
            else {
                pollset->p->result_set[j] = *fdptr;
                pollset->p->result_set[j].rtnevents =
                    get_epoll_revent(pollset->p->pollset[i].events);
                j++;
            }
        }
        if (((*num) = j)) { /* any event besides wakeup pipe? */
            rv = CE_OK;

            if (descriptors) {
                *descriptors = pollset->p->result_set;
            }
        }
    }

    if (!(pollset->flags & CE_POLLSET_NOCOPY)) {
        pollset_lock_rings();

        /* Shift all PFDs in the Dead Ring to the Free Ring */
        CE_RING_CONCAT(&(pollset->p->free_ring), &(pollset->p->dead_ring), pfd_elem_t, link);

        pollset_unlock_rings();
    }

    return rv;
}

static ce_pollset_provider_t impl = {
    impl_pollset_create,
    impl_pollset_add,
    impl_pollset_remove,
    impl_pollset_poll,
    "epoll"
};

ce_pollset_provider_t *ce_pollset_provider_epoll = &impl;


ce_int_t ce_wait_for_io_or_timeout(ce_file_t *f,
				        ce_socket_t *s,
                                        int for_read)
{
    f=f;
    struct pollfd pfd;
    int rc;
    int timeout;
    timeout    = s->timeout / 1000;
    pfd.fd     =  s->socketdes;
    pfd.events = for_read ? POLLIN: POLLOUT;

    do {
        rc = poll(&pfd, 1, timeout);
    } while (rc == -1 && errno == EINTR);
    if (rc == 0) {
        return CE_TIMEUP;
    }
    else if (rc > 0) {
        return CE_OK;
    }
    else {
        return errno;
    }
}


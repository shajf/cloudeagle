/*
 * =====================================================================================
 *
 *       Filename:  ce_poll.h
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

#ifndef CE_POLL_H
#define CE_POLL_H
/**
 * @file ce_poll.h
 * @brief CE Poll interface
 */
#include <sys/epoll.h>

#include "ce_basicdefs.h"
#include "ce_palloc.h"
#include "ce_errno.h"
#include "ce_file.h" 
#include "ce_network_io.h" 
#include "ce_thread_mutex.h"
#include "ce_ring.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup ce_poll Poll Routines
 * @ingroup CE 
 * @{
 */

/**
 * Poll options
 */
#define CE_POLLIN    0x001     /**< Can read without blocking */
#define CE_POLLPRI   0x002     /**< Priority data available */
#define CE_POLLOUT   0x004     /**< Can write without blocking */
#define CE_POLLERR   0x010     /**< Pending error */
#define CE_POLLHUP   0x020     /**< Hangup occurred */
#define CE_POLLNVAL  0x040     /**< Descriptor invalid */

/**
 * Pollset Flags
 */
#define CE_POLLSET_THREADSAFE 0x001 /**< Adding or removing a descriptor is
                                      * thread-safe
                                      */
#define CE_POLLSET_NOCOPY     0x002 /**< Descriptors passed to ce_pollset_add()
                                      * are not copied
                                      */
#define CE_POLLSET_WAKEABLE   0x004 /**< Poll operations are interruptable by
                                      * ce_pollset_wakeup()
                                      */
#define CE_POLLSET_NODEFAULT  0x010 /**< Do not try to use the default method if
                                      * the specified non-default method cannot be
                                      * used
                                      */

 #define pollset_lock_rings()\
 ce_thread_mutex_lock(pollset->p->ring_lock);

 #define pollset_unlock_rings() \
 ce_thread_mutex_unlock(pollset->p->ring_lock);

/**
 * Pollset Methods
 */
typedef enum {
    CE_POLLSET_DEFAULT,        /**< Platform default poll method */
    CE_POLLSET_SELECT,         /**< Poll uses select method */
    CE_POLLSET_KQUEUE,         /**< Poll uses kqueue method */
    CE_POLLSET_PORT,           /**< Poll uses Solaris event port method */
    CE_POLLSET_EPOLL,          /**< Poll uses epoll method */
    CE_POLLSET_POLL            /**< Poll uses poll method */
} ce_pollset_method_e;

#define POLLSET_DEFAULT_METHOD CE_POLLSET_EPOLL
/** Used in ce_pollfd_t to determine what the ce_descriptor is */
typedef enum { 
    CE_NO_DESC,                /**< nothing here */
    CE_POLL_SOCKET,            /**< descriptor refers to a socket */
    CE_POLL_FILE,              /**< descriptor refers to a file */
    CE_POLL_LASTDESC           /**< @deprecated descriptor is the last one in the list */
} ce_datatype_e ;

/** Union of either an CE file or socket. */
typedef union {
    ce_file_t *f;              /**< file */
    ce_socket_t *s;            /**< socket */
} ce_descriptor;

/** @see ce_pollfd_t */
typedef struct ce_pollfd_t ce_pollfd_t;

/** Poll descriptor set. */
struct ce_pollfd_t {
    ce_pool_t *p;              /**< associated pool */
    ce_datatype_e desc_type;   /**< descriptor type */
    ce_int16_t reqevents;      /**< requested events */
    ce_int16_t rtnevents;      /**< returned events */
    ce_descriptor desc;        /**< @see ce_descriptor */
    unsigned accepted:1;
    unsigned read:1;
    unsigned write:1;
    void *client_data;          /**< allows app to associate context */
};
#define CE_POLLFD_T_SIZE \
	sizeof(ce_pollfd_t)

typedef struct pfd_elem_t pfd_elem_t;
struct pfd_elem_t 
{
	CE_RING_ENTRY(pfd_elem_t) link;
	ce_pollfd_t pfd;
};


typedef struct ce_pollset_private_t ce_pollset_private_t;
typedef struct ce_pollset_provider_t ce_pollset_provider_t;


/* General-purpose poll API for arbitrarily large numbers of
 * file descriptors
 */

typedef struct ce_pollset_t ce_pollset_t;
struct ce_pollset_t
{
	ce_pool_t *pool;
	ce_uint32_t nelts;
	ce_uint32_t nalloc;
	ce_uint32_t flags;
	/* Pipe descriptors used for wakeup */
	ce_file_t *wakeup_pipe[2];
	ce_pollfd_t wakeup_pfd;
	ce_pollset_private_t *p;
	ce_pollset_provider_t *provider;
};
#define CE_POLLSET_T_SIZE \
	sizeof(ce_pollset_t)


struct ce_pollset_provider_t 
{
	ce_int_t (*create)(ce_pollset_t *,
			     ce_uint32_t,
			     ce_pool_t *,
			     ce_uint32_t);
	
	ce_int_t (*add)(ce_pollset_t *, 
			     ce_pollfd_t *);

	ce_int_t (*remove)(ce_pollset_t *, 
			      ce_pollfd_t *);
	ce_int_t (*poll)(ce_pollset_t *, 
			   ce_interval_time_t, 
			   ce_int32_t*,
			    ce_pollfd_t **);
	 char *name;
};


extern ce_int_t ce_pollfd_create(ce_pool_t *pool,
				     ce_pollfd_t **pfd,
				     ce_datatype_e fd_type,
				     ce_int16_t reqevents,
				     ce_int16_t rtnevents,
				     void *user_data,
				     void* fd_ptr);

extern ce_int_t ce_wait_for_io_or_timeout(ce_file_t *f,
				        ce_socket_t *s,
                                        int for_read);

/**
 * Set up a pollset object
 * @param pollset  The pointer in which to return the newly created object 
 * @param size The maximum number of descriptors that this pollset can hold
 * @param p The pool from which to allocate the pollset
 * @param flags Optional flags to modify the operation of the pollset.
 *
 * @remark If flags coceins CE_POLLSET_THREADSAFE, then a pollset is
 *         created on which it is safe to make concurrent calls to
 *         ce_pollset_add(), ce_pollset_remove() and ce_pollset_poll()
 *         from separate threads.  This feature is only supported on some
 *         platforms; the ce_pollset_create() call will fail with
 *         CE_ENOTIMPL on platforms where it is not supported.
 * @remark If flags coceins CE_POLLSET_WAKEABLE, then a pollset is
 *         created with an additional internal pipe object used for the
 *         ce_pollset_wakeup() call. The actual size of pollset is
 *         in that case size + 1. This feature is only supported on some
 *         platforms; the ce_pollset_create() call will fail with
 *         CE_ENOTIMPL on platforms where it is not supported.
 * @remark If flags coceins CE_POLLSET_NOCOPY, then the ce_pollfd_t
 *         structures passed to ce_pollset_add() are not copied and
 *         must have a lifetime at least as long as the pollset.
 * @remark Some poll methods (including CE_POLLSET_KQUEUE,
 *         CE_POLLSET_PORT, and CE_POLLSET_EPOLL) do not have a
 *         fixed limit on the size of the pollset. For these methods,
 *         the size parameter controls the maximum number of
 *         descriptors that will be returned by a single call to
 *         ce_pollset_poll().
 */
extern ce_int_t 
ce_pollset_create(ce_pollset_t **pollset,
			ce_uint32_t size,
			ce_pool_t *p,
			ce_uint32_t flags);

/**
 * Set up a pollset object
 * @param pollset  The pointer in which to return the newly created object 
 * @param size The maximum number of descriptors that this pollset can hold
 * @param p The pool from which to allocate the pollset
 * @param flags Optional flags to modify the operation of the pollset.
 * @param method Poll method to use. See #ce_pollset_method_e.  If this
 *         method cannot be used, the default method will be used unless the
 *         CE_POLLSET_NODEFAULT flag has been specified.
 *
 * @remark If flags coceins CE_POLLSET_THREADSAFE, then a pollset is
 *         created on which it is safe to make concurrent calls to
 *         ce_pollset_add(), ce_pollset_remove() and ce_pollset_poll()
 *         from separate threads.  This feature is only supported on some
 *         platforms; the ce_pollset_create_ex() call will fail with
 *         CE_ENOTIMPL on platforms where it is not supported.
 * @remark If flags coceins CE_POLLSET_WAKEABLE, then a pollset is
 *         created with additional internal pipe object used for the
 *         ce_pollset_wakeup() call. The actual size of pollset is
 *         in that case size + 1. This feature is only supported on some
 *         platforms; the ce_pollset_create_ex() call will fail with
 *         CE_ENOTIMPL on platforms where it is not supported.
 * @remark If flags coceins CE_POLLSET_NOCOPY, then the ce_pollfd_t
 *         structures passed to ce_pollset_add() are not copied and
 *         must have a lifetime at least as long as the pollset.
 * @remark Some poll methods (including CE_POLLSET_KQUEUE,
 *         CE_POLLSET_PORT, and CE_POLLSET_EPOLL) do not have a
 *         fixed limit on the size of the pollset. For these methods,
 *         the size parameter controls the maximum number of
 *         descriptors that will be returned by a single call to
 *         ce_pollset_poll().
 */
ce_int_t ce_pollset_create_ex(ce_pollset_t **pollset,
				ce_uint32_t size,
				ce_pool_t *p,
				ce_uint32_t flags,
				ce_pollset_method_e method);

/**
 * Destroy a pollset object
 * @param pollset The pollset to destroy
 */
ce_int_t ce_pollset_destroy(ce_pollset_t *pollset);

/**
 * Add a socket or file descriptor to a pollset
 * @param pollset The pollset to which to add the descriptor
 * @param descriptor The descriptor to add
 * @remark If you set client_data in the descriptor, that value
 *         will be returned in the client_data field whenever this
 *         descriptor is signalled in ce_pollset_poll().
 * @remark If the pollset has been created with CE_POLLSET_THREADSAFE
 *         and thread T1 is blocked in a call to ce_pollset_poll() for
 *         this same pollset that is being modified via ce_pollset_add()
 *         in thread T2, the currently executing ce_pollset_poll() call in
 *         T1 will either: (1) automatically include the newly added descriptor
 *         in the set of descriptors it is watching or (2) return immediately
 *         with CE_EINTR.  Option (1) is recommended, but option (2) is
 *         allowed for implemecetions where option (1) is impossible
 *         or impractical.
 * @remark If the pollset has been created with CE_POLLSET_NOCOPY, the 
 *         ce_pollfd_t structure referenced by descriptor will not be copied
 *         and must have a lifetime at least as long as the pollset.
 * @remark Do not add the same socket or file descriptor to the same pollset
 *         multiple times, even if the requested events differ for the 
 *         different calls to ce_pollset_add().  If the events of interest
 *         for a descriptor change, you must first remove the descriptor 
 *         from the pollset with ce_pollset_remove(), then add it again 
 *         specifying all requested events.
 */
ce_int_t ce_pollset_add(ce_pollset_t *pollset,
				 ce_pollfd_t *descriptor);

/**
 * Remove a descriptor from a pollset
 * @param pollset The pollset from which to remove the descriptor
 * @param descriptor The descriptor to remove
 * @remark If the pollset has been created with CE_POLLSET_THREADSAFE
 *         and thread T1 is blocked in a call to ce_pollset_poll() for
 *         this same pollset that is being modified via ce_pollset_remove()
 *         in thread T2, the currently executing ce_pollset_poll() call in
 *         T1 will either: (1) automatically exclude the newly added descriptor
 *         in the set of descriptors it is watching or (2) return immediately
 *         with CE_EINTR.  Option (1) is recommended, but option (2) is
 *         allowed for implemecetions where option (1) is impossible
 *         or impractical.
 * @remark ce_pollset_remove() cannot be used to remove a subset of requested
 *         events for a descriptor.  The reqevents field in the ce_pollfd_t
 *         parameter must cocein the same value when removing as when adding.
 */
ce_int_t ce_pollset_remove(ce_pollset_t *pollset,
				 ce_pollfd_t *descriptor);

/**
 * Block for activity on the descriptor(s) in a pollset
 * @param pollset The pollset to use
 * @param timeout The amount of time in microseconds to wait.  This is a
 *                maximum, not a minimum.  If a descriptor is signalled, the
 *                function will return before this time.  If timeout is
 *                negative, the function will block until a descriptor is
 *                signalled or until ce_pollset_wakeup() has been called.
 * @param num Number of signalled descriptors (output parameter)
 * @param descriptors Array of signalled descriptors (output parameter)
 * @remark CE_EINTR will be returned if the pollset has been created with
 *         CE_POLLSET_WAKEABLE, ce_pollset_wakeup() has been called while
 *         waiting for activity, and there were no signalled descriptors at the
 *         time of the wakeup call.
 * @remark Multiple signalled conditions for the same descriptor may be reported
 *         in one or more returned ce_pollfd_t structures, depending on the
 *         implemecetion.
 * @bug With versions 1.4.2 and prior on Windows, a call with no descriptors
 *      and timeout will return immediately with the wrong error code.
 */
ce_int_t ce_pollset_poll(ce_pollset_t *pollset,
				ce_interval_time_t timeout,
				ce_int32_t *num,
	       			 ce_pollfd_t **descriptors);

/**
 * Interrupt the blocked ce_pollset_poll() call.
 * @param pollset The pollset to use
 * @remark If the pollset was not created with CE_POLLSET_WAKEABLE the
 *         return value is CE_EINIT.
 */
ce_int_t ce_pollset_wakeup(ce_pollset_t *pollset);

/**
 * Poll the descriptors in the poll structure
 * @param ceset The poll structure we will be using. 
 * @param numsock The number of descriptors we are polling
 * @param nsds The number of descriptors signalled (output parameter)
	rc=ce_pollset_add(p_set,
	rc=ce_pollset_add(p_set,
	llset_poll
			   &pfd1);	
			   &pfd1);	
 * @param timeout The amount of time in microseconds to wait.  This is a
 *                maximum, not a minimum.  If a descriptor is signalled, the
 *                function will return before this time.  If timeout is
 *                negative, the function will block until a descriptor is
 *                signalled or until ce_pollset_wakeup() has been called.
 * @remark The number of descriptors signalled is returned in the third argument. 
 *         This is a blocking call, and it will not return until either a 
 *         descriptor has been signalled or the timeout has expired. 
 * @remark The rtnevents field in the ce_pollfd_t array will only be filled-
 *         in if the return value is CE_SUCCESS.
 * @bug With versions 1.4.2 and prior on Windows, a call with no descriptors
 *      and timeout will return immediately with the wrong error code.
 */
extern ce_int_t ce_poll(ce_pollfd_t *ceset, 
				ce_int32_t numsock,
                                   ce_int32_t *nsds, 
                                   ce_interval_time_t timeout);

/**
 * Return a priceble represecetion of the pollset method.
 * @param pollset The pollset to use
 */
extern  char * ce_pollset_method_name(ce_pollset_t *pollset);

/**
 * Return a priceble represecetion of the default pollset method
 * (CE_POLLSET_DEFAULT).
 */
extern  char * ce_poll_method_defname(void);



/** @} */

#ifdef __cplusplus
}
#endif

#endif  /* ! CE_POLL_H */


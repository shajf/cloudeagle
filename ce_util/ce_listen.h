/*
 * =====================================================================================
 *
 *       Filename:  ce_listen.h
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

#ifndef CE_LISTEN_H
#define CE_LISTEN_H

#include "ce_basicdefs.h"
#include "ce_network_io.h"
#include "ce_connection.h"


typedef struct ce_listen_rec_t ce_listen_rec_t;

typedef ce_int_t (*accept_function)(void **csd, 
				     ce_listen_rec_t *lr,
				     ce_pool_t *ptrans);

/**
 * @brief  listeners record.  
 *
 * These are used in the Multi-Processing Modules
 * to setup all of the sockets for the MPM to listen to and accept on.
 */
struct ce_listen_rec_t{
    /**
     * The next listener in the list
     */
    ce_listen_rec_t *next;
    /**
     * The actual socket 
     */
    ce_socket_t *sd;
    /**
     * The sockaddr the socket should bind to
     */
    ce_sockaddr_t *bind_addr;
    /**
     * The accept function for this socket
     */
    accept_function accept_func;
    /**
     * Is this socket currently active 
     */
    int active;
};

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/
/**
 * The global list of ce_listen_rec structures
 */
extern ce_listen_rec_t *ce_listeners;

/**
 * Setup all of the defaults for the listener list
 */
extern void ce_listen_pre_config(void);


extern 
ce_listen_rec_t *ce_open_listener(ce_pool_t *pool,
				  char *addr,
                                  int port);

extern void ce_close_listeners(void);


#ifdef __cplusplus
}
#endif

#endif
/** @} */

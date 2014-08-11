/*
 * =====================================================================================
 *
 *       Filename:  ce_connection.h
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  07/01/2013 13:56:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  NSFOCUS,2013,CE TEAM
 *
 * =====================================================================================
 */

#ifndef CE_CONNECTION_H
#define CE_CONNECTION_H
#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/

#include "ce_basicdefs.h"
#include "ce_network_io.h"
#include "ce_palloc.h"
#include "ce_poll.h"
#include "ce_buf.h"
/*forward declearation*/
typedef struct ce_reqrec_t ce_reqrec_t;
typedef struct ce_conrec_t ce_conrec_t;
typedef struct ce_server_rec_t ce_server_rec_t;
typedef struct ce_msg_t ce_msg_t;

/*brief:a structure to store a request's records*/
struct ce_reqrec_t 
{
	/*the pool associated with a request
	 * when the request procced completely,
	 * the pool will be destroy*/
	ce_pool_t *pool;

	/** byte count in stream is for content */
	off_t sent_cont_byte;
	/** content byte count, for easy access */
	off_t bytes_sent;
	/** The "real" content length */
	off_t clength;
	/** Remaining bytes left to read from the request body*/
	off_t remaining;
	/** Number of bytes that have been read  
	 * from the request body*/
	off_t read_length;

	/*buf will be used to store content of this request*/
	ce_buf_t *buf;

	/*the connection associated with this request*/
	ce_conrec_t *conrec;


};

/*brief Enumeration of connection keepalive options*/
typedef enum 
{
	CE_CONN_UNKNOWN,
	CE_CONN_CLOSE,
	CE_CONN_KEEPALIVE
}ce_conn_keepalive_e;

/*brief:a structure to store records which are per connection*/
struct ce_conrec_t
{
	/*the pool associated with a connection
	 * when the connection closed completely
	 * the pool will be destroy*/
	ce_pool_t *pool;
	ce_conrec_t *next;

	/*the request associated with this connection*/
	ce_reqrec_t *reqrec;
	

	ce_server_rec_t *server;/*<--server this conn came in on*/
	ce_msg_t *msg;

	ce_sockaddr_t *local_addr;/*<--local address*/
	ce_sockaddr_t *remote_addr;/*<--remote address*/

	char *remote_ip; /*<--Client's IP address */

	unsigned aborted:1;/*<-- Are we still talking?*/

	 /** Are we going to keep the connection alive 
	  * for another request?*/
	ce_conn_keepalive_e keepalive; 

	size_t  keepalives; /*<-- How many times have we used it?*/
	char * local_ip; /*<--server IP address*/


	ce_pollfd_t pfd;/*<--poll file decriptor information*/

};

 /*brief A structure to store information for each server*/

 struct ce_server_rec_t 
 {
	ce_server_rec_t *next;/*<--The next server in the list*/
	ce_sockaddr_t *host_addr; 
	ce_port_t host_port;/*<--server's port*/
	
 }; 



extern ce_int_t 
ce_connection_create(ce_pool_t *pool,
		       ce_socket_t *r_sock,
		       ce_conrec_t **connection);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_CONNECTION_H*/

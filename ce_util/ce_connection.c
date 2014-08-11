/*
 * =====================================================================================
 *
 *       Filename:  ce_connection.c
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

#include "ce_connection.h"
#include "ce_palloc.h"
#include "ce_network_io.h"

ce_int_t 
ce_connection_create(ce_pool_t *pool,
		       ce_socket_t *r_sock,
		       ce_conrec_t **connection)
{
	*connection=(ce_conrec_t*)ce_palloc(pool,
					sizeof(ce_conrec_t));
	if(*connection==NULL)
	{
		return CE_ERROR;
	}
	
	(*connection)->pool=pool;
	(*connection)->reqrec=NULL;
	(*connection)->server=NULL;
	(*connection)->local_addr=r_sock->local_addr;
	(*connection)->remote_addr=r_sock->remote_addr;
	(*connection)->remote_ip=NULL;
	(*connection)->aborted=0;
	(*connection)->msg=NULL;
	/*create pfd*/
	(*connection)->pfd.p=pool;
	(*connection)->pfd.desc_type=CE_POLL_SOCKET;
	(*connection)->pfd.reqevents=1;
	(*connection)->pfd.rtnevents=0;
	(*connection)->pfd.desc.s=r_sock;
	(*connection)->next=NULL;
	(*connection)->pfd.client_data=(void*)(*connection);
	return CE_OK;
}


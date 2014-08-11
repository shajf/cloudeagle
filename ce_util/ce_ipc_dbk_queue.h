/*
 * =====================================================================================
 *
 *       Filename:  ce_ipc_dbk_quue.h
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  08/01/2013 11:05:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */
#ifndef CE_IPC_DBK_QUEUE_H
#define CE_IPC_DBK_QUEUE_H
#include "ce_basicdefs.h"
#include "ce_string.h"

#define MAX_QUEUE_ELTS_N 1024
typedef struct 
{
	void * arr[MAX_QUEUE_ELTS_N];
	size_t front;
	size_t last;
	size_t count;
}ce_ipc_dbk_queue_t;

static inline void ce_ipc_dbk_queue_init(ce_ipc_dbk_queue_t *q)
{
	ce_memset(q->arr,0,MAX_QUEUE_ELTS_N);
	q->front = 0;
	q->last = 0;
	q->count = 0;
}

static inline ce_ipc_dbk_queue_t * ce_ipc_dbk_queue_create(ce_pool_t *pool)
{
	 ce_ipc_dbk_queue_t * queue = (ce_ipc_dbk_queue_t*)ce_palloc(pool,sizeof(ce_ipc_dbk_queue_t));
	 if(!queue)
		return NULL;

	ce_ipc_dbk_queue_init(queue);

	return queue;
}

static inline int ce_ipc_dbk_queue_empty(ce_ipc_dbk_queue_t *q)
{
	return q->count ==0;	
}

static inline int ce_ipc_dbk_queue_full(ce_ipc_dbk_queue_t *q)
{
	return (q->count>0&&q->last==q->front);
}

static inline ce_int_t ce_ipc_dbk_queue_push(ce_ipc_dbk_queue_t *q,void *data)
{
	if(ce_ipc_dbk_queue_full(q))
		return CE_ERROR;
	q->arr[q->last] = data;
	q->last = (q->last+1)%MAX_QUEUE_ELTS_N;
	q->count++;
	return CE_OK;
}

static inline void* ce_ipc_dbk_queue_pop(ce_ipc_dbk_queue_t *q)
{
	void *res;
	if(ce_ipc_dbk_queue_empty(q))
		return NULL;
	
	res = q->arr[q->front];
	q->front = (q->front+1)%MAX_QUEUE_ELTS_N;
	q->count--;
	return res;
}

#endif /*CE_IPC_DBK_QUEUE_H*/

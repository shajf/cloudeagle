/*
 * =====================================================================================
 *
 *       Filename:  ce_ipc_shm.h
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
#ifndef CE_IPC_SHM_H
#define CE_IPC_SHM_H

#include "ce_ipc_context.h"

#define CE_QUEUE_MAX_READER_NUM	5
#define CE_DBK_PUBLISHER_RCD_NUM	64
#define CE_DBK_MIN_NUM			1
/*32m*/
#define SHM_SIZE_DEFAULT 32*1024*1024 
#define SHM_QUEUE_SIZE_DEFAULT 4*1024*1024	
#define SHM_QUEUE_ALLOC_SIZE	1024

#define NTA_SHM_KEY "/var/run/engine/shm"
#define NTA_SHM_ID (1234)


#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/


extern void ipc_beat_funcs_set(beat_funcs_t *funcs);
extern void ipc_shm_debug(ce_ipc_t *ipc);

extern ce_ipc_t * ipc_shm_create(ce_ipc_context_t *context,const char *key,int id,size_t *shm_size, int dbk_pub_port);

extern int destory_ipc_context(ce_ipc_context_t *ipc_context);

extern ce_ipc_t * ipc_shm_open(ce_ipc_context_t *context);

extern ce_ipc_t * ipc_shm_open2(const char *key,int id);

extern ce_int_t ipc_shm_close2(ce_ipc_t *ipc);

extern void ipc_shm_debug(ce_ipc_t *ipc);

extern void exit_pub_dbk(ce_ipc_context_t *ipc_context);

extern ce_int_t   ipc_shm_queue_create(ce_ipc_context_t *context,int queue_id,size_t queue_size,size_t rcd_size);

extern ce_data_writer_t* ipc_shm_writer_register(ce_ipc_t *ipc,int q_id);

extern void ipc_shm_writer_unregister(ce_ipc_t *ipc,ce_data_writer_t *data_writer);

extern ce_data_reader_t* ipc_shm_reader_register(ce_ipc_t *ipc, int q_id,void *private_data, char *reader_name);

extern  void ipc_shm_reader_unregister(ce_ipc_t *ipc,ce_data_reader_t *data_reader);

extern ce_int_t ipc_shm_record_write(ce_data_writer_t* data_writer,ce_data_record_t *data_record,size_t *w_size);

extern ce_int_t ipc_shm_record_read(ce_data_reader_t *data_reader,ce_data_record_t *data_record,ce_ipc_read_handle_fn read_handle); 

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*CE_IPC_SHM_H*/

/*
 * =====================================================================================
 *
 *       Filename:  ce_ipc_context.h
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
#ifndef CE_IPC_CONTEXT_H
#define CE_IPC_CONTEXT_H
#include "ce_basicdefs.h"
#include "ce_palloc.h"
#include "ce_thread_proc.h"
#include "ce_string.h"
#include "ce_data_writer.h"
#include "ce_data_reader.h"
#include "ce_data_record.h"

#define MAX_RCD_SIZE 4096

typedef ce_int_t (*ce_ipc_read_handle_fn)(ce_data_reader_t *data_reader,ce_data_record_t *data_record,void *user_data);

typedef struct ce_ipc_context_t ce_ipc_context_t;
typedef struct ce_ipc_t ce_ipc_t;

enum 
{
	IPC_SOCK,
	IPC_SHM,
	IPC_TYPE_NUM
};

struct ce_ipc_context_t
{
	ce_pool_t *pool; 					//内存池要提前初始化,创建第一个写者,须先创建ipc_context
	ce_pool_t *tmp_pool;
	
	ce_thread_t *timeout_write_thread;  ///数据块老化线程,10s 未写被老化
    ce_bool_t is_thread_stop;
	ce_ipc_t *ipc;
	void *zmq_context;                  ///zmq环境
	void *zmq_pub_sock;                 ///发布socket

	pid_t creator_pid;					/// 创建context的进程
};//ipc上下文结构

struct ce_ipc_t
{
	ce_pool_t *pool;				///IPC内存池
	int ipc_type;					///IPC类型
    void *pub_sock;					///发布sock
	ce_str_t key;					///IPC key
	int id;
};


typedef struct beat_funcs
{
	void (*beat_register)(pthread_t tid, const char *thrd_name, int expire);
	void (*beat)(pthread_t tid);
	void (*beat_unregister)(pthread_t tid);
}beat_funcs_t;


#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

extern ce_ipc_context_t * ipc_create_context(void);


extern ce_ipc_t *ipc_create(ce_ipc_context_t *context,const char *key,int id,size_t *size, int dbk_pub_port,int ipc_type);
extern int ipc_destroy(ce_ipc_context_t *context,int ipc_type);
extern ce_int_t   ipc_queue_create(ce_ipc_context_t *context,int queue_id,size_t queue_size,size_t rcd_size,int ipc_type);

extern ce_ipc_t * ipc_open(ce_ipc_context_t *context,int ipc_type);

extern ce_ipc_t * ipc_open2(const char *key,int id,int ipc_type);

extern ce_int_t  ipc_close2(ce_ipc_t *ipc,int ipc_type);
extern ce_data_writer_t* ipc_writer_register(ce_ipc_t *ipc,int q_id,int ipc_type);
extern void ipc_writer_unregister(ce_ipc_t *ipc,ce_data_writer_t *d_writer,int ipc_type);

extern ce_data_reader_t* ipc_reader_register(ce_ipc_t *ipc, int q_id,int ipc_type,void *private_data, char *reader_name);
extern void ipc_reader_unregister(ce_ipc_t *ipc, ce_data_reader_t *d_reader,int ipc_type);

extern ce_int_t ipc_record_write(ce_data_writer_t* data_writer,ce_data_record_t *data_record,size_t *w_size,int ipc_type);

extern ce_int_t ipc_record_read(ce_data_reader_t *data_reader,ce_data_record_t *data_record,ce_ipc_read_handle_fn read_handle,int ipc_type); 

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*CE_IPC_CONTEXT_H*/

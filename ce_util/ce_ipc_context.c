/*
 * =====================================================================================
 *
 *       Filename:  ce_ipc_context.c
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  08/01/2013 11:05:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *
 * =====================================================================================
 */
#include "ce_ipc_context.h"
#include "ce_ipc_shm.h"
#include "ce_log.h"

/*brief: create a ipc context in pool;
 *@param: pool,mem pool
 *return: if ok,return context's pointer,otherwise return NULL
 */

ce_ipc_context_t * ipc_create_context(void)
{
    ce_pool_t *pool = ce_create_pool(4096);
	assert(pool);
	ce_ipc_context_t *context = (ce_ipc_context_t*)ce_palloc(pool,sizeof(ce_ipc_context_t));
	assert(context);
	context->pool = pool;
	context->tmp_pool = ce_create_pool(1024);

	if(context->tmp_pool==NULL)
	{
		return NULL;
	}
	context->timeout_write_thread=NULL;
	context->ipc=NULL;
	context->zmq_pub_sock=NULL;
	context->zmq_context=NULL;
	context->creator_pid = getpid();
//    ce_log_init();
//    ce_log_set_level(CE_LOG_DEBUG);
	return context;
}

/*
*brief:  此接口创建指定大小的共享内存，并初始化共享内存头部信息，需要在系统启动时调用此接口创建共享内存，以供各个模块对共享内存进行读写操作。
* @param: context  ipc 上下文结构指针；
* @param: key    共享内存key,必须是文件系统中有效的文件或是目录；
* @param: id      一个整型值，连同key 一起构建共享内存唯一的标识id;
* @param: size    要创建的共享内存大小，由于系统对共享内存的大小有所限制，所以size 还用来返回共享内存的实际大小；
* @ param: dbk_rcv_port 用来接收需要读取的数据块的端口；
* @param: dbk_pub_port 用来向所有读通告可读数据块的端口；
* @param: ipc_type 要创建的ipc 类型，目前只支持共享内存，所以调用时需要传递IPC_SHM;
* @return : 当共享内存创建成功时，返回共享内存的指针，否则返回NULL。
*/


ce_ipc_t *
ipc_create(ce_ipc_context_t *context,const char *key,int id,size_t *size, int dbk_pub_port,int ipc_type)
{
	switch(ipc_type)
	{
		case IPC_SHM:
			return ipc_shm_create(context,key,id,size, dbk_pub_port);

		default:

			return NULL;
	}
}
int
ipc_destroy(ce_ipc_context_t *context, int ipc_type)
{
	switch(ipc_type)
	{
		case IPC_SHM:
			return destory_ipc_context(context);

		default:

			return CE_ERROR;;
	}
}
ce_ipc_t * 
ipc_open(ce_ipc_context_t *context,int ipc_type)
{

	switch(ipc_type)
	{
		case IPC_SHM:
			return ipc_shm_open(context);

		default:

			return NULL;
	}
}

/*
* brief:  此接口用来连接共享内存，将共享内存地址映射到调用进程的内存空间，读写队列的进程必须要先调用此接口。
* @param: key 共享内存key,必须是文件系统中有效的文件或是目录；
* @param: id 一个整型值，连同key 一起构建共享内存唯一的标识id;
* @param: ipc_type 同上；
* @return: 若打开成功，则返回指向ce_ipc_t 的指针，否则返回NULL。
*/

ce_ipc_t * 
ipc_open2(const char* key,int id,int ipc_type)
{

	switch(ipc_type)
	{
		case IPC_SHM:
			return ipc_shm_open2(key,id);

		default:

			return NULL;
	}
}

ce_int_t  
ipc_close2(ce_ipc_t *ipc,int ipc_type)
{

	switch(ipc_type)
	{
		case IPC_SHM:
			return ipc_shm_close2(ipc);

		default:

			return CE_ERROR;
	}
}
/*
* brief: 此接口用来在共享内存上创建指定大小的队列，并对队列数据区进行分块。需要系统启动时调用创建队列。
* @param: context   ipc 上下文结构指针；
* @param: queue_id  队列唯一的标识符；
* @param: queue_size 要创建的队列大小；
* @param: rcd_size  往队列里面写的一条数据（记录）的最大大小，用于队列数据区分块；
* @param: ipc_type  同上；
*  @return : 若创建成功则返回0，否则返回-1。
*/

ce_int_t   
ipc_queue_create(ce_ipc_context_t *context,int queue_id,size_t queue_size,size_t rcd_size,int ipc_type)
{

	switch(ipc_type)
	{
		case IPC_SHM:
			return ipc_shm_queue_create(context,queue_id,queue_size,rcd_size);

		default:

			return CE_ERROR;
	}
}

/*
 * brief: 写进程调用此接口指明要写的队列，并获取写指针，对队列进行写操作，写进程在写具体的数据（记录）之前需调用此接口。
 *@ param: ipc,打开的ipc指针；
 *@ param: q_id 要写的队列标识符；
 *@ param: ipc_type  同上；
 *@ return : 若注册成功，则返回指向结构nta_data_writer_t 实例的指针，利用该指针对队列进行写操作；否则返回NULL。
 */

ce_data_writer_t* 
ipc_writer_register( ce_ipc_t *ipc,int q_id,int ipc_type)
{

	switch(ipc_type)
	{
		case IPC_SHM:
			return ipc_shm_writer_register(ipc,q_id);

		default:

			return NULL;
	}
}

/*
 * brief: 写进程在不需要对队列进行写或是退出时调用此接口来释放写进程占用的资源。
 *@ param: ipc,打开的ipc指针；
 *@ param: d_writer, 注册时返回的指向nta_data_writer_t 实例的指针；
 *@ param: ipc_type  同上；
 *@ return : void 。
 */

void 
ipc_writer_unregister(ce_ipc_t *ipc, ce_data_writer_t *d_writer,int ipc_type)
{

	switch(ipc_type)
	{
		case IPC_SHM:

			ipc_shm_writer_unregister(ipc,d_writer);

		default:
		break;

	}

}

/*
 * brief: 读进程调用此接口指明要读的队列，并获取读指针，对队列进行读操作，读进程在读具体的数据（记录）之前需调用此接口。
 *@ param: ipc,打开的ipc指针；
 *@ param: q_id 要读的队列标识符；
 *@ param: ipc_type  同上；
 *@ return : 若注册成功，则返回指向结构nta_data_reader_t 实例的指针，利用该指针对队列进行读操作；否则返回NULL。
 */
ce_data_reader_t* 
ipc_reader_register(ce_ipc_t *ipc, int q_id,int ipc_type,void *private_data, char *reader_name)
{

	switch(ipc_type)
	{
		case IPC_SHM:
			return ipc_shm_reader_register(ipc,q_id,private_data,reader_name);

		default:

			return NULL;
	}

}

/*
 * brief: 读进程在不需要对队列进行读或是退出时调用此接口来释放读进程占用的资源。
 *@ param: ipc,打开的ipc指针；
 *@ param: d_reader, 注册时返回的指向nta_data_reader_t 实例的指针；
 *@ param: ipc_type  同上；
 *@ return : void 。
 */
void 
ipc_reader_unregister(ce_ipc_t *ipc, ce_data_reader_t *d_reader,int ipc_type)
{

	switch(ipc_type)
	{
		case IPC_SHM:

			ipc_shm_reader_unregister(ipc,d_reader);

		default:
		break;

	}

}

/*
*  brief : 写进程需要写数据时需要不断的调用此接口将数据写入队列。在写的过程中，若队列里面有写忙的数据块，
   写进程将调用通告接口，通告对该队列进行读取的所有进程进行读取操作。若队列里没有空闲的数据块可供写数据，
   将通过通告接口发送数据，直到有空闲的数据块。
* @param: data_writer 册时返回的指向nta_data_writer_t 实例的指针；
* @param: data_record  需要写的数据记录指针。需要往共享内存队列里写的结构体必须要内嵌nta_data_record_t 结构体，并且要实现读写回调函数；
* @param: w_size  返回写大小；
* @param: ipc_type 同上。
* @ return : 若成功则返回0，否则返回-1。
*/
ce_int_t 
ipc_record_write(ce_data_writer_t* data_writer,ce_data_record_t *data_record,size_t *w_size,int ipc_type)
{

	switch(ipc_type)
	{
		case IPC_SHM:
			return ipc_shm_record_write(data_writer,data_record,w_size);

		default:

			return CE_ERROR;
	}
}

/*
*  brief : 读进程调用此接口，连接上可读数据块的通报接口，接收可读数据块，并对数据块内的数据进行读取，并调用读进程的记录处理回调函数
对读取的记录进行处理。

* @param: data_reader 册时返回的指向nta_data_reader_t 实例的指针；
* @param: data_record  一个未初始化的记录空间指针，用于接收队列中的数据记录；
* @param: read_handle 读进程处理被读取数据的回调函数。其声明如下：
typedef nta_int_t (*nta_ipc_read_handle_fn)(nta_data_reader_t *data_reader,nta_data_record_t *data_record,void *user_data);
* @ return : 若成功则返回0，否则返回-1。
*/

ce_int_t 
ipc_record_read(ce_data_reader_t *data_reader,ce_data_record_t *data_record,ce_ipc_read_handle_fn read_handle,int ipc_type)
{

	switch(ipc_type)
	{
		case IPC_SHM:
			return ipc_shm_record_read(data_reader,data_record,read_handle);

		default:

			return CE_ERROR;
	}
}

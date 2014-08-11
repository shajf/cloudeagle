/*
 * =====================================================================================
 *
 *       Filename:  ce_ipc_shm.c
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
#include "ce_ipc_shm.h"
#include "ce_shm.h"
#include "ce_compiler.h"
#include "ce_atomic.h"
#include <sys/sysctl.h>
#include <zmq.h>
#include "ce_list.h"
#include "ce_log.h"
#include "ce_thread_mutex.h"
#include "ce_ipc_dbk_queue.h"

typedef struct ce_ipc_shm_t ce_ipc_shm_t;
typedef struct ce_ipc_shm_header_t ce_ipc_shm_header_t;
typedef struct ce_ipc_shm_queue_t ce_ipc_shm_queue_t;
typedef struct ce_ipc_shm_reader_info_t ce_ipc_shm_reader_info_t; //reader_pid,free,last_dbk_index
typedef struct ce_ipc_writer_t ce_ipc_writer_t;
typedef struct ce_ipc_reader_t ce_ipc_reader_t;
typedef struct ce_ipc_data_block_t ce_ipc_data_block_t;
typedef struct ce_ipc_publisher_content_t ce_ipc_publisher_content_t;
typedef struct ce_ipc_dbk_iter_t  ce_ipc_dbk_iter_t;

#define RCD_START_MARK	0xfd
#define RCD_END_MARK	0xfe

#define QUEUE_FREE	0
#define QUEUE_BUSY	1
#define PUB_DBK		1
#define PUB_CONTENT	2
#define PUB_READER_AWAKE 3

/*DBK_WRITE_TIME_OUT(5m):dbk write timeout,when a dbk not to be
 * write ,ant its time is over DBK_WRITE_TIME_OUT,will be notify
 * all readers to read it.*/
#define DBK_WRITE_TIME_OUT 	5

#define SLEEP do{struct timespec t;t.tv_sec = 0;t.tv_nsec = 1000;nanosleep(&t,NULL);}while(0)

struct ce_ipc_shm_t 
{
    ce_ipc_t ipc;
#define ipc_shm_pool 	 ipc.pool            ///ipc内存池,reader需要销毁
#define ipc_shm_key_data ipc.key.data		///ipc key
#define ipc_shm_key_len  ipc.key.len
#define ipc_shm_id	 ipc.id
#define ipc_shm_type     ipc.ipc_type
#define ipc_shm_thread   ipc.ipc_thread
    ce_shm_t shm;	
#define ipc_shm_addr 		shm.start_addr		///共享内存首址
#define ipc_shm_size		shm.size			///共享内存大小
#define ipc_shm_name_data	shm.name.data		///共享内存名称
#define ipc_shm_name_len	shm.name.len
#define ipc_shm_proj_id		shm.proj_id			///共享内存proj
};

struct ce_ipc_writer_t
{
    ce_data_writer_t data_writer;  		 ///写数据的结构
#define writer_buf data_writer.buf
#define writer_ops data_writer.ops
#define writer_user_data data_writer.user_data
    ce_pool_t *pool;       				///写者内存池
    void *pub_sock;        					///写者公告数据的socket
}; 

#define ce_ipc_writer_get(writer)  container_of((writer),ce_ipc_writer_t,data_writer)


struct ce_ipc_reader_t
{
    ce_data_reader_t data_reader;
#define reader_buf data_reader.buf      	    ///读者数据缓冲区
#define reader_ops data_reader.ops				///读者操作
#define reader_user_data data_reader.user_data  ///读者数据,为流队列
    ce_pool_t *pool;								///读者内存池
    ce_thread_t *read_thread;						/// 读者线程
    ce_bool_t   stop_read;                         ///停止读标志
    int q_id;
    void *sub_context;								///读者的context
    void *sub_sock;									//读者建立的sub socket
    void *private_data;								///保留字段
}; 


#define ce_ipc_reader_get(reader)  container_of((reader),ce_ipc_reader_t,data_reader)

#pragma pack(push,1)

struct  ce_ipc_shm_header_t
{
    int32_t shm_magic;
#define CE_IPC_SHM_MAGIC 0Xfff
    size_t shm_size;            ///共享内存大小
    size_t shm_queue_num;       ///共享内存队列个数 
    size_t shm_queue_header;   ///共享内存第一个队列
    size_t shm_free_pos;       ///共享内存的空闲空间偏移量
    int dbk_pub_port;         ///发送给读者消息的port
};//共享内存头部信息

struct ce_ipc_shm_reader_info_t
{
#define READER_NAME_LEN 30
    pid_t reader_pid;			///读者进程PID
    char  reader_name[READER_NAME_LEN];      ///读者的名字
    int free;					///这个read_info结构是否使用

    uint64_t  rcv_cnt; //读者读的个数
};

struct ce_ipc_shm_queue_t
{
    size_t q_offset;                  ///本队列的偏移
    size_t q_len;                     ///本队列大小
    size_t q_next_offset;            //下一个队列偏移

    ce_atomic_t  cur_w_dbk;               ///现在正在写的dbk偏移
    size_t  dbk_header;               ///写完的dbk 的头部
    size_t   free_dbk_header;         ///空闲的dbk头部

    ce_atomic_t dbk_queue_mark;      ///操作队列dbk的锁
    ce_atomic_t dbk_write_mark;      ///写队列的锁

    size_t  rcd_max_size; 			///最大记录大小
    int   q_id;						 ///队列ID
    int  collect_count;

    uint64_t send_cnt; //发布个数
    uint64_t send_failed_cnt; //发布失败个数
    uint64_t get_blk_failed_cnt; //获取blk失败的个数
    uint64_t re_reg_clear_cnt; //读者重新注册时清除个数

    ce_atomic_t   reader_info_mark;   ///修改读者信息的锁
    ce_atomic_t   ref_count;         ///这个队列的读者个数
    ce_ipc_shm_reader_info_t readers[CE_QUEUE_MAX_READER_NUM];
};//共享内存队列头部信息

struct ce_ipc_data_block_t
{
    size_t dbk_offset; 			/// 数据块在队列中的偏移
    size_t dbk_size;  			///数据块大小

    size_t dbk_next_offset;     ///下一个数据块偏移
    size_t  rcd_num;			///本数据块中的记录总数
    size_t  dbk_pos; 			///本数据块的目前写指针记录位置

    time_t last_read_time; 		///
    time_t first_write_time;		///本数据块的第一次写事件
    int    dbk_state;				///本数据块的状态

    ce_atomic_t  read_count;   ///数据块还要被几个读者读
    pid_t        reader_pid[CE_QUEUE_MAX_READER_NUM];
    ce_atomic_t dbk_write_mark;      ///写数据块的锁,如果一个写者正在写,其他读者或写者都不能写

#define DBK_WRITE 0
#define DBK_READING  1
#define DBK_READED   2
#define DBK_FREE     3
#define DBK_BUSY     4
};//共享内存数据块

#pragma pack(pop)

struct ce_ipc_dbk_iter_t 
{
    ce_ipc_data_block_t *dbk;          ///指向正在遍历的数据块
    ce_data_reader_t *d_reader;         ///读者结构
    void *user_data;                     ///保留数据
    char  *cur_addr;                      ///当前地址
    uint32_t cur_size;                  ///当前记录大小
    size_t cur_rcd_num;                ///已经读取的记录总数
    int (*has_next)(ce_ipc_dbk_iter_t* iter); ///判断是否有下一条记录的回调函数
    ce_data_reader_t* (*next)(ce_ipc_dbk_iter_t *iter); ///得到下一条记录的回调函数

};

typedef struct 
{
    void *addr;
    size_t len;
}ce_ipc_shm_data_area_t;

#define ce_ipc_shm_get(ipc_ptr) container_of((ipc_ptr),ce_ipc_shm_t,ipc)

#define CE_IPC_LOCK(mark) while(ce_atomic_cas(mark,1,0)!=0)

#define CE_IPC_UNLOCK(mark) ce_atomic_set(mark,0)




static beat_funcs_t g_ipc_beat_funcs =
{
    NULL,
    NULL,
    NULL,
};

void ipc_beat_funcs_set(beat_funcs_t *funcs)
{
    g_ipc_beat_funcs.beat_register = funcs->beat_register;
    g_ipc_beat_funcs.beat = funcs->beat;
    g_ipc_beat_funcs.beat_unregister = funcs->beat_unregister;
}

static
ce_bool_t _get_proc_exist(pid_t pid)
{
	char proc_file[16] = { 0 };

	snprintf(proc_file, sizeof(proc_file), "/proc/%d", pid);

	if (0 == access(proc_file, 0))
	{
		return CE_TRUE;
	}

	return CE_FALSE;
}

static void set_sys_shm_limit(size_t *shm_size)
{
    int key[2]={CTL_KERN,KERN_SHMMAX};
    size_t  shm_max;
    size_t val_len;

    int rc=sysctl(key,sizeof(key)/sizeof(key[0]),&shm_max,&val_len,0,0);

    if(rc==0)
    {

        if(shm_max<*shm_size)
        {
            /*try change shm_max*/ 	
            rc=sysctl(key,
                    sizeof(key)/sizeof(int),
                    &shm_max,
                    &val_len,
                    shm_size,
                    sizeof(size_t));

            if(rc) /*change shm_max failed*/
            {
                *shm_size=shm_max;
            }
        }
    }

}

    static inline void * 
ipc_shm_base_addr_get(ce_ipc_shm_t *ipc_shm)
{
    return (void*)(ipc_shm->ipc_shm_addr);
}

    static inline void*
ipc_shm_payload_base_addr_get(ce_ipc_shm_t *ipc_shm)
{
    void * shm_base=ipc_shm_base_addr_get(ipc_shm);
    return ((char *)shm_base+sizeof(ce_ipc_shm_header_t));
}

    static inline void*
ipc_shm_addr_by_offset(ce_ipc_shm_t *ipc_shm,size_t offset)
{
    void *base_shm_addr=ipc_shm_base_addr_get(ipc_shm);
    return (void*)((char *)base_shm_addr+offset);
}

    static inline size_t
ipc_shm_offset_by_shm_addr(ce_ipc_shm_t *ipc_shm,void *addr)
{
    void *base_shm_addr=ipc_shm_base_addr_get(ipc_shm);
    return (size_t)((char *)addr-(char *)base_shm_addr);
}
//得到共享内存头部地址
    static inline ce_ipc_shm_header_t *
ipc_shm_header_get(ce_ipc_shm_t *ipc_shm)
{
    return (ce_ipc_shm_header_t*)(ipc_shm_base_addr_get(ipc_shm));
}
//根据一个队列得到下一个队列地址
    static inline ce_ipc_shm_queue_t*
ipc_shm_queue_next(ce_ipc_shm_t *ipc_shm,ce_ipc_shm_queue_t *shm_queue)
{
    if(shm_queue->q_next_offset==0)
    {
        return NULL;
    }

    return (ce_ipc_shm_queue_t*)ipc_shm_addr_by_offset(ipc_shm,shm_queue->q_next_offset);
}
//根据共享内存地址和队列头部偏移得到第一个队列地址
    static  ce_ipc_shm_queue_t *
ipc_shm_queue_first(ce_ipc_shm_t *ipc_shm)
{

    ce_ipc_shm_header_t *shm_h=ipc_shm_header_get(ipc_shm);

    if(shm_h->shm_queue_num==0)
    {
        return NULL;
    }


    assert(shm_h->shm_queue_header>0);

    return (ce_ipc_shm_queue_t*)(ipc_shm_addr_by_offset(ipc_shm,shm_h->shm_queue_header));
}

//取得共享内存队列基址
	static inline void *
ipc_shm_queue_base_addr_get(ce_ipc_shm_queue_t *shm_queue)
{
	return (void*)(shm_queue);
}
//根据偏移量取得共享内存队列基址
	static inline void*
ipc_shm_queue_addr_by_offset(ce_ipc_shm_queue_t *shm_queue,size_t offset)
{
	void *base=ipc_shm_queue_base_addr_get(shm_queue);
	return (void*)((char *)base+offset);
}

#define list_for_ipc_shm_queue(shm_queue,ipc_shm) \
    for(shm_queue=ipc_shm_queue_first(ipc_shm);shm_queue!=NULL;shm_queue=ipc_shm_queue_next(ipc_shm,shm_queue)) 


#define list_for_ipc_data_block(dbk,header,shm_q) \
    for(dbk=ipc_queue_dbk_first(shm_q,header);dbk!=NULL;dbk=ipc_queue_dbk_next(shm_q,dbk))

//从共享内存队列取得第一个dbk
    static inline ce_ipc_data_block_t *
ipc_queue_dbk_first(ce_ipc_shm_queue_t *shm_q,size_t header)
{
    if(header==0)
    {
        return NULL;
    }
    return (ce_ipc_data_block_t*)(ipc_shm_queue_addr_by_offset(shm_q,header));
}

    static inline ce_ipc_data_block_t *
ipc_queue_dbk_next(ce_ipc_shm_queue_t *shm_q,ce_ipc_data_block_t *dbk)
{
    if(dbk->dbk_next_offset==0)
    {
        return NULL;
    }

    return (ce_ipc_data_block_t*)(ipc_shm_queue_addr_by_offset(shm_q,dbk->dbk_next_offset));
}


//从pool中分配size大小的内存，并且设置data_writer的buf 地址和ops函数组
    static void 
ipc_make_data_writer(ce_pool_t *pool,ce_data_writer_t *data_writer,size_t size)
{

    char *addr = (char *)ce_palloc(pool,size);
    assert(addr);

    ce_data_wrapbuf_t *buf = &data_writer->buf;

    CE_DATA_SET_BUF_START(buf,addr);
    CE_DATA_SET_BUF_POS(buf,addr);
    CE_DATA_SET_BUF_END(buf,addr+size);
    data_writer->ops = &ce_writer_encoding;	
    buf->expand=NULL;
}
    static inline int
ipc_queue_dbk_full(ce_ipc_data_block_t *dbk,size_t size)
{
    return(dbk->dbk_pos+size>dbk->dbk_size);
}
/*
 *@brief:       写进程的工作，通过ipc_context结构检查 ipc shm的每一个消息队列的正在写的datablock是否老化
 *@param ipc_context:  ipc_context
 */
    static void 
check_and_pub_timeout_dbk(ce_ipc_context_t *ipc_context)
{

    ce_ipc_shm_t *ipc_shm = ce_ipc_shm_get(ipc_context->ipc);
    ce_ipc_shm_queue_t *shm_q;
    ce_ipc_data_block_t *cur_write_dbk;
    ce_data_writer_t d_writer ; 

    int cur_write_dbk_offset;
    int queue_id = 0;
    uint8_t i = 0;
    time_t cur_times =0;
    list_for_ipc_shm_queue(shm_q,ipc_shm)
    {
        cur_write_dbk_offset = ce_atomic_read(&shm_q->cur_w_dbk);
        if(cur_write_dbk_offset == 0)
        {
        	continue;
        }

        cur_write_dbk = (ce_ipc_data_block_t*)ipc_shm_queue_addr_by_offset(shm_q,
                cur_write_dbk_offset);
        ///避免被同时发布,已经满了的数据块只能被正常发布
        if(ipc_queue_dbk_full(cur_write_dbk, shm_q ->rcd_max_size))
        {
            continue;
        }

        cur_times = time((time_t*)NULL);

        /*if write timeout will notify all readers to read*/
        if((cur_times-cur_write_dbk->first_write_time) >= DBK_WRITE_TIME_OUT)
        {
        	if (ce_atomic_cas(&shm_q->cur_w_dbk, 0, cur_write_dbk_offset) != cur_write_dbk_offset)
			{
				ce_log(CE_LOG_INFO,"shm_q %d's cur write dlb already publish.", shm_q ->q_id);
			}
        	else
        	{
        		ce_log(CE_LOG_DEBUG,"queue %d publish a timeout dbk offset %d\n", shm_q ->q_id, cur_write_dbk_offset);

				ce_reset_pool(ipc_context->tmp_pool);

				ipc_make_data_writer(ipc_context->tmp_pool,&d_writer,1+4+4);

				/*write pub type*/
				ce_write_uint8(&d_writer,PUB_DBK);
				/*write q_id*/
				ce_write_int(&d_writer,shm_q->q_id);
				/*write dbk offset*/
				ce_write_uint32(&d_writer,cur_write_dbk_offset);
				cur_write_dbk ->dbk_state = DBK_READING;
				ce_atomic_set(&cur_write_dbk ->read_count,shm_q->ref_count);
				CE_IPC_LOCK(&shm_q ->reader_info_mark);
				for (i = 0; i < ce_atomic_read(&shm_q ->ref_count); i++)
				{
					cur_write_dbk ->reader_pid[i] = shm_q ->readers[i].reader_pid;
				}
				CE_IPC_UNLOCK(&shm_q ->reader_info_mark);

				if (-1 == zmq_send(ipc_context->zmq_pub_sock,CE_DATA_GET_BUF_START(&d_writer.buf),1+4+4,0))
				{
					shm_q->send_failed_cnt++;
					ce_log(CE_LOG_ERR,"zmq_send error no %d.", errno);
				}
				else
				{
					shm_q->send_cnt++;
				}
        	}//else
        }//if
    }//list
}

static int stop_timeout_write_thread_fn(ce_thread_t *thread,void *user_data)
{	
    //ce_log(CE_LOG_INFO,"entry ipc write thread exit func.");

    return CE_OK;
}

/*
 *@brief:  进程退出时发布还未写完的数据块
 *
 */
void exit_pub_dbk(ce_ipc_context_t *ipc_context)
{

    ce_ipc_shm_t *ipc_shm = ce_ipc_shm_get(ipc_context->ipc);
    ce_ipc_shm_queue_t *shm_q;
    ce_data_writer_t d_writer ; 

    int cur_write_dbk_offset;
#ifdef __DEBUG__
    int queue_id = 0;
#endif
    uint8_t i = 0;
    list_for_ipc_shm_queue(shm_q,ipc_shm)
    {
        cur_write_dbk_offset = shm_q->cur_w_dbk;
        ce_ipc_data_block_t  *cur_write_dbk = (ce_ipc_data_block_t*)ipc_shm_queue_addr_by_offset(shm_q,
                cur_write_dbk_offset);

        if(cur_write_dbk_offset==0)
            continue;
#ifdef __DEBUG__
        queue_id = shm_q ->q_id;	
#endif
        /*if write timeout will notify all readers to read*/
#ifdef __DEBUG__
        DEBUG_LOG("queue %d publish a dbk offset %d\n", queue_id, cur_write_dbk_offset);		
#endif
        if (ce_atomic_cas(&shm_q->cur_w_dbk, 0, cur_write_dbk_offset) != cur_write_dbk_offset)
		{
			ce_log(CE_LOG_INFO,"shm_q %d's cur write dlb already publish.", shm_q ->q_id);
		}
        else
        {
			ce_reset_pool(ipc_context->tmp_pool);

			ipc_make_data_writer(ipc_context->tmp_pool,&d_writer,1+4+4);

			/*write pub type*/
			ce_write_uint8(&d_writer,PUB_DBK);
			/*write q_id*/
			ce_write_int(&d_writer,shm_q->q_id);
			/*write dbk offset*/
			ce_write_uint32(&d_writer,cur_write_dbk_offset);

			cur_write_dbk ->dbk_state = DBK_READING;
			ce_atomic_set(&cur_write_dbk->read_count,shm_q->ref_count);
			CE_IPC_LOCK(&shm_q ->reader_info_mark);
			for (i = 0; i < ce_atomic_read(&shm_q ->ref_count); i++)
			{
				cur_write_dbk ->reader_pid[i] = shm_q ->readers[i].reader_pid;
			}
			CE_IPC_UNLOCK(&shm_q ->reader_info_mark);

			if (-1 == zmq_send(ipc_context->zmq_pub_sock,CE_DATA_GET_BUF_START(&d_writer.buf),1+4+4,0))
			{
				shm_q->send_failed_cnt++;
				ce_log(CE_LOG_ERR,"zmq_send error no %d.", errno);
			}
			else
			{
				shm_q->send_cnt++;
			}
        }//else
    }//list
}


static void ipc_wr_thrd_reg_cleanup(void * arg)
{
    ce_ipc_context_t *ipc_context = (ce_ipc_context_t*)arg;

    ce_log(CE_LOG_INFO,"ipc thread cleanup.");

    if (NULL != g_ipc_beat_funcs.beat_unregister)
    {
        g_ipc_beat_funcs.beat_unregister(pthread_self());
    }
    /*强制设置为TRUE*/
    ipc_context ->is_thread_stop = CE_TRUE;
}

/* 
 * @brief:          当正在写的数据块30s都没有写完，强行向读者通报，可以读取.常驻线程，10s工作一次
 * @param thread:   线程结构体
 * @param user_data:ipc_context
 */
static void* timeout_write_thread_fn(ce_thread_t *thread,void *user_data)
{	
    ce_ipc_context_t *ipc_context = (ce_ipc_context_t*)user_data;

    char thd_name[32] = {0};
    pthread_t tid = pthread_self();

    snprintf(thd_name, 32, "ipc-wr-thrd");
    if (NULL != g_ipc_beat_funcs.beat_register)
    {
        g_ipc_beat_funcs.beat_register(tid, thd_name, 15); /*15秒超时*/
    }

    /// 注册线程退出清理函数
    pthread_cleanup_push(ipc_wr_thrd_reg_cleanup, (void*)ipc_context);

    for(; ipc_context ->is_thread_stop == CE_FALSE;)
    {
        if (NULL != g_ipc_beat_funcs.beat)
        {
            g_ipc_beat_funcs.beat(tid);
        }

        check_and_pub_timeout_dbk(ipc_context);

        sleep(5);

        //        CE_CHECK_THEAD_EXIT(thread);
        if(thread->thread_exit_stat==THD_EXIT)
        {
            if(thread->exit_fun)
                thread->exit_fun(thread,thread->data);

            thread->thread_exit_stat=THD_EXITED;

            break;
        }
    }

    /*以return方式返回时，需要手动pop*/
    pthread_cleanup_pop(1);
    return NULL;
}


/*
 *@breif:                初始化共享内存头部
 *@param ipc_context:    IPC上下文
 *@param ipc_shm:        ipc_shm结构
 *@param dbk_rcv_port:   接收写者的通告消息端口
 *@param dbk_pub_port:   给读者发送通告消息的端口
 */
static void ipc_shm_header_init(ce_ipc_shm_t *ipc_shm,int dbk_pub_port)
{

    ce_ipc_shm_header_t *ipc_shm_header;
    void *shm_free_addr;
    ipc_shm_header = (ce_ipc_shm_header_t*)(ipc_shm->ipc_shm_addr);
    ipc_shm_header->shm_magic = CE_IPC_SHM_MAGIC;
    ipc_shm_header->shm_size = ipc_shm->ipc_shm_size;
    ipc_shm_header->shm_queue_num = 0;
    ipc_shm_header->shm_queue_header = 0;

    shm_free_addr = ipc_shm_payload_base_addr_get(ipc_shm);

    ipc_shm_header->shm_free_pos = ipc_shm_offset_by_shm_addr(ipc_shm,shm_free_addr);

    ipc_shm_header->dbk_pub_port = dbk_pub_port;


}


/*
 *@brief: 初始化ipc context 
 */
static void init_ipc_context(ce_ipc_context_t *ipc_context,ce_ipc_shm_t *ipc_shm, int dbk_pub_port)
{
    ce_ipc_t *ipc=&ipc_shm->ipc;
    char bind_port[64]={0};
    ipc_shm->ipc_shm_pool = ipc_context->pool;
    ipc_shm->ipc_shm_key_data = ipc_shm->ipc_shm_name_data;
    ipc_shm->ipc_shm_key_len = ipc_shm->ipc_shm_name_len;
    ipc_shm->ipc_shm_type = IPC_SHM;
    ipc_context->ipc = ipc;

    /*create zmq context to publisher or rcv dbk info.*/
    assert((ipc_context->zmq_context = zmq_init(1)));

    /*create a zeromq socket to publisher dbk to all readers to read*/
    assert((ipc_context->zmq_pub_sock = zmq_socket(ipc_context->zmq_context,ZMQ_PUB)));

    /*bind zeromq socket into specified port*/
    sprintf(bind_port,"tcp://*:%d",dbk_pub_port);

    if(zmq_bind(ipc_context->zmq_pub_sock,bind_port) != 0)
    {
    	ERR_LOG("zmq_bind failed, error %d.", errno);
    	assert(0);

    }

    ipc->pub_sock = ipc_context ->zmq_pub_sock;	

    ipc_context ->is_thread_stop = CE_FALSE;
    ce_thread_create(&ipc_context->timeout_write_thread,NULL,timeout_write_thread_fn,stop_timeout_write_thread_fn,(void*)ipc_context,ipc_context->pool);
    assert(ipc_context->timeout_write_thread);
}
/*
 *@brief:                释放ipc 环境,包含ipc 环境线程,发布sock,发布环境,内存池
 *@param ipc_context:    ipc环境
 */
int destory_ipc_context(ce_ipc_context_t *ipc_context)
{
    CE_WAIT_THREAD_EXIT(ipc_context->timeout_write_thread);
    exit_pub_dbk(ipc_context);
    ce_ipc_shm_t *ipc_shm = ce_ipc_shm_get(ipc_context->ipc);
    ce_shm_free(&ipc_shm ->shm);
    zmq_close(ipc_context ->zmq_pub_sock);
    zmq_term(ipc_context->zmq_context);
    ce_destroy_pool(ipc_context ->tmp_pool);
    ce_destroy_pool(ipc_context ->pool);
    return CE_OK;
}
/*
 *@breif:   从context内存池中分配ipc_shm结构，根据key和id创建共享内存，并填充ipc_shm结构
 *param context: ipc上下文
 *param key:     共享内存key
 *param id:      共享内存projectid
 *param shm_size:      共享内存大小
 *param dbk_rcv_port:  数据块接收端口
 *param dbk_pub_port:  数据块发布端口
 */
ce_ipc_t *ipc_shm_create(ce_ipc_context_t *context,const char *key,int id,size_t *shm_size, int dbk_pub_port)
{

    ce_ipc_shm_t *ipc_shm;

    size_t r_shm_size;

    assert(context&&key);

    r_shm_size = *shm_size;

    if(*shm_size <= sizeof(ce_ipc_shm_header_t))
    {
        r_shm_size =SHM_SIZE_DEFAULT;			
    }

    /*try to change the max shared memory size can be alloced.*/	
    set_sys_shm_limit(&r_shm_size);

    assert(r_shm_size>sizeof(ce_ipc_shm_header_t));

    *shm_size=r_shm_size;

    ipc_shm = (ce_ipc_shm_t*)ce_palloc(context->pool,
            sizeof(ce_ipc_shm_t));

    assert(ipc_shm);
    ipc_shm->ipc_shm_addr=NULL;
    ipc_shm->ipc_shm_size=r_shm_size;
    ipc_shm->ipc_shm_name_data=(u_char*)ce_pstrdup(context->pool,key);
    ipc_shm->ipc_shm_name_len=ce_strlen(key);
    ipc_shm->ipc_shm_proj_id=id;
    ipc_shm->ipc_shm_id = id;	
    /*create a new shared memory,and init the shm header, if exist attach to it*/
    if(ce_shm_alloc(&ipc_shm->shm)!=CE_OK)
    {
        ce_shm_get(&ipc_shm->shm);
        assert(ipc_shm->ipc_shm_addr);
#if 1
        INFO_LOG("attach shm");
        //       ipc_shm_debug(&ipc_shm ->ipc);
#endif
    }
    else
    {
        assert(ipc_shm->ipc_shm_addr);
        /*init shared memory area header.*/	
        ipc_shm_header_init(ipc_shm,dbk_pub_port);
#if 1
        INFO_LOG("new shm");
        //        ipc_shm_debug(&ipc_shm->ipc);
#endif
    }
    /*init ipc context*/
    init_ipc_context(context, ipc_shm, dbk_pub_port);
    return(&(ipc_shm->ipc));
}

//the reader to attach the share memory
ce_ipc_t * ipc_shm_open(ce_ipc_context_t *context)
{
    ce_pool_t *pool;
    ce_ipc_t *main_ipc = context->ipc;
    ce_ipc_shm_t *main_ipc_shm;
    ce_ipc_shm_t *ipc_shm;

    assert(main_ipc);
    /*if the same process as the creator of shm,
     * then return main ipc which created in 
     * init process*/
    if(context->creator_pid == getpid())
    {
        return main_ipc;
    }
    main_ipc_shm = ce_ipc_shm_get(main_ipc);

    pool = ce_create_pool(512);

    if(!pool)
        return NULL;

    ipc_shm = (ce_ipc_shm_t*)ce_palloc(pool,sizeof(ce_ipc_shm_t));
    assert(ipc_shm);

    ipc_shm->ipc_shm_addr=NULL;
    ipc_shm->ipc_shm_size=main_ipc_shm->ipc_shm_size;
    ipc_shm->ipc_shm_name_data=(u_char*)ce_pstrdup(pool,(char*)main_ipc_shm->ipc_shm_name_data);
    ipc_shm->ipc_shm_name_len=main_ipc_shm->ipc_shm_name_len;
    ipc_shm->ipc_shm_proj_id=main_ipc_shm->ipc_shm_proj_id;

    /*attach to  shared memory*/
    if(ce_shm_get(&ipc_shm->shm)==CE_ERROR)
    {
        ce_destroy_pool(pool);
        return NULL;	
    }
    ipc_shm->ipc_shm_pool = pool;

    return (&ipc_shm->ipc);
}

/*
 *@breif: 读者打开IPC shm
 *@param: SHM key, id
 *@return: 返回IPC结构
 */
ce_ipc_t * ipc_shm_open2(const char *key,int id)
{
    assert(key&&id>=0);

    ce_pool_t *pool;
    ce_ipc_shm_t *ipc_shm;

    /*创建内存池*/
    pool = ce_create_pool(512);

    if(!pool)
        return NULL;

    ipc_shm = (ce_ipc_shm_t*)ce_palloc(pool,sizeof(ce_ipc_shm_t));
    assert(ipc_shm);

    ipc_shm->ipc_shm_addr=NULL;
    ipc_shm->ipc_shm_name_data=(u_char*)ce_pstrdup(pool,key);
    ipc_shm->ipc_shm_name_len=ce_strlen(key);
    ipc_shm->ipc_shm_proj_id=id;

    /*attach to  shared memory*/
    if(ce_shm_get(&ipc_shm->shm)==CE_ERROR)
    {
        ce_destroy_pool(pool);
        return NULL;	
    }

    ipc_shm->ipc_shm_pool = pool;
    return (&ipc_shm->ipc);
}

/*
 *@breif: 读者关闭IPC shm
 *@param ipc: ipc环境
 *@return: OK or ERROR
 */
ce_int_t ipc_shm_close2(ce_ipc_t *ipc)
{

    ce_ipc_shm_t *ipc_shm = ce_ipc_shm_get(ipc);
    ce_shm_free(&ipc_shm ->shm);
    ce_destroy_pool(ipc_shm ->ipc_shm_pool);

    return CE_OK;
}
//get the shm_queue accroding to queue id
    static inline ce_ipc_shm_queue_t *
ipc_shm_queue_get(ce_ipc_shm_t *ipc_shm,int q_id)
{
    ce_ipc_shm_queue_t *shm_q=NULL;

    list_for_ipc_shm_queue(shm_q,ipc_shm)
    {
        if(shm_q->q_id==q_id)
        {
            return shm_q;
        }

    }

    return NULL;
}
//get the shm remain size
    static inline size_t 
ipc_shm_remain_size(ce_ipc_shm_t *ipc_shm)
{
    ce_ipc_shm_header_t *shm_h=ipc_shm_header_get(ipc_shm);
    return (shm_h->shm_size-shm_h->shm_free_pos);
}

//从共享内存分配size大小的空间
    static inline void*
ipc_shm_alloc(ce_ipc_shm_t *ipc_shm,size_t size)
{
    ce_ipc_shm_header_t *shm_h = ipc_shm_header_get(ipc_shm);
    void *res = ipc_shm_addr_by_offset(ipc_shm,shm_h->shm_free_pos);
    shm_h->shm_free_pos+=size;
    return res;
}
//共享内存队列通过链表方式连接到到共享内存队列链
    static inline void 
ipc_shm_queue_add(ce_ipc_shm_t *ipc_shm,ce_ipc_shm_queue_t *shm_q)
{
    ce_ipc_shm_header_t *ipc_shm_h = ipc_shm_header_get(ipc_shm);

    shm_q->q_next_offset = ipc_shm_h->shm_queue_header;

    ipc_shm_h->shm_queue_header = shm_q->q_offset;
    ipc_shm_h->shm_queue_num+=1;
}


/*根据name查找*/
	static ce_ipc_shm_reader_info_t *
ipc_shm_queue_reader_find_by_name(ce_ipc_shm_queue_t *shm_q, char *reader_name)
{
   ce_ipc_shm_reader_info_t *r_info;
   uint8_t i = 0;

   for(i=0; i<shm_q ->ref_count;i++)
   {
	   r_info=&(shm_q->readers[i]);

	   if (0 == strncmp(r_info->reader_name, reader_name, READER_NAME_LEN))
	   {
		   return r_info;
	   }
   }

   return NULL;
}

//根据Pid查找reader info
	static ce_ipc_shm_reader_info_t *
ipc_shm_queue_reader_find(ce_ipc_shm_queue_t *shm_q,pid_t r_pid, size_t *r_indx)
{
	ce_ipc_shm_reader_info_t *r_info;
	uint8_t i = 0;
	for(i=0; i<shm_q ->ref_count;i++)
	{
		r_info=&(shm_q->readers[i]);

		if(r_info->reader_pid == r_pid)
		{
			*r_indx = i;
			return r_info;
		}
	}

	return NULL;
}

//队列头部读者初始化
    static void 
ipc_shm_queue_reader_info_init(ce_ipc_shm_reader_info_t *r_info,
        pid_t reader_pid,
        int is_free, char *reader_name)
{

    r_info->reader_pid = reader_pid;
    r_info->free = is_free;
    r_info->rcv_cnt = 0;
    if (reader_name)
    {
        if (strlen(reader_name) < READER_NAME_LEN)
        {
            strncpy(r_info->reader_name, reader_name, strlen(reader_name));
        }
        else
        {
            strncpy(r_info->reader_name, reader_name, READER_NAME_LEN);
        }
    }
}

static void _ipc_clear_dbk_for_reader(ce_ipc_shm_queue_t *shm_q, ce_ipc_shm_reader_info_t *r_info)
{
	int reader_count = ce_atomic_read(&shm_q ->ref_count);
	ce_ipc_data_block_t *dbk = NULL;

//		CE_IPC_LOCK(&shm_q ->reader_info_mark);

	INFO_LOG( "clear dbk read info for reader %s.", r_info->reader_name);

	list_for_ipc_data_block(dbk, shm_q ->dbk_header, shm_q)
	{
		uint8_t i;

		for (i = 0; i < reader_count; i++)
		{
			if (0 == dbk ->reader_pid[i])
			{
				continue;
			}

			if (dbk ->reader_pid[i] == r_info->reader_pid)
			{
				/*清除dbk计数*/
				dbk->reader_pid[i] = 0;

				ce_atomic_dec(&dbk->read_count);
				if(ce_atomic_read(&dbk->read_count) == 0)
				{
					dbk->dbk_state =DBK_READED;
					shm_q->re_reg_clear_cnt++;
				}
				break;
			}
		}
	}
//		CE_IPC_UNLOCK(&shm_q ->reader_info_mark);
}

static void ipc_shm_queue_reader_info_clean(ce_ipc_shm_queue_t *shm_q, pid_t reader_pid)
{
	size_t indx;
	ce_ipc_shm_reader_info_t *read_info = NULL;
	unsigned int reader_count = 0;

	read_info = ipc_shm_queue_reader_find(shm_q, reader_pid, &indx);
	if (NULL == read_info)
	{
		CRIT_LOG("queue_id = %d read info is NULL.", shm_q ->q_id);
		return;
	}
	else
	{
		INFO_LOG("queue_id = %d ipc read thread unregister, pid %d.", shm_q ->q_id, reader_pid);

		/*如果有此读者之前的数据，清除掉*/
		_ipc_clear_dbk_for_reader(shm_q, read_info);

		CE_IPC_LOCK(&shm_q->reader_info_mark);

		reader_count = ce_atomic_read(&shm_q ->ref_count);

		 /*dec the ref number of queue*/
		if (reader_count == 0)
		{
			CRIT_LOG("shm_q ->ref_count already zero. and can't desc");
		}
		else
		{
			ce_atomic_dec(&shm_q->ref_count);

			///正好是最后一个注册的读者,则将这个读者信息改为空闲
			if (indx == reader_count - 1)
			{
				read_info->free = 1;
				read_info->reader_pid = 0;
				bzero(read_info ->reader_name, READER_NAME_LEN);
			}
			///否则,要将读者信息前移,并将最后一个读者信息变为空闲
			else
			{
				memmove(&shm_q ->readers[indx], &shm_q ->readers[indx + 1], \
						(reader_count-indx -1)*sizeof(ce_ipc_shm_reader_info_t));
				shm_q ->readers[reader_count - 1].free = 1;
				shm_q ->readers[reader_count - 1].reader_pid = 0;
				bzero(shm_q ->readers[reader_count - 1].reader_name, READER_NAME_LEN);
			}
		}
		CE_IPC_UNLOCK(&shm_q->reader_info_mark);
	}
}


//根据地址得到共享内存队列偏移量
    static inline size_t
ipc_shm_queue_offset_by_addr(ce_ipc_shm_queue_t *shm_queue,void* addr)
{
    void *base=ipc_shm_queue_base_addr_get(shm_queue);
    return (size_t)((char *)addr-(char *)base);
}
//初始化共享内存队列的数据块
    static inline void 
ipc_shm_queue_data_block_init(ce_ipc_data_block_t *dbk,size_t dbk_offset,
        size_t dbk_size,size_t dbk_pos, int dbk_state,
        size_t read_count)
{
    dbk->dbk_offset = dbk_offset;
    dbk->dbk_size = dbk_size;

    dbk->dbk_next_offset = 0;	
    ce_atomic_set(&dbk->read_count,read_count);

    dbk->rcd_num = 0;	
    dbk->dbk_pos = dbk_pos;	

    dbk->dbk_state = dbk_state;
}

//数据块链增加一个数据块，这个操作要对dbk_queue_mark加原子锁
    static inline void 
ipc_queue_dbk_add(ce_ipc_shm_queue_t *shm_q,ce_ipc_data_block_t *dbk,size_t *header)
{

    CE_IPC_LOCK(&shm_q->dbk_queue_mark);

    dbk->dbk_next_offset = *header;
    *header = dbk->dbk_offset;
    CE_IPC_UNLOCK(&shm_q->dbk_queue_mark);
}
//给共享内存队列创建dbk_num个大小为dbk_size的dbk,并且这些dbk初始化，连接起来
    static void 
ipc_shm_queue_data_block_create(ce_ipc_shm_queue_t *shm_q,size_t r_queue_size,
        size_t dbk_size,size_t f_dbk_size,size_t dbk_num)
{
    ce_unused(r_queue_size);
    size_t dbk_h = sizeof(ce_ipc_data_block_t);
    size_t offset = sizeof(ce_ipc_shm_queue_t);
    size_t i;

    ce_ipc_data_block_t *dbk;
    for(i=0;i<dbk_num-1;i++)
    {
        dbk = (ce_ipc_data_block_t*)ipc_shm_queue_addr_by_offset(shm_q,offset);
        ipc_shm_queue_data_block_init(dbk,offset,dbk_size,dbk_h,DBK_FREE,0);
        ipc_queue_dbk_add(shm_q,dbk,&shm_q->free_dbk_header);
        offset  +=dbk_size;
    }

    dbk = (ce_ipc_data_block_t*)ipc_shm_queue_addr_by_offset(shm_q,offset);
    ipc_shm_queue_data_block_init(dbk,offset,f_dbk_size,dbk_h,DBK_FREE,0);
    ipc_queue_dbk_add(shm_q,dbk,&shm_q->free_dbk_header); 
}

//初始化共享内存队列，根据queue_id,r_queue_size,dbk_size,f_dbk_size,dbk_num,rcd_size,最后一个dbk_size可以和前面不一样
static void 
ipc_shm_queue_init(ce_ipc_shm_t *ipc_shm, ce_ipc_shm_queue_t *shm_q,int queue_id,\
        size_t r_queue_size,size_t dbk_size,size_t f_dbk_size,size_t dbk_num,size_t rcd_size)
{

    void *base_addr = ipc_shm_queue_base_addr_get(shm_q);

    shm_q->q_id = queue_id;

    shm_q->q_offset=ipc_shm_offset_by_shm_addr(ipc_shm,base_addr);
    shm_q->q_len=r_queue_size;
    shm_q->q_next_offset = 0;	
    shm_q->rcd_max_size = rcd_size;

    ce_atomic_set(&shm_q->cur_w_dbk, 0);

    shm_q->dbk_header = 0;	
    shm_q->free_dbk_header = 0;
    shm_q ->collect_count = 0;
    ce_atomic_set(&shm_q->ref_count,0);
    ce_atomic_set(&shm_q->reader_info_mark,0);
    ce_atomic_set(&shm_q->dbk_write_mark,0);
    ce_atomic_set(&shm_q->dbk_queue_mark,0);

    size_t i;
    ce_ipc_shm_reader_info_t *r_info;
    for(i=0;i<CE_QUEUE_MAX_READER_NUM;i++)
    {
        r_info=&(shm_q->readers[i]);
        ipc_shm_queue_reader_info_init(r_info,0,1,"");
    }

    ipc_shm_queue_data_block_create(shm_q,r_queue_size,dbk_size,f_dbk_size,dbk_num);
}

//根据rcd_size,r_queue_size计算dbk_size，f_dbk_size，dbk_num
    static void 
calculate_queue_args(size_t r_queue_size,size_t rcd_size,size_t *dbk_size,
        size_t *f_dbk_size,size_t *dbk_num)
{
    size_t payload_queue_size = r_queue_size-sizeof(ce_ipc_shm_queue_t);
    size_t dbk_rcd_num = CE_DBK_PUBLISHER_RCD_NUM*2;
    size_t s_dbk_size = dbk_rcd_num*(rcd_size+sizeof(ce_ipc_data_block_t));
    size_t s_dbk_num = payload_queue_size/s_dbk_size;

    while(dbk_rcd_num>CE_DBK_PUBLISHER_RCD_NUM&&s_dbk_num<CE_DBK_MIN_NUM)
    {
        dbk_rcd_num -=1;
        s_dbk_size = dbk_rcd_num*(rcd_size+sizeof(ce_ipc_data_block_t));
        s_dbk_num = payload_queue_size/s_dbk_size;
    }

    assert(s_dbk_num>CE_DBK_MIN_NUM&&dbk_rcd_num>=CE_DBK_PUBLISHER_RCD_NUM);
    *dbk_size = s_dbk_size;

    if(payload_queue_size%s_dbk_size==0)
    {
        *dbk_num = s_dbk_num;
        *f_dbk_size = s_dbk_size;
    }

    else
    {
        *dbk_num = s_dbk_num+1;
        *f_dbk_size=payload_queue_size-s_dbk_size*s_dbk_num;
        assert(*f_dbk_size>0);
    }
}
static void
ipc_shm_stat_init(ce_ipc_shm_queue_t *shm_q)
{
	int i=0;

	shm_q->send_cnt = 0;
	shm_q->send_failed_cnt = 0;
	shm_q->get_blk_failed_cnt = 0;
	shm_q->re_reg_clear_cnt = 0;

	for(i=0; i<CE_QUEUE_MAX_READER_NUM; i++)
	{
		shm_q->readers[i].rcv_cnt = 0;
	}
}

//写者创建共享内存队列，创建共享内存队列，如果放不下一个记录，则为4M
    ce_int_t   
ipc_shm_queue_create(ce_ipc_context_t *context,int queue_id,size_t queue_size,size_t rcd_size)
{
    ce_ipc_shm_t *ipc_shm;
    ce_ipc_shm_queue_t *shm_q;
    size_t r_queue_size,dbk_size=0,dbk_num=0,f_dbk_size=0;    

    /*the queue only can be created by process which creating
     * the shared memeory*/
    assert(rcd_size > 0  &&getpid()==context->creator_pid);
    ipc_shm = ce_ipc_shm_get(context->ipc);

    r_queue_size = queue_size;
    if(queue_size <= sizeof(ce_ipc_shm_queue_t)+rcd_size)
    {
        r_queue_size = SHM_QUEUE_SIZE_DEFAULT < sizeof(ce_ipc_shm_queue_t)+rcd_size ? \
                       sizeof(ce_ipc_shm_queue_t)+rcd_size:SHM_QUEUE_SIZE_DEFAULT;

    }
    r_queue_size = ce_align(r_queue_size,((sizeof(ce_ipc_data_block_t)+rcd_size)*CE_DBK_PUBLISHER_RCD_NUM));		

    /*if queue has existed,then return error and set the atomic variables*/
    if((shm_q = ipc_shm_queue_get(ipc_shm,queue_id)) != NULL)
    {
        INFO_LOG("queue already exist");
        //    	ipc_shm_debug(&ipc_shm ->ipc);
        ipc_shm_stat_init(shm_q);
//
//        /*重新初始化队列的blk*/
//        calculate_queue_args(r_queue_size,rcd_size,&dbk_size,&f_dbk_size,&dbk_num);
//
//        shm_q->dbk_header = 0;
//		shm_q->free_dbk_header = 0;
//		shm_q ->collect_count = 0;
//		ce_atomic_set(&shm_q->cur_w_dbk, 0);
//		ce_atomic_set(&shm_q->dbk_write_mark,0);
//		ce_atomic_set(&shm_q->dbk_queue_mark,0);
//        ipc_shm_queue_data_block_create(shm_q,r_queue_size,dbk_size,f_dbk_size,dbk_num);

        return CE_OK;	
    }

    /*if shared memory has no enough space to create
     * queue,then return error.*/
    if(ipc_shm_remain_size(ipc_shm)<r_queue_size)
    {
        return CE_ERROR;
    }
    calculate_queue_args(r_queue_size,rcd_size,&dbk_size,&f_dbk_size,&dbk_num);

    /*alloc space in shared memory to create queue on it*/

    shm_q = (ce_ipc_shm_queue_t*)ipc_shm_alloc(ipc_shm,r_queue_size);
    assert(shm_q);
    ipc_shm_queue_init(ipc_shm,shm_q,queue_id,r_queue_size,dbk_size,f_dbk_size,dbk_num,rcd_size);

    ipc_shm_queue_add(ipc_shm,shm_q);	

    ipc_shm_stat_init(shm_q);

    INFO_LOG("alloc new queue");

    return CE_OK;
}


//查找下一个可用的reader index，并范围readerinfo
    static ce_ipc_shm_reader_info_t *
ipc_shm_queue_reader_free(ce_ipc_shm_queue_t *shm_q,size_t *indx)
{
    ce_ipc_shm_reader_info_t *r_info;
    size_t r_indx;

    for(r_indx=(size_t)shm_q ->ref_count; r_indx<CE_QUEUE_MAX_READER_NUM;r_indx++)
    {
        r_info=&(shm_q->readers[r_indx]);

        if(r_info->free)
        {
            *indx = r_indx;
            return r_info;
        }
    }
    *indx = CE_QUEUE_MAX_READER_NUM;
    return NULL;
}
//给shm_q增加reader,增加shmq的ref_count
    static void  
ipc_shm_queue_reader_add(ce_ipc_shm_queue_t *shm_q, char *reader_name)
{
    ce_ipc_shm_reader_info_t *r_info = NULL;

    pid_t r_pid = getpid();
    size_t indx;

    if((r_info=ipc_shm_queue_reader_find_by_name(shm_q, reader_name)) == NULL)
    {
    	INFO_LOG( "ipc read thread new register, pid %d, name %s.", r_pid, reader_name);

    	/*get the new reader info*/
		r_info = ipc_shm_queue_reader_free(shm_q,&indx);

		assert(r_info);

		/*inc the number of readers*/
		ce_atomic_inc(&shm_q->ref_count);

		ipc_shm_queue_reader_info_init(r_info, r_pid, 0, reader_name);
    }
    else
    {
    	/*已经添加过*/
    	INFO_LOG( "ipc read %s already register, update pid %d.", reader_name, r_pid);

		/*更新pid*/
		r_info->reader_pid = r_pid;
		/*设置为已经使用*/
		r_info->free = 0;
    }
}


#define ipc_shm_queue_get_from_writer(writer) (ce_ipc_shm_queue_t*)((writer)->writer_user_data)
#define ipc_shm_queue_get_from_reader(reader) (ce_ipc_shm_queue_t*)((reader)->reader_user_data)


//得到c_dbk的前一个dbk
    static inline ce_ipc_data_block_t *
ipc_queue_dbk_pre_get(ce_ipc_shm_queue_t *shm_q,ce_ipc_data_block_t *c_dbk,size_t header)
{
    ce_ipc_data_block_t *dbk,*pre_dbk = NULL;

    assert(shm_q&&c_dbk);	

    list_for_ipc_data_block(dbk,header,shm_q)
    {
        if(dbk->dbk_offset == c_dbk->dbk_offset)
        {
            break;
        }
        pre_dbk = dbk;
    }

    return pre_dbk;
}
//针对共享内存队列，将m_dbk删除
    static  void
ipc_queue_dbk_mov(ce_ipc_shm_queue_t *shm_q,ce_ipc_data_block_t *m_dbk,int is_free)
{
    ce_ipc_data_block_t *pre_dbk;

    assert(shm_q&&m_dbk);

    size_t* header = is_free?&(shm_q->free_dbk_header):&(shm_q->dbk_header);

    CE_IPC_LOCK(&shm_q->dbk_queue_mark);
    pre_dbk = ipc_queue_dbk_pre_get(shm_q,m_dbk,*header);

    if(pre_dbk==NULL)
    {
        *header = m_dbk->dbk_next_offset;//从链中直接删除
    }
    else
    {
        pre_dbk->dbk_next_offset = m_dbk->dbk_next_offset;	
    }

    m_dbk->dbk_next_offset = 0;

    CE_IPC_UNLOCK(&shm_q->dbk_queue_mark);
}



    static inline void 
ipc_queue_dbk_pos_update(ce_ipc_data_block_t *dbk,size_t size)
{
    dbk->dbk_pos+=size;
}
//ipc发布dbk
//int publish_count=0;
    static void 
ipc_publisher_dbk(ce_ipc_writer_t *ipc_writer,ce_ipc_data_block_t *dbk)
{
    ce_ipc_shm_queue_t *shm_q = ipc_shm_queue_get_from_writer(ipc_writer);
    //	char ok[1];
    //publish_count++;
    uint8_t i = 0;
    ce_data_writer_t d_writer ;

    ce_reset_pool(ipc_writer->pool);

    ipc_make_data_writer(ipc_writer->pool,&d_writer,1+4+4);

    /*write pub type*/
    ce_write_uint8(&d_writer,PUB_DBK);
    /*write q_id*/
    ce_write_int(&d_writer,shm_q->q_id);
    /*write dbk offset*/
    ce_write_uint32(&d_writer,dbk->dbk_offset);
    /*   if (publish_count == 10000)
         {
         DEBUG_LOG("publish dbk offset %d\n", dbk ->dbk_offset);	
         publish_count=0;
         } */
    /*set dbk state to reading*/

    dbk->dbk_state = DBK_READING;
    ce_atomic_set(&dbk->read_count,shm_q->ref_count);
    CE_IPC_LOCK(&shm_q ->reader_info_mark);
    for (i = 0; i < shm_q ->ref_count; i++)
    {
        dbk ->reader_pid[i] = shm_q ->readers[i].reader_pid;
    }
    CE_IPC_UNLOCK(&shm_q ->reader_info_mark);

    /*publish this buf to all readers*/
    if (-1 == zmq_send(ipc_writer->pub_sock,CE_DATA_GET_BUF_START(&d_writer.buf),1+4+4,0))
    {
    	shm_q->send_failed_cnt++;
    	ce_log(CE_LOG_ERR,"zmq_send error no %d.", errno);
    }
    else
    {
    	shm_q->send_cnt++;
    }
}

//检查dbk可否回收
    static inline int
ipc_dbk_is_collectalbe(ce_ipc_data_block_t *dbk)
{
    if (ce_atomic_read(&dbk->read_count)==0 || dbk ->dbk_state == DBK_READED)
    {
        return 1;
    }
    if(dbk->dbk_state==DBK_WRITE ||dbk->dbk_state ==DBK_READING )
        return 0;
    return 0;
}
//dbk回收
static void ipc_dbk_collect(ce_ipc_shm_queue_t *shm_q)
{
    ce_ipc_data_block_t *dbk;
    ce_int_t next_offset=1;
    // list_for_ipc_data_block(dbk,shm_q ->dbk_header,shm_q) //如果使用这句话， 下面的ipc_queue_dbk_add会把dbk->next_offset改变
    ce_int_t collected=0;	
    for(dbk=ipc_queue_dbk_first(shm_q,shm_q->dbk_header);next_offset;dbk=(ce_ipc_data_block_t*)ipc_shm_queue_addr_by_offset(shm_q,next_offset))
    {
        next_offset = dbk->dbk_next_offset;
        if(ipc_dbk_is_collectalbe(dbk))
        {
            collected =1;
            //   DEBUG_LOG("collect datablk offset %d\n", dbk->dbk_offset);
            ipc_queue_dbk_mov(shm_q,dbk,0);
            ipc_shm_queue_data_block_init(dbk,dbk->dbk_offset,dbk->dbk_size,sizeof(ce_ipc_data_block_t),DBK_FREE,0);
            ipc_queue_dbk_add(shm_q,dbk,&shm_q->free_dbk_header);

        }
    }
    //找不到可以回收的内存块,则打印哪些读者读得太慢
    if (collected == 0 && shm_q ->collect_count == 0)
    {
        int reader_count;
        shm_q ->collect_count = 1;
        reader_count = ce_atomic_read(&shm_q ->ref_count);
        ERR_LOG("shm queue %d reader_count=%d and can't collect", shm_q ->q_id, reader_count);

        if (reader_count > 0)
        {
//            CE_IPC_LOCK(&shm_q ->reader_info_mark);
            list_for_ipc_data_block(dbk,shm_q ->dbk_header,shm_q)
            {
                uint8_t i;
                size_t r_idx;
                int blk_rd_cnt = 0;

                for (i = 0; i < reader_count; i++)
                {
                    if (dbk ->reader_pid[i] !=  0)
                    {
                        ce_ipc_shm_reader_info_t *r_info = ipc_shm_queue_reader_find(shm_q, dbk ->reader_pid[i], &r_idx);
                        if (NULL == r_info)
                        {
                        	ERR_LOG("shm queue %d dbk offset %ld, find reader failed by index %d",
                        			shm_q ->q_id, dbk ->dbk_offset, i);
                        }
                        else
                        {
                        	blk_rd_cnt = ce_atomic_read(&dbk->read_count);
							ERR_LOG("shm queue %d dbk offset %ld ,state %d, read cnt %d, it has to be readed by %s",
									shm_q ->q_id, dbk ->dbk_offset,
									dbk->dbk_state, blk_rd_cnt,
									r_info ->reader_name);
                        }
                    }
                }
            }
//            CE_IPC_UNLOCK(&shm_q ->reader_info_mark);
        }
    } 
    /*    if (collected == 0)
          {
          collect_count++;

          }
          if (collect_count == 1 && collected == 0)
          {
          int reader_count;
          reader_count = ce_atomic_read(&shm_q ->ref_count);
          ERR_LOG("collect all dbk, and desc ref count, reader_count=%d\n", reader_count);
          if (reader_count > 0)
          {
          list_for_ipc_data_block(dbk,shm_q ->dbk_header,shm_q)
          {
          ERR_LOG("shm queue %d dbk offset %d , it has to be readed %d",shm_q ->q_id, dbk ->dbk_offset, \
          ce_atomic_read(&dbk->read_count));
          ce_atomic_set(&dbk->read_count, 0);
          }
          ce_atomic_dec(&shm_q ->ref_count);
          }
          collect_count=0;
          }  */
}
//获取dbk
    static ce_ipc_data_block_t *
ipc_queue_dbk_get(ce_ipc_writer_t *ipc_writer,ce_ipc_shm_queue_t *shm_q,size_t rcd_size)
{
    ce_ipc_data_block_t *dbk=NULL;

    int cur_write_dbk_offset = ce_atomic_read(&shm_q->cur_w_dbk);
    if(cur_write_dbk_offset)
    {
        dbk = (ce_ipc_data_block_t*)ipc_shm_queue_addr_by_offset(shm_q,ce_atomic_read(&shm_q->cur_w_dbk));

        if(!ipc_queue_dbk_full(dbk,rcd_size))
        {
            dbk->rcd_num+=1;
            return dbk;
        }
        /*not allowed to write */	
//        ce_atomic_set(&shm_q->cur_w_dbk, 0);
        if (ce_atomic_cas(&shm_q->cur_w_dbk, 0, cur_write_dbk_offset) != cur_write_dbk_offset)
		{
        	ce_log(CE_LOG_INFO,"shm_q %d's cur write dlb already publish by timeout.", shm_q->q_id);
		}
        else
        {
			/*publish this dbk to read by all processes*/
			ipc_publisher_dbk(ipc_writer,dbk);
        }
    }

    if(shm_q->free_dbk_header == 0)
    {
        //printf("begin to collect\n");
        ipc_dbk_collect(shm_q);	
    }

    list_for_ipc_data_block(dbk,shm_q->free_dbk_header,shm_q)
    {
        if(!ipc_queue_dbk_full(dbk,rcd_size))
            break;
    }

    if(dbk==NULL)
        return NULL;

    ipc_queue_dbk_mov(shm_q,dbk,1);
    ipc_queue_dbk_add(shm_q,dbk,&shm_q->dbk_header);
    ce_atomic_set(&shm_q->cur_w_dbk, dbk->dbk_offset);
    /*set dbk state to write*/
    dbk->dbk_state = DBK_WRITE;
    dbk->rcd_num+=1;
    return dbk;
}

static inline void * ipc_queue_dbk_addr_by_offset(ce_ipc_data_block_t *dbk,size_t offset)
{
    return (char *)dbk+offset;
}

static void decision_write_area(ce_ipc_writer_t *ipc_writer,ce_ipc_shm_queue_t *shm_q,ce_ipc_data_block_t *dbk,
        ce_ipc_shm_data_area_t *d_area,size_t rcd_size)
{
    ce_unused(shm_q);
    char *addr = (char *)ipc_queue_dbk_addr_by_offset(dbk,dbk->dbk_pos);
    ipc_queue_dbk_pos_update(dbk,rcd_size);

    ce_data_wrapbuf_t *buf = &ipc_writer->writer_buf;

    d_area->addr=addr;
    d_area->len=addr==NULL?0:rcd_size;

    CE_DATA_SET_BUF_START(buf,addr);
    CE_DATA_SET_BUF_POS(buf,addr);
    CE_DATA_SET_BUF_END(buf,addr+d_area->len);

    buf->expand=NULL;
}

/*
 *@breif:                共享内存头部信息打印
 *@param ipc_context:    IPC上下文
 *@param ipc_shm:        ipc_shm结构
 */
void ipc_shm_debug(ce_ipc_t *ipc)
{

    ce_ipc_shm_header_t *ipc_shm_header;
    ce_ipc_shm_t *ipc_shm = ce_ipc_shm_get(ipc);
    ipc_shm_header = (ce_ipc_shm_header_t*)(ipc_shm->ipc_shm_addr);

    INFO_LOG("shm header magic: %x", ipc_shm_header->shm_magic);
    INFO_LOG("shm header queue_header: %lx", ipc_shm_header->shm_queue_header);
    INFO_LOG("shm header size: %ld", ipc_shm_header->shm_size);
    INFO_LOG("shm header queue_num: %ld", ipc_shm_header->shm_queue_num);
    INFO_LOG("shm header shm_free_pos: %ld", ipc_shm_header->shm_free_pos);
    INFO_LOG("shm header dbk_pub_port: %d", ipc_shm_header->dbk_pub_port);

    ce_ipc_shm_queue_t *shm_q=NULL;
    int i = 0;
    int read_cnt = 0;
    ce_ipc_shm_reader_info_t *r_info = NULL;

    ce_ipc_data_block_t *dbk = NULL;
	uint64_t used_cnt = 0, free_cnt = 0;
	uint64_t reader_blocked_cnt[CE_QUEUE_MAX_READER_NUM] = {0};

    list_for_ipc_shm_queue(shm_q,ipc_shm)
    {
    	used_cnt = 0;
    	free_cnt = 0;
    	memset(reader_blocked_cnt, 0, sizeof(reader_blocked_cnt));

    	read_cnt = ce_atomic_read(&shm_q ->ref_count);

		CE_IPC_LOCK(&shm_q ->reader_info_mark);
		list_for_ipc_data_block(dbk, shm_q ->dbk_header, shm_q)
		{
			used_cnt++;

			for (i = 0; i < read_cnt; i++)
			{
				if (dbk ->reader_pid[i] != 0)
				{
					reader_blocked_cnt[i]++;

					if (dbk ->reader_pid[i] != shm_q->readers[i].reader_pid)
					{
						ERR_LOG("dbk of shm %d owner to pid %d, expect %d, index %d.",
								shm_q ->q_id,
								dbk ->reader_pid[i],
								shm_q->readers[i].reader_pid, i);
					}
				}
			}
		}

		list_for_ipc_data_block(dbk, shm_q->free_dbk_header, shm_q)
		{
			free_cnt++;
		}
		CE_IPC_UNLOCK(&shm_q ->reader_info_mark);


        INFO_LOG("==============shm_queue id = %d", shm_q ->q_id);
        INFO_LOG("shm_queue offset = %ld", shm_q ->q_offset);
        INFO_LOG("shm_queue len = %ld", shm_q ->q_len);
        INFO_LOG("shm_queue next offset = %ld", shm_q ->q_next_offset);
        INFO_LOG("shm_queue cur_w_dbk = %ld", ce_atomic_read(&shm_q ->cur_w_dbk));
        INFO_LOG("shm_queue dbk_header = %ld", shm_q ->dbk_header);
        INFO_LOG("shm_queue free_dbk_header = %ld", shm_q ->free_dbk_header);
        INFO_LOG("shm_queue ref_count = %ld", ce_atomic_read(&shm_q ->ref_count));
        INFO_LOG("shm_queue dbk_queue_mark = %ld", ce_atomic_read(&shm_q ->dbk_queue_mark));
        INFO_LOG("shm_queue dbk_write_mark = %ld", ce_atomic_read(&shm_q ->dbk_write_mark));
        INFO_LOG("shm_queue rcd_max_size = %ld", shm_q ->rcd_max_size);
        INFO_LOG("shm_queue reader_info_mark = %ld", ce_atomic_read(&shm_q ->reader_info_mark));
        INFO_LOG("shm_queue send cnt = %ld", shm_q->send_cnt);
        INFO_LOG("shm_queue send failed cnt = %ld", shm_q->send_failed_cnt);
        INFO_LOG("shm_queue blk get failed cnt = %ld", shm_q->get_blk_failed_cnt);
        INFO_LOG("shm_queue read clear cnt = %ld", shm_q->re_reg_clear_cnt);
        INFO_LOG("shm_queue used blk cnt = %lu", used_cnt);
        INFO_LOG("shm_queue free blk cnt = %lu", free_cnt);
        INFO_LOG("reader_ino:");
        i=0;
        r_info=&(shm_q->readers[i]);
        for( i=0;i<CE_QUEUE_MAX_READER_NUM && r_info ->free == 0;i++)
        {
            r_info=&(shm_q->readers[i]);
            INFO_LOG("reader info pid %d", r_info ->reader_pid);
            INFO_LOG("reader info free %d", r_info ->free);
            INFO_LOG("reader info name %s", r_info ->reader_name);
            INFO_LOG("reader info rcv cnt %ld", r_info ->rcv_cnt);
            INFO_LOG("reader cur block cnt %lu", reader_blocked_cnt[i]);
        }
    }
}

#if 0
static void ipc_shm_info_debug(ce_ipc_shm_queue_t *shm_q)
{
	running_log_t r_log;
	memset(&r_log, 0, sizeof(r_log));

	int i=0, read_cnt = 0;
	ce_ipc_shm_reader_info_t *r_info = NULL;

	read_cnt = ce_atomic_read(&shm_q ->ref_count);

	ce_ipc_data_block_t *dbk = NULL;
	uint64_t used_cnt = 0, free_cnt = 0;
	uint64_t reader_blocked_cnt[CE_QUEUE_MAX_READER_NUM] = {0};

	CE_IPC_LOCK(&shm_q ->reader_info_mark);
	list_for_ipc_data_block(dbk, shm_q ->dbk_header, shm_q)
	{
		used_cnt++;

		for (i = 0; i < read_cnt; i++)
		{
			if (dbk ->reader_pid[i] != 0)
			{
				reader_blocked_cnt[i]++;

				if (dbk ->reader_pid[i] != shm_q->readers[i].reader_pid)
				{
					ERR_LOG("dbk of shm %d owner to pid %d, expect %d, index %d.",
							shm_q ->q_id,
							dbk ->reader_pid[i],
							shm_q->readers[i].reader_pid, i);
				}
			}
		}
	}

	list_for_ipc_data_block(dbk, shm_q->free_dbk_header, shm_q)
	{
		free_cnt++;
	}
	CE_IPC_UNLOCK(&shm_q ->reader_info_mark);

	r_log.type = ENGINE_INFO;
	snprintf(r_log.desc, sizeof(r_log.desc),
			"blk full in shm queue %d, reader cnt %d, send %lu, send failed %lu, failed %lu, clear %lu, used blk %lu, free blk %lu",
			shm_q ->q_id,
			read_cnt,
			shm_q->send_cnt, shm_q->send_failed_cnt, shm_q->get_blk_failed_cnt,
			shm_q->re_reg_clear_cnt,
			used_cnt, free_cnt);

	RunningLog(CE_LOG_WARN, &r_log);

	WARN_LOG("blk full in shm queue %d, reader cnt %d, send %lu, send failed %lu, failed %lu, clear %lu, used blk %lu, free blk %lu",
							shm_q ->q_id,
							read_cnt,
							shm_q->send_cnt, shm_q->send_failed_cnt, shm_q->get_blk_failed_cnt,
							shm_q->re_reg_clear_cnt,
							used_cnt, free_cnt);

	for( i=0; i<CE_QUEUE_MAX_READER_NUM; i++)
	{
		r_info=&(shm_q->readers[i]);
		if (r_info ->free)
		{
			break;
		}

		memset(&r_log, 0, sizeof(r_log));
		r_log.type = ENGINE_INFO;
		snprintf(r_log.desc, sizeof(r_log.desc),
				"blk full in shm queue %d, reader %s, pid %d, recv %lu, block cnt %lu",
				shm_q ->q_id, r_info->reader_name, r_info->reader_pid, r_info->rcv_cnt, reader_blocked_cnt[i]);
		RunningLog(CE_LOG_WARN, &r_log);

		WARN_LOG("blk full in shm queue %d, reader %s, pid %d, recv %lu, block cnt %lu",
				shm_q ->q_id, r_info->reader_name, r_info->reader_pid, r_info->rcv_cnt, reader_blocked_cnt[i]);
	}
}
#endif

/*
 *@breif:                写者写数据接口
 *@param data_writer:    注册得到的写者数据结构
 *@param data_record:    要写的记录结构
 *@param w_size:         实际上占用共享内存大小
 */
ce_int_t ipc_shm_record_write(ce_data_writer_t *data_writer,ce_data_record_t *data_record,size_t *w_size)
{

    ce_ipc_shm_data_area_t d_area;
    size_t write_size;
    ce_ipc_shm_queue_t *shm_q;
    ce_ipc_writer_t *ipc_writer;
    ce_ipc_data_block_t *wr_dbk = NULL;
    size_t t_size = data_record->rcd_size+4+1;
    //collect_count=0;
    assert(data_writer&&data_record);

    ipc_writer = ce_ipc_writer_get(data_writer);

    shm_q = ipc_shm_queue_get_from_writer(ipc_writer);

    CE_IPC_LOCK(&shm_q->dbk_write_mark);
    /*find data block to write.*/
    do{
        /*find data block to write.*/
    	wr_dbk = ipc_queue_dbk_get(ipc_writer,shm_q,t_size);
        if(wr_dbk)
        {
            break;
        }
        else
        {
            CE_IPC_UNLOCK(&shm_q->dbk_write_mark);

            shm_q->get_blk_failed_cnt++;

            /*每20000个记录一次运行日志*/
            if (0 == (shm_q->get_blk_failed_cnt % 200000))
            {
            	int i = 0;
            	ce_ipc_shm_reader_info_t *r_info = NULL;

            	//ipc_shm_info_debug(shm_q);

            	for( i=0; i<CE_QUEUE_MAX_READER_NUM; i++)
				{
					r_info=&(shm_q->readers[i]);
					if (r_info ->free)
					{
						break;
					}

					if (CE_FALSE == _get_proc_exist(r_info->reader_pid))
					{
						WARN_LOG("read %s of shm queue %d not exist, pid %d, will clear all blce.",
								r_info->reader_name, shm_q ->q_id, r_info->reader_pid);

						/*清除注册信息*/
						ipc_shm_queue_reader_info_clean(shm_q, r_info->reader_pid);
					}
				}
            }

            return CE_ERROR;
        }

        struct timespec t;
        t.tv_sec = 0;
        t.tv_nsec = 1000;
        nanosleep(&t,NULL);
    }while(!wr_dbk);

    decision_write_area(ipc_writer,shm_q,wr_dbk,&d_area,t_size);

    if(d_area.addr==NULL)
    {
    	ERR_LOG("d_area.addr NULL when write record.");
    	shm_q->get_blk_failed_cnt++;

        CE_IPC_UNLOCK(&shm_q->dbk_write_mark);
        return CE_ERROR;
    }

    /*write start flag*/
    ce_write_int8(data_writer,RCD_START_MARK);
    ce_write_uint32(data_writer,data_record->rcd_size);				
    write_size = ce_data_record_write(data_writer,data_record);
    assert(write_size==data_record->rcd_size);
    /*update write time*/
    if (wr_dbk ->rcd_num == 1)
    	wr_dbk->first_write_time = time((time_t*)NULL);

    *w_size = t_size;
    CE_IPC_UNLOCK(&shm_q->dbk_write_mark);

    return CE_OK;
}

typedef struct 
{
    ce_ipc_reader_t *ipc_reader;
    ce_data_record_t *data_record;
    ce_ipc_read_handle_fn read_handle;
}read_thread_user_data_t;

    static int 
dbk_has_next(ce_ipc_dbk_iter_t *iter)
{
    return (iter->cur_addr!=NULL);
}

    static ce_data_reader_t* 
dbk_next(ce_ipc_dbk_iter_t *iter)
{
    char *next_addr;
    uint32_t next_rcd_size;
    int8_t start_mark;

    ce_data_reader_t *d_reader = iter->d_reader;

    ce_data_wrapbuf_t *buf = &d_reader->buf;

    /*没有下一个记录了*/
    if (iter ->dbk ->rcd_num <= 0)
    {
        CRIT_LOG("dbk rcd num = %ld", iter ->dbk ->rcd_num);
    }
    if( iter->cur_rcd_num >= iter->dbk->rcd_num-1)
    {
        next_addr = NULL;
        next_rcd_size = 0;
    }

    else
    {   
        ///读取下一个记录的地址和大小
        next_addr = (char *)iter->cur_addr+5+iter->cur_size;

        CE_DATA_SET_BUF_START(buf,next_addr);

        CE_DATA_SET_BUF_POS(buf,next_addr);

        CE_DATA_SET_BUF_END(buf,next_addr+5);

        /*read record start mark*/
        ce_read_int8(d_reader,&start_mark);
        if (start_mark != (int8_t)RCD_START_MARK)
        {
            CRIT_LOG("dbk offset 0x%lx, cur_rcd_num %ld, dbk_rcd_num %ld, start_mark 0x%x is not RCD_START_MARK.",
            		iter ->dbk ->dbk_offset, iter ->cur_rcd_num, iter ->dbk ->rcd_num, start_mark);
            assert(0);
        }
        /*read record size*/
        ce_read_uint32(d_reader,&next_rcd_size);
    }
    ///准备读取本记录
    CE_DATA_SET_BUF_START(buf, iter->cur_addr+5);

    CE_DATA_SET_BUF_POS(buf, iter->cur_addr+5);

    CE_DATA_SET_BUF_END(buf, iter->cur_addr+5+iter->cur_size);

    ///设置下次读取的记录位置
    iter->cur_addr = next_addr;
    iter->cur_size = next_rcd_size;
    iter->cur_rcd_num+=1;

    return d_reader;
}
/*
 *@brief:                初始化数据块iter
 *@param d_reader:       data_reader结构
 *@param iter:          iter结构
 */
static void ipc_dbk_iter_init(ce_ipc_dbk_iter_t *iter,ce_ipc_data_block_t *dbk,ce_data_reader_t *d_reader,void *user_data)
{
    int8_t start_mark;
    uint32_t rcd_size;

    iter->dbk = dbk;
    iter->cur_rcd_num=0;  ///已经读到0个记录
    iter->d_reader = d_reader;

    iter->user_data = user_data; 

    iter->cur_addr = (char *)dbk+sizeof(ce_ipc_data_block_t);  ///去掉数据块头部,开始就是记录

    /*先读取记录开始标志和记录长度*/ 
    ce_data_wrapbuf_t *buf = &d_reader->buf;

    CE_DATA_SET_BUF_START(buf,iter->cur_addr);

    CE_DATA_SET_BUF_POS(buf,iter->cur_addr);

    CE_DATA_SET_BUF_END(buf, iter->cur_addr+5);

    /*read record start mark*/
    ce_read_int8(d_reader,&start_mark);
    if (start_mark != (int8_t)RCD_START_MARK)
    {
        CRIT_LOG("start_mark 0x%x is not RCD_START_MARK.", start_mark);
        assert(0);
    }
    /*read record size*/
    ce_read_uint32(d_reader,&rcd_size);

    iter->cur_size = rcd_size;
    iter->has_next = dbk_has_next;
    iter->next = dbk_next;
}
/*
 *@brief:                    线程读数据块
 *@param ipc_reader:         ipc读者
 *@param data_record:        读记录的结构 
 *@param read_handle:        读者回调函数
 *@param dbk:                要读的数据块
 */
static void do_dkb_data_record_read(ce_ipc_reader_t *ipc_reader,ce_data_record_t *data_record,ce_ipc_read_handle_fn read_handle,ce_ipc_data_block_t *dbk)
{

    ce_ipc_dbk_iter_t iter;	
    ce_ipc_shm_queue_t *shm_q = (ce_ipc_shm_queue_t *)ipc_reader ->reader_user_data;
    ce_data_reader_t *d_reader;
    ipc_dbk_iter_init(&iter,dbk,&ipc_reader->data_reader,NULL); 
    while(iter.has_next(&iter))
    {
        data_record ->rcd_size = iter.cur_size;
        d_reader = iter.next(&iter);
        if (NULL == d_reader)
        {
            CRIT_LOG( "d_reader NULL.");
            assert(0);
        }
        //       DEBUG_LOG("record size %d\n", data_record ->rcd_size);
        ce_data_record_read(d_reader,data_record);
        /*call callback function to handle the data record*/
        read_handle(d_reader,data_record,ipc_reader->private_data);
    }
    /*dec the number of read count, and set the dbk has read by this reader*/
    if(ce_atomic_read(&dbk->read_count) > 0)
    {
        int i;
        int reader_count = ce_atomic_read(&shm_q ->ref_count);
        for (i = 0; i < reader_count;i++)
        {
            if (dbk ->reader_pid[i] == getpid())
            {
                dbk ->reader_pid[i] = 0;
                /*接受统计*/
                shm_q->readers[i].rcv_cnt++;
                break;
            }
        }
        if (i < reader_count)
        {
            ce_atomic_dec(&dbk->read_count);
        }
        else
        {
            ERR_LOG("the dbk should not be reader by %d", getpid());
        }
    }
    else
    {
        ERR_LOG("queue id %d dbk offset %ld reader count %ld and can't desc, dbk state = %d",\
                ipc_reader ->q_id, dbk ->dbk_offset, ce_atomic_read(&dbk ->read_count), dbk ->dbk_state);
    }

    if(ce_atomic_read(&dbk->read_count)==0)
    {
        dbk->dbk_state =DBK_READED;
    }
}

/*
 *@brief:           读者线程停止处理函数
 *@param user_data: 读者线程数据结构体
 */
static int stop_read_thread_fn(ce_thread_t *read_thread, void *user_data)
{
    ce_unused(read_thread);
#if 0
    read_thread_user_data_t * ud = (read_thread_user_data_t*)user_data;

    ce_ipc_reader_t *ipc_reader = ud->ipc_reader;
    ipc_reader ->stop_read = CE_TRUE;
#else
    ce_unused(user_data);
#endif

    INFO_LOG("entry ipc read thread exit func.");

    return CE_OK;
}

#if 0
static void ipc_rd_thrd_reg_cleanup(void * arg)
{
    ce_ipc_reader_t *ipc_reader = (ce_ipc_reader_t*)arg;

    INFO_LOG( "ipc read thread cleanup begin.");

    if (NULL != g_ipc_beat_funcs.beat_unregister)
    {
        g_ipc_beat_funcs.beat_unregister(pthread_self());
    }

    /*清除线程相关的资源*/

    ce_ipc_shm_queue_t *shm_q = ipc_shm_queue_get_from_reader(ipc_reader);

    /*清除注册信息*/
    ipc_shm_queue_reader_info_clean(shm_q, getpid());

	/*强制设置为TRUE*/
	ipc_reader ->stop_read = CE_TRUE;

	INFO_LOG( "ipc read thread cleanup success.");
}
#endif

/*
 *@brief:               读者工作线程
 *@param read_thread:   线程结构体
 *@param user_data:     读者传递给线程的封装数据
 */
static void *read_thread_fn(ce_thread_t *read_thread,void *user_data)
{

    int rc;
    uint8_t cmd;
    uint32_t dbk_offset;
    int q_id;
    ce_ipc_data_block_t *dbk;


    ce_data_reader_t d_reader;
    d_reader.ops = &ce_reader_encoding;

    ce_data_wrapbuf_t *wrap_buf=&d_reader.buf;

    read_thread_user_data_t * ud = (read_thread_user_data_t*)user_data;

    ce_ipc_reader_t *ipc_reader = ud->ipc_reader;

    ce_data_record_t *data_record = ud->data_record;

    ce_ipc_read_handle_fn read_handle = ud->read_handle;

    ce_ipc_shm_queue_t *shm_q = ipc_shm_queue_get_from_reader(ipc_reader);

    char *buf = (char *)ce_palloc(ipc_reader->pool,shm_q->rcd_max_size+32);
    assert(buf);
    /* entry_thrd_reg_t thrd_reg;
       pthread_t tid = pthread_self();
       bzero(&thrd_reg, sizeof(entry_thrd_reg_t));
       thrd_reg.exit_func = NULL;
       thrd_reg.expire_time = 15;
       snprintf(thrd_reg.thrd_name, sizeof(thrd_reg.thrd_name), "ipc_read_%d", shm_q ->q_id);
       thrd_reg.expire_handle_type = ENTRY_EXPIRE_NOTHING;

       if (CE_OK != entry_thrd_register(tid, &thrd_reg))
       {
       fprintf(stderr, "fail to regsiter read ipc thread moniter\n");
       exit(-1);
       } */
    #if 0
    char thd_name[32] = {0};
    pthread_t tid = pthread_self();

    snprintf(thd_name, 32, "ipc-rd-%s-queue",
            (shm_q->q_id == IN_REGION_QUEUE) ? "inreg" :
            (shm_q->q_id == OUT_REGION_QUEUE) ? "outreg" :
            (shm_q->q_id == IN_NETWORK_QUEUE) ? "innet" :
            (shm_q->q_id == OUT_NETWORK_QUEUE) ? "outnet" :
            (shm_q->q_id == FULL_QUEUE) ? "full" : "unknown");
    if (NULL != g_ipc_beat_funcs.beat_register)
    {
        g_ipc_beat_funcs.beat_register(tid, thd_name, 20); /*10秒超时*/
    }

    /// 注册线程退出清理函数
    pthread_cleanup_push(ipc_rd_thrd_reg_cleanup, (void*)ipc_reader);
   #endif

    zmq_pollitem_t items[1] = 
    {
        {ipc_reader ->sub_sock, 0, ZMQ_POLLIN, 0},
    };
    while( 1/*ipc_reader ->stop_read == CE_FALSE*/)
    {
        #if 0
	if (NULL != g_ipc_beat_funcs.beat)
        {
            g_ipc_beat_funcs.beat(tid);
        }
	#endif

        //        CE_CHECK_THEAD_EXIT(read_thread);
        if(read_thread->thread_exit_stat == THD_EXIT)
        {
            if(read_thread->exit_fun)
                read_thread->exit_fun(read_thread,read_thread->data);

           read_thread->thread_exit_stat = THD_EXITED;
            break;
        }

        zmq_poll(items, 1, 3000);
        if (items[0].revents & ZMQ_POLLIN)
        {
            rc = zmq_recv(ipc_reader->sub_sock,buf,
                    shm_q->rcd_max_size+32,0);

            if(rc>0)
            {
                CE_DATA_SET_BUF_START(wrap_buf,buf);

                CE_DATA_SET_BUF_POS(wrap_buf,buf);

                CE_DATA_SET_BUF_END(wrap_buf,(char *)buf+rc);

                wrap_buf->expand=NULL;

                /*read cmd type*/
                ce_read_uint8(&d_reader,&cmd);
                if(cmd == PUB_DBK)
                {
                    /*read q_id*/
                    ce_read_int(&d_reader,&q_id);

                    if(q_id!= ipc_reader->q_id)
                        continue;
                }

                switch(cmd)
                {

                    case PUB_READER_AWAKE:

                        break;

                    case PUB_DBK:
                        ce_read_uint32(&d_reader,&dbk_offset);

                        dbk =(ce_ipc_data_block_t*)ipc_shm_queue_addr_by_offset(shm_q,dbk_offset);
                        if (dbk->dbk_offset != dbk_offset)
                        {
                            CRIT_LOG( "dbk->dbk_offset %ld, expect %u.", dbk->dbk_offset, dbk_offset);
                            assert(0);
                        }

                        do_dkb_data_record_read(ipc_reader,data_record,read_handle,dbk);			

                        break;

                    default:
                        break;
                }
            }
        }
    }

    /*以return方式返回时，需要手动pop*/
    //pthread_cleanup_pop(1);
    return NULL;	
}

/*
 *@brief:                读者创建读共享内存线程
 *param data_reader:     注册的读者结构
 *param data_record:     要读的记录结构
 *param read_handle:     读者的处理函数
 */
ce_int_t ipc_shm_record_read(ce_data_reader_t *data_reader,ce_data_record_t *data_record,ce_ipc_read_handle_fn read_handle)
{
    ce_ipc_reader_t *ipc_reader;

    assert(data_reader&&data_record&&read_handle);

    ipc_reader = ce_ipc_reader_get(data_reader);
    //    DEBUG_LOG("alloc from the reader pool for read thread user data\n");	
    read_thread_user_data_t *user_data = (read_thread_user_data_t*)ce_palloc(ipc_reader->pool,sizeof(read_thread_user_data_t));

    assert(user_data);

    user_data->ipc_reader = ipc_reader;
    user_data->data_record = data_record;
    user_data->read_handle = read_handle;

    /*create dbk read thread to accept the dbk*/
    //	printf("create dbk read thread to accept the dbk\n");
    ce_thread_create(&ipc_reader->read_thread,
            NULL,
            read_thread_fn,
            stop_read_thread_fn,
            (void*)user_data,
            ipc_reader->pool);

    if(ipc_reader->read_thread==NULL)
        return CE_ERROR;
    return CE_OK;
}


/*
 *@brief:               读者注册流队列q_id
 *@param ipc :          注册的IPC结构
 *@param q_id :         要注册的队列ID
 *@param private_data : 要注册的读者私有数据.
 */
    ce_data_reader_t*
ipc_shm_reader_register(ce_ipc_t *ipc, int q_id,void *private_data, char *reader_name)
{

    char connect_port[64] = {0};
    ce_ipc_shm_queue_t *shm_q;
    ce_ipc_shm_t *ipc_shm;
    ce_ipc_shm_header_t *ipc_shm_header;
    ce_data_wrapbuf_t *buf;
    ce_ipc_reader_t *ipc_reader;

    assert(ipc);
    ipc_shm = ce_ipc_shm_get(ipc);
    ipc_shm_header = ipc_shm_header_get(ipc_shm);

    shm_q=ipc_shm_queue_get(ipc_shm,q_id);
    /*queue to be read no existed*/
    if(!shm_q)
    {
    	ERR_LOG("get queue %d failed.", q_id);
        return NULL;
    }

    /*check the number of readers */
    if(ce_atomic_read(&shm_q->ref_count) >= CE_QUEUE_MAX_READER_NUM)
    {
    	ERR_LOG("queue %d reader full.", q_id);
        return NULL;
    }

    /*如果有此读者之前的数据，清除掉*/
    ce_ipc_shm_reader_info_t *read_info = ipc_shm_queue_reader_find_by_name(shm_q, reader_name);
    if (NULL != read_info)
    {
    	_ipc_clear_dbk_for_reader(shm_q, read_info);
    }

    ipc_reader = (ce_ipc_reader_t*)ce_palloc(ipc_shm->ipc_shm_pool,sizeof(ce_ipc_reader_t));

    assert(ipc_reader);

    buf = &ipc_reader->reader_buf;

    ipc_reader->reader_user_data=(void*)shm_q;

    ipc_reader->reader_ops=&ce_reader_encoding;
    ipc_reader->private_data = private_data;
    CE_DATA_SET_BUF_START(buf,NULL);
    CE_DATA_SET_BUF_POS(buf,NULL);
    CE_DATA_SET_BUF_END(buf,NULL);

    buf->expand=NULL;
    int rc =CE_OK;

    do
    {
        /*create zeromq context for reader*/
        if(!(ipc_reader->sub_context = zmq_init(1)))
        {
            rc =CE_ERROR;
            break;
        }

        /*create zeromq socket to connect to writer*/
        if(!(ipc_reader->sub_sock = zmq_socket(ipc_reader->sub_context,ZMQ_SUB)))
        {
            rc = CE_ERROR;
            break;
        }

        /*connect to shm_queue's publisher port*/
        sprintf(connect_port,"tcp://localhost:%d",ipc_shm_header->dbk_pub_port);

        if(zmq_connect(ipc_reader->sub_sock,connect_port)<0)
        {
            rc = CE_ERROR;
            break;
        }

        /*set the socket options*/
        if(zmq_setsockopt(ipc_reader->sub_sock,ZMQ_SUBSCRIBE,NULL,0)<0)
        {
            rc = CE_ERROR;
            break;
        }

    }while(0);

    if(rc==CE_ERROR)
    {
        if(ipc_reader->sub_sock)
        {
            zmq_close(ipc_reader->sub_sock);
        }

        if(ipc_reader->sub_context)
        {
            zmq_term(ipc_reader->sub_context);
        }

        ERR_LOG("queue %d zmq init failed.", q_id);
        return NULL;
    }

    /*add this ipc_reader into queue*/
    CE_IPC_LOCK(&shm_q->reader_info_mark);
    ipc_shm_queue_reader_add(shm_q, reader_name);
    CE_IPC_UNLOCK(&shm_q->reader_info_mark);

    ipc_reader->pool = ce_create_pool(512+sizeof(ce_pool_t));
    assert(ipc_reader->pool);

    ipc_reader->read_thread = NULL;
    ipc_reader->q_id = q_id;
    ipc_reader ->stop_read = CE_FALSE;

    INFO_LOG("reader register queue id %d", q_id);

    return &(ipc_reader->data_reader);
}

/*
 *@brief:                读者注销接口
 *@param data_reader:    注册的读者数据结构
 *@param ipc:            注册的IPC结构
 */
void ipc_shm_reader_unregister(ce_ipc_t *ipc,ce_data_reader_t *data_reader)
{
    assert(ipc && data_reader);

    ce_ipc_reader_t *ipc_reader = ce_ipc_reader_get(data_reader);

    /*wait to exit read thread*/
    // ipc_awake_reader_thread(ipc_reader);

    /*disconnected to publisher and awake port */
   zmq_close(ipc_reader->sub_sock);
   zmq_term(ipc_reader->sub_context);

    if (CE_FALSE == ipc_reader->stop_read)
    {
        INFO_LOG( "wait ipc read thread exit, pid %d.", getpid());

        CE_WAIT_THREAD_EXIT(ipc_reader->read_thread);

        /*等待cleap up被调用*/
        while(CE_FALSE == ipc_reader->stop_read)
        {
        	sleep(1);
        }
    }

	/*destroy reader 's pool*/
	ce_destroy_pool(ipc_reader->pool);
}
/*
 *@breif:               共享内存注册写者
 *@param ipc:           注册的IPC结构
 *@param q_id:          注册的流队列id
 *@return:              写者结构体
 */
    ce_data_writer_t*
ipc_shm_writer_register(ce_ipc_t *ipc,int q_id)
{

    ce_ipc_writer_t *ipc_writer;

    ce_ipc_shm_queue_t *shm_q;
    ce_ipc_shm_t *ipc_shm;
    ce_data_wrapbuf_t *buf;

    assert(ipc);
    ipc_shm = ce_ipc_shm_get(ipc);

    ipc_writer = (ce_ipc_writer_t*)ce_palloc(ipc_shm->ipc_shm_pool,sizeof(ce_ipc_writer_t));

    assert(ipc_writer);

    buf=&ipc_writer->writer_buf;

    /*try to get the queue to be written,if queue is not existed,then return NULL
     * */
    shm_q=ipc_shm_queue_get(ipc_shm,q_id);

    if(!shm_q)
    {
        return NULL;
    }

    ipc_writer->writer_user_data = (void*)shm_q;

    ipc_writer->writer_ops=&ce_writer_encoding;
    /*create a zeromq context */

    /*create a zeromq socket*/
    ipc_writer->pub_sock = ipc ->pub_sock;

    if(ipc_writer->pub_sock==NULL)
        return NULL;


    CE_DATA_SET_BUF_START(buf,NULL);
    CE_DATA_SET_BUF_POS(buf,NULL);
    CE_DATA_SET_BUF_END(buf,NULL);

    buf->expand=NULL;


    if((ipc_writer->pool = ce_create_pool(512+sizeof(ce_pool_t)))==NULL)
        return NULL;

    return &(ipc_writer->data_writer);
}

/*
 *@brief:               写者注销接口
 *@param ipc:           注册的IPC结构
 *@param data_writer:   写者数据结构
 */
void ipc_shm_writer_unregister(ce_ipc_t *ipc,ce_data_writer_t *data_writer)
{

    assert(ipc && data_writer);

    ce_ipc_writer_t *ipc_writer = ce_ipc_writer_get(data_writer);

    ce_destroy_pool(ipc_writer->pool);
}

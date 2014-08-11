/*
 * =====================================================================================
 *
 *    Filename:  ce_fs_monitor.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013/07/09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng 
 *   Organization:  CE,2012,NTA PROJECT TEAM 
 *
 * =====================================================================================
 */

#include "ce_fs_monitor.h"
#include "ce_file.h"
static int fsm_thread_exit_callback_fun(ce_thread_t*,void *);
static ce_file_t file,*file_ptr=&file;
static ce_pollfd_t pfd,*pfd_ptr=&pfd;

static ce_fs_monitor_object_t *get_mon_object_by_fd(ce_fs_monitor_t *fsm,int fd)
{
	if(fsm==NULL||fd<0)
	{
		return NULL;
	}
	ce_fs_monitor_object_t *fsm_obj=NULL;
	FSM_LOCK(fsm);
	list_for_each_entry(fsm_obj,&fsm->objs,anchor)
	{
		if(fsm_obj->wd==fd)
		{
			FSM_UNLOCK(fsm);
			return fsm_obj;
		}	
	}
	FSM_UNLOCK(fsm); 
	return NULL;
}

static ce_fs_monitor_object_t *get_mon_object_by_name(ce_fs_monitor_t *fsm,ce_str_t *obj_name)
{
	if(fsm==NULL||obj_name==NULL||obj_name->data==NULL||obj_name->len<=0)
	{
		return NULL;
	}

	ce_fs_monitor_object_t *fsm_obj=NULL;
	FSM_LOCK(fsm);
	list_for_each_entry(fsm_obj,&fsm->objs,anchor)
	{
		if((fsm_obj->obj_name.len==obj_name->len)&&(ce_memcmp(fsm_obj->obj_name.data,
									obj_name->data,
									obj_name->len)==0))
		{
			FSM_UNLOCK(fsm);
			return fsm_obj;
		}	
	}
	FSM_UNLOCK(fsm); 
	return NULL;
}

static void 
handle_event(ce_fs_monitor_t *fsm,struct inotify_event *event)
{
	ce_fs_monitor_object_t *fsm_obj=NULL;
	if(fsm==NULL||event==NULL)
	{
		return;
	}
	fsm_obj=get_mon_object_by_fd(fsm,event->wd);
	if(fsm_obj==NULL)
	{
		return;
	}
	if(fsm_obj->event_fun)
	{
		fsm_obj->event_fun(fsm_obj->private_data,
				   fsm_obj->obj_name.data,
				  event->name,
				  event->mask);
	}		
}

#define MAX_EVENT_NUM	64
#define MAX_BUF_SIZE	4096
static void* 
fsm_thread_fun(ce_thread_t *fsm_thread,void *userdata)
{
	ce_fs_monitor_t *fsm=(ce_fs_monitor_t*)userdata;
	ce_pollfd_t *e_pfd=NULL;
       	struct inotify_event *i_event;	
	int num=0;
	int q_size=0;

	u_char i_event_buf[MAX_BUF_SIZE];
	u_char pre_event_name[MAX_BUF_SIZE];
        u_char *offset=NULL;
	int len,tmp_len;
	
	if(fsm==NULL||fsm_thread==NULL)
	{
		return NULL;
	}
	for(;;)
	{
		ce_pollset_poll(fsm->p_set,-1,&num,&e_pfd);
		if(num<=0)
		{
			continue;
		}

		ioctl(e_pfd->desc.f->fd,FIONREAD,&q_size);
		if(q_size<=0)
		{
			continue;
		}

		while((len=ce_read_fd(e_pfd->desc.f->fd,
					i_event_buf,
					MAX_BUF_SIZE)))
		{
			i_event=(struct inotify_event *)i_event_buf;
			offset=i_event_buf;

			while(((u_char*)i_event-i_event_buf)<len)
			{
				if(ce_memcmp(pre_event_name,
					       i_event->name,
					       ce_strlen(i_event->name)))
				{
					handle_event(fsm,i_event);	
				}

				ce_memzero(pre_event_name,MAX_BUF_SIZE);
				ce_memcpy(pre_event_name,
					    i_event->name,
					    ce_strlen(i_event->name));

				tmp_len=sizeof(struct inotify_event)+\
				i_event->len;

				i_event=(struct inotify_event *)\
				(offset+tmp_len);
				offset+=tmp_len;
			}
			if(len<MAX_BUF_SIZE)
			{
				break;
			}
		}//while
	}//for(;;)
}

ce_fs_monitor_t*
ce_create_fs_monitor(ce_pool_t *pool)
{
	if(pool==NULL)
	{
		return NULL;
	}
	ce_int_t rc;
	ce_fs_monitor_t *fsm=(ce_fs_monitor_t*)ce_palloc(pool,
					sizeof(ce_fs_monitor_t));
	if(fsm==NULL)
	{
		return NULL;
	}
	fsm->pool=pool;
	INIT_LIST_HEAD(&fsm->objs);
	fsm->tmp_pool=ce_create_pool(1024);
	if(fsm->tmp_pool==NULL)
	{
		return NULL;
	}
	rc=ce_pollset_create(&fsm->p_set,
				4096,
				pool,
				CE_POLLSET_THREADSAFE);

	if(rc!=CE_OK||fsm->p_set==NULL)
	{

		 return NULL;
	}
	rc=ce_thread_mutex_create(&fsm->lock,0,pool);	
	
	if(rc!=CE_OK||fsm->lock==NULL)
	{

		 return NULL;
	}

	fsm->i_fd=inotify_init();

	if(fsm->i_fd<0)
	{
		 return NULL;
	}
	file_ptr->fd=fsm->i_fd;

	pfd_ptr->p=NULL;
	pfd_ptr->desc_type=CE_POLL_FILE;
	pfd_ptr->reqevents=1;
	pfd_ptr->rtnevents=0;
	pfd_ptr->desc.f=file_ptr;
	pfd_ptr->accepted=0;
	pfd_ptr->read=1;
	pfd_ptr->write=0;
	pfd_ptr->client_data=(void*)fsm;

	/*add to pollset*/
	ce_pollset_add(fsm->p_set,pfd_ptr);
	/*create fsm thread.*/
	rc=ce_thread_create(&fsm->fsm_thread,
				NULL,
				fsm_thread_fun,
				fsm_thread_exit_callback_fun,
				(void*)fsm,
				pool);
	
	if(rc!=CE_OK||fsm->fsm_thread==NULL)
	{

		return NULL;
	}
	return fsm;
}

ce_int_t
ce_register_fs_monitor_object(ce_fs_monitor_t *fsm,
				void *private_data,
			        ce_str_t *obj_name,
				int event_flag,
				mon_event_handler_fun event_fun)
{
	if(fsm==NULL||obj_name==NULL||obj_name->data==NULL||obj_name->len<=0)
	{
		return CE_ERROR;
	}
	if(get_mon_object_by_name(fsm,obj_name))
	{
		return CE_OK;
	}
	FSM_LOCK(fsm);
	ce_fs_monitor_object_t *fsm_obj=(ce_fs_monitor_object_t*)ce_palloc(fsm->tmp_pool,
						sizeof(ce_fs_monitor_object_t));
	fsm_obj->private_data=private_data;
	fsm_obj->event_fun=event_fun;
	fsm_obj->mon_event=event_flag;
	fsm_obj->pool=fsm->tmp_pool;
	fsm_obj->event_mask=0;
	fsm_obj->event=NULL;
	
	fsm_obj->obj_name.data=(u_char*)ce_pstrldup(fsm->tmp_pool,
					   (char*)obj_name->data,
					   obj_name->len);

	fsm_obj->wd=inotify_add_watch(fsm->i_fd,(char*)obj_name->data,event_flag);
	if(fsm_obj->wd<0)
	{
		FSM_UNLOCK(fsm);
		return CE_ERROR;
	}

	list_add_tail(&fsm_obj->anchor,&fsm->objs);
	FSM_UNLOCK(fsm);
	return CE_OK;	
}

ce_int_t
ce_unregister_fs_monitor_object(ce_fs_monitor_t *fsm,
				void *private_data,
			        ce_str_t *obj_name)
{
	private_data=NULL;
	ce_fs_monitor_object_t *fsm_obj=get_mon_object_by_name(fsm,obj_name);
	if(fsm_obj==NULL)
	{
		return CE_OK;
	}
	 FSM_LOCK(fsm); 
	list_del(&fsm_obj->anchor);
	FSM_UNLOCK(fsm);
	return CE_OK;
}

static int fsm_thread_exit_callback_fun(ce_thread_t* fsm_thread,
					void * private_data)
{
	fsm_thread=fsm_thread;
	private_data=private_data;
	return CE_OK;

}

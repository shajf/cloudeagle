/*
 * =====================================================================================
 *
 *       Filename:  ce_fs_monitor.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2012/07/09
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng 
 *   Organization:  ,2012,NTA PROJECT TEAM
 *
 * =====================================================================================
 */
#ifndef CE_FS_MONITOR_H
#define CE_FS_MONITOR_H
#include "ce_basicdefs.h"
#include "ce_list.h"
#include <sys/inotify.h>
#include "ce_thread_proc.h"
#include "ce_poll.h"

#define CE_MON_CREATE 		IN_CREATE  /* Subfile was created.  */
#define CE_MON_DELETE 		IN_DELETE  /* Subfile was deleted.  */
#define CE_MON_MODIFY 		IN_MODIFY  /* File was modified.  */  
#define CE_MON_ACCESS 		IN_ACCESS  /* File was accessed.  */
#define CE_MON_ATTRIB		IN_ATTRIB  /* Metadata changed.  */
#define CE_MON_CLOSE_WRITE	IN_CLOSE_WRITE	/* Writtable file was closed.  */
#define CE_MON_CLOSE_NOWRITE 	IN_CLOSE_NOWRITE /* Unwrittable file closed.  */
#define CE_MON_CLOSE	 (IN_CLOSE_WRITE | IN_CLOSE_NOWRITE) /* Close.  */
#define CE_MON_OPEN		IN_OPEN	/* File was opened.  */
#define CE_MON_MOVED_FROM	IN_MOVED_FROM	 /* File was moved from X.  */
#define CE_MON_MOVED_TO        IN_MOVED_TO      /* File was moved to Y.  */
#define CE_MON_MOVE            (IN_MOVED_FROM | IN_MOVED_TO) /* Moves.  */
#define CE_MON_DELETE_SELF     IN_DELETE_SELF	 /* Self was deleted.  */
#define CE_MON_MOVE_SELF       IN_MOVE_SELF	 /* Self was moved.  */
/* All events which a program can wait on.  */
#define CE_MON_ALL  IN_ALL_EVENTS	 
typedef struct ce_fs_monitor_t ce_fs_monitor_t;
typedef enum
{
	M_FILE,
	M_DIR
}monitor_object_e;


typedef  ce_int_t (*mon_event_handler_fun)(void *private_data,
					     u_char *mon_name,
					     char *fs_name,
					     int event_mask);

typedef struct
{
	struct list_head anchor;
	int object_type;
	int wd;
	ce_pool_t *pool;
	int mon_event;
	struct inotify_event *event;
	int event_mask;
	mon_event_handler_fun event_fun;
	ce_str_t obj_name;
	void *private_data;
}ce_fs_monitor_object_t;

struct ce_fs_monitor_t
{
	ce_pool_t *pool;
	ce_pool_t *tmp_pool;
	ce_thread_mutex_t *lock;
	struct list_head objs;
	ce_pollset_t *p_set;
	int i_fd;
	ce_thread_t *fsm_thread;
};
#define FSM_LOCK(fsm)	ce_thread_mutex_lock(fsm->lock)
#define FSM_UNLOCK(fsm) ce_thread_mutex_unlock(fsm->lock)
#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/

extern ce_fs_monitor_t*
ce_create_fs_monitor(ce_pool_t *pool);

extern ce_int_t
ce_register_fs_monitor_object(ce_fs_monitor_t *fsm,
				void *private_data,
			        ce_str_t *obj_name,
				int event_flag,
				mon_event_handler_fun event_fun);

extern ce_int_t
ce_unregister_fs_monitor_object(ce_fs_monitor_t *fsm,
				void *private_data,
			        ce_str_t *obj_name);
extern ce_int_t
ce_exit_fs_monitor(ce_fs_monitor_t *fsm);
#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_FS_MONITOR_H*/

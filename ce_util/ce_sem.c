/*
 * =====================================================================================
 *
 *       Filename:  ce_sem.c
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

#include "ce_sem.h"

ce_int_t 
ce_sem_get(ce_sem_t *sem)
{

	int sem_id;
	key_t sem_key;
	/*get key*/
	sem_key=ftok((const char*)sem->name.data,sem->proj_id);
    
    if(-1 == sem_key)
	{
		sem->sem_id=-1;
		return CE_ERROR;
	}
	
	sem_id=semget(sem_key,0,0);
	sem->sem_id=sem_id;
	return CE_OK;
}

ce_int_t ce_sem_create(ce_sem_t *sem, unsigned int semnum)
{
	int sem_id;
	key_t sem_key;
	union semun arg;
	struct sembuf	initop;
	int save_errno;
	int cnt = 0;
	
	/*get key*/
	sem_key=ftok((const char*)sem->name.data,sem->proj_id);    
	if(-1 == sem_key)
	{
		return CE_ERROR;
	}
	
	sem_id=semget(sem_key, semnum, IPC_CREAT|IPC_EXCL|0666);    
	if(sem_id != -1)
	{
		for (cnt = 0; cnt < semnum; cnt++)
		{
			/*init sem*/
			arg.val=0;
			if(semctl(sem_id,cnt,SETVAL,arg)<0)
			{
				goto err;
			}
			
			initop.sem_num=cnt;
			initop.sem_op=sem->sem_val<=0?1:sem->sem_val;
			initop.sem_flg=0;

			if(semop(sem_id,&initop,1)<0)
			{
				goto err;
			}
		}
		goto fin;
	}	
	else if(errno!=EEXIST)
	{
		
		goto err;
	}
	else //sem existed,then open it
	{
		sem_id=semget(sem_key,0,0);
		goto fin;
	}

	err:
	save_errno=errno;
	if(sem_id!=-1)
	{
		semctl(sem_id,0,IPC_RMID);
	}
	errno=save_errno;
	return CE_ERROR;
	fin:
	sem->sem_id=sem_id;
	return CE_OK;
}

ce_int_t ce_sem_destroy(ce_sem_t *sem)
{
	#if 0
	if(sem->sem_magic!=SEM_MAGIC)
	{
		return CE_ERROR;
	}
	#endif

	if(semctl(sem->sem_id,0,IPC_RMID)==-1)
	{
		return CE_ERROR;
	}
	return CE_OK;
}

ce_int_t ce_sem_lock(ce_sem_t *sem, unsigned int semno)
{
	struct sembuf op;
	op.sem_num = semno;
	op.sem_op = -1;
	op.sem_flg = 0;

	if(semop(sem->sem_id,&op,1)<0)
	{
		return CE_ERROR;
	}
	return CE_OK;
}

ce_int_t ce_sem_trylock(ce_sem_t *sem, unsigned int semno)
{
	struct sembuf op;
	
	op.sem_num = semno;
	op.sem_op = -1;
	op.sem_flg = IPC_NOWAIT;
	if(semop(sem->sem_id,&op,1)<0)
	{
		return CE_ERROR;
	}

	return CE_OK;
}

ce_int_t ce_sem_unlock(ce_sem_t *sem, unsigned int semno)
{	
	struct sembuf op;

	op.sem_num = semno;
	op.sem_op = 1;
	op.sem_flg = 0;
	if(semop(sem->sem_id,&op,1)<0)
	{
		return CE_ERROR;
	}

	return CE_OK;
}

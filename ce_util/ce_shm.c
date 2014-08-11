/*
 * =====================================================================================
 *
 *       Filename:  ce_shm.c
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

#include "ce_shm.h"

/*del existed share memory*/
static ce_int_t
del_existed_shm(key_t shm_key)
{
	struct shmid_ds buf,*buf_ptr=&buf;
	int id = shmget(shm_key,0,0);

       	if (id == -1) {
	       	return CE_OK;
	}

	memset(buf_ptr,0,sizeof(struct shmid_ds));

	if(shmctl(id,IPC_STAT,buf_ptr)==0)
	{
		if(buf_ptr->shm_nattch==0)
		{
			shmctl(id,IPC_RMID,NULL);
            printf("rm shm id\n");
			return CE_OK;
		}
	}
	printf("share memory has not destroyed..........................\n");
	return CE_ERROR;
}

ce_int_t 
ce_shm_alloc(ce_shm_t *shm)
{
    int  id;
    key_t shm_key;
    /*get key*/
    shm_key=ftok((const char*)shm->name.data,shm->proj_id);
    
    if(shm_key==-1)
    {
	perror("ftok");
	return CE_ERROR;
    }
    if(del_existed_shm(shm_key)==CE_ERROR)
    {
	return CE_ERROR;
    }

    id = shmget(shm_key, shm->size, (SHM_R|SHM_W|IPC_CREAT|IPC_EXCL));

    if (id == -1) {
        perror("shmget error");
        return CE_ERROR;
    }

    shm->start_addr = (char*)shmat(id, NULL, 0);

    if (shm->start_addr == (void *) -1) 
    {
        printf("attach error\n");
	return CE_ERROR;
    }

   shm->shm_id=id;
    return CE_OK;
}


ce_int_t 
ce_shm_get(ce_shm_t *shm)
{
    int  id;
    key_t shm_key;
    /*get key*/
    shm_key=ftok((const char*)shm->name.data,shm->proj_id);
    
    if(shm_key==-1)
    {
		return CE_ERROR;
    }

    id = shmget(shm_key,0,0);
    if (id == -1) {
    	printf("no share memory\n");
        return CE_ERROR;
    }
    
    shm->start_addr = (char*)shmat(id, NULL, 0);

    if (shm->start_addr == (void *) -1) 
    {
	return CE_ERROR;
    }

    shm->shm_id=id;
    return CE_OK;
}


void
ce_shm_free(ce_shm_t *shm)
{
    if (shmdt(shm->start_addr) == -1) 
    {
       //nothing
    }
}

ce_int_t
ce_shm_destroy(ce_shm_t *shm)
{
	key_t shm_key;
    struct shmid_ds buf,*buf_ptr=&buf;
    
	shm_key = ftok((const char*)shm->name.data,shm->proj_id);
	if (-1 == shm_key)
	{
		printf("shm:ftok failed when destroy.\n");
		return CE_ERROR;
	}
	
	int id = shmget(shm_key, 0, 0);
	if (-1 == id)
	{
		return CE_OK;
	}

	if (-1 == shmdt(shm->start_addr))
	{
		printf("fail to dt, %p.\n", shm->start_addr);
	}	
	
	memset(buf_ptr, 0, sizeof(struct shmid_ds));

	if(shmctl(id,IPC_STAT,buf_ptr)==0)
	{
		if(buf_ptr->shm_nattch==0)
		{
			shmctl(id,IPC_RMID,NULL);
			return CE_OK;
		}
		else
		{
			printf("shm:shm_nattch %d, expect 0\n", buf_ptr->shm_nattch);
		}
	}
	else
	{
		printf("shm:shmctl failed, id %d.\n", id);
	}
	
	return CE_ERROR;
}


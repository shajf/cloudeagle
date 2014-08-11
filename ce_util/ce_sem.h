/*
 * =====================================================================================
 *
 *       Filename:  ce_sem.h
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  07/01/2013 13:56:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */

#ifndef CE_SEM_H
#define CE_SEM_H

#include "ce_basicdefs.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include "ce_string.h"
/*brief:define ce_sem_t struct*/
typedef struct
{
	ce_str_t name;
	int proj_id;
	unsigned int sem_val;
	int sem_id;
}ce_sem_t;

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
       /* union semun is defined by including <sysm.h> */
#else
       /* according to X/OPEN we have to define it ourselves */
       union semun {
             int val;              
             struct semid_ds *buf;    
             unsigned short *array;   
                  
             struct seminfo *__buf;   
       };
#endif

#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/

#ifndef SEMMSL
#define SEMMSL  250
#endif

ce_int_t ce_sem_create(ce_sem_t *sem, unsigned int semnum);
ce_int_t ce_sem_get(ce_sem_t *sem);
ce_int_t ce_sem_destroy(ce_sem_t *sem);
ce_int_t ce_sem_lock(ce_sem_t *sem, unsigned int semno);
ce_int_t ce_sem_trylock(ce_sem_t *sem, unsigned int semno);
ce_int_t ce_sem_unlock(ce_sem_t *sem, unsigned int semno);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_SEM_H*/

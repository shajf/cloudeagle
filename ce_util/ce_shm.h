
/*
 * =====================================================================================
 *
 *       Filename:  ce_shm.h
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

#ifndef CE_SHM_H
#define CE_SHM_H
#include "ce_basicdefs.h"
#include "ce_string.h"
/*brief:define waf_shm_t struct*/
typedef struct
{
	char *start_addr;
	size_t size;
	ce_str_t name;
	int shm_id;
	int proj_id;
}ce_shm_t;
#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/
#define CE_USE_SHM_ADDR(shm,addr) shmat((shm)->shm_id,addr,0)
#define CE_UNUSE_SHM_ADDR(addr)   shmdt(addr)

ce_int_t ce_shm_alloc(ce_shm_t *shm);

ce_int_t ce_shm_get(ce_shm_t *shm);
void ce_shm_free(ce_shm_t *shm);


ce_int_t
ce_shm_destroy(ce_shm_t *shm);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_SHM_H*/

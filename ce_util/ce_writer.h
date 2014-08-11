/*
 * =====================================================================================
 *
 *       Filename:  ce_writer.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/04/2012 12:31:06 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization: ,2012,CE PROJECT TEAM 
 *
 * =====================================================================================
 */
#ifndef CE_WRITER_H
#define CE_WRITER_H
#include "ce_basicdefs.h"
#include "ce_palloc.h"

typedef struct ce_writer_t ce_writer_t;

typedef size_t (*write_fun_proxy)(ce_writer_t *writer,u_char *key,
				  size_t key_size,u_char *value,size_t value_size);

typedef int (*write_done_fun_proxy)(ce_writer_t *writer);

typedef size_t (*flush_fun_proxy)(ce_writer_t *writer);


typedef struct {
      	size_t (*write_fun)(u_char *key,size_t key_size,u_char *value,size_t value_size,u_char *private_data);

       	int (*write_done)(u_char *private_data);

       	size_t (*flush_fun)(u_char *private_data);
}ce_writer_ops_t;

struct ce_writer_t
{
	ce_pool_t *pool;
	u_char *private_data;
	size_t writen_n;
	ce_writer_ops_t* writer_ops;
	write_fun_proxy write;
	flush_fun_proxy flush;
	write_done_fun_proxy write_done;
};

#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/
extern ce_writer_t *ce_create_writer(ce_pool_t *pool,
					ce_writer_ops_t *w_ops,
					u_char *private_data);
#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_WRITER_H*/

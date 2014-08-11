/*
 * =====================================================================================
 *
 *       Filename:  ce_buf.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  07/01/2013 02:05:10 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng 
 *   Organization: nsfocus,2012,ce project team
 *
 * =====================================================================================
 */
#ifndef CE_BUF_H
#define CE_BUF_H
#include "ce_basicdefs.h"

typedef struct ce_buf_t ce_buf_t;
#pragma pack(push,1)
struct ce_buf_t
{
	u_char *start;
	u_char *pos;
	u_char *last;
	u_char *end;
};
#pragma pack(pop)


#define CE_INCR_BUF_POS(ls_buf_ptr,kv_size)   ((ls_buf_ptr)->pos+=(kv_size))
#define CE_BUF_EMPTY(ls_buf_ptr) ((ls_buf_ptr)->pos==(ls_buf_ptr)->start)
#define CE_BUF_REMAIN_SIZE(buf)  ((buf)->end-(buf)->pos)
#define CE_BUF_POS(buf) ((buf)->pos)			
#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/

extern ce_buf_t *
ce_create_buf(size_t buf_size);

extern ce_buf_t*
ce_init_buf(u_char *mem,size_t mem_size);

extern void
ce_destroy_buf(ce_buf_t *buf);

extern u_char *
ce_push_buf(ce_buf_t *buf,size_t size);

extern int
ce_buf_full(ce_buf_t *buf,size_t size);

extern size_t
ce_buf_size(ce_buf_t *buf);

extern size_t
ce_buf_content_size(ce_buf_t *buf);

extern void
ce_reset_buf(ce_buf_t *buf);

extern u_char*
ce_buf_content(ce_buf_t *buf);
#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_BUF_H*/

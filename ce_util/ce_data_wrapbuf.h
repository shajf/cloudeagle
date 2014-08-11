/*
 * =====================================================================================
 *
 *       Filename:  ce_data_wrapbuf.h
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  08/01/2013 12:04:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */
#ifndef CE_DATA_WRAPBUF_H
#define CE_DATA_WRAPBUF_H
#include "ce_basicdefs.h"

typedef struct ce_data_wrapbuf_t ce_data_wrapbuf_t;

struct ce_data_wrapbuf_t
{
	ce_data_wrapbuf_t* (*expand)(ce_data_wrapbuf_t *old_buf,
					    void *user_data);
	char *start;
	char *pos;
	char *end;
};

#define CE_DATA_GET_BUF_POS(b) ((b)->pos)
#define CE_DATA_SET_BUF_POS(b,p) ((b)->pos=(p))
#define CE_DATA_GET_BUF_START(b) ((b)->start)
#define CE_DATA_SET_BUF_START(b,s) ((b)->start=(s))
#define CE_DATA_GET_BUF_END(b)  ((b)->end)
#define CE_DATA_SET_BUF_END(b,e)  ((b)->end=(e))
#define CE_DATA_GET_BUF_SIZE(b) ((b)->end-(b)->start)
#define CE_DATA_GET_BUF_UNUSED_SIZE(b) ((b)->end-(b)->pos)
#define CE_DATA_GET_BUF_USED_SIZE(b) ((b)->pos-(b)->start)
#define CE_DATA_UPDATE_BUF_POS(b,sz) ((b)->pos+=(sz))
#define CE_DATA_RESET_BUF(b) ((b)->pos=(b)->start)
#define CE_DATA_EXPAND_BUF(b,user_data) ((b)->expand(b,user_data))
#define CE_DATA_GET_CONTENT(b) ((b)->start)
#define CE_DATA_GET_CONTENT_SIZE(b) ((b)->pos-(b)->start)
#define CE_DATA_IS_FULL(b,size) ((b)->pos+(size)>(b)->end)

#endif /*CE_DATA_WRAPBUF_H*/

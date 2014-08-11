/*
 *Authors: shajianfeng<csp001314@163.com>
 * */
#ifndef _CSP_KERNEL_RECORD_H
#define _CSP_KERNEL_RECORD_H

#include <csp_kernel.h>

typedef unsigned char byte_t;
struct csp_wrap_buf
{
	void *start;
	void *pos;
	void *end;
};

struct csp_msg_hdr
{
	__u8 start_mark;
	__u16 record_size;
};
#define SET_MSG_HDR(pos) ((struct csp_msg_hdr*)(pos))
#define RECORD_START_MARK 0XFD
struct csp_record
{
	struct csp_wrap_buf buf;
	struct csp_msg_hdr *cur_msg_hdr;
	void *private_data;
	__u8 start_mark;
	__u16 record_size;
};

#define BUF_FULL(buf,sz) 	((buf)->pos+(sz)>(buf)->end)
#define BUF_FIRST(buf)    	((buf)->start)
#define BUF_POS(buf) 	 	((buf)->pos)
#define BUF_LAST(buf)   	((buf)->end)
#define BUF_POS_UPDATE(buf,sz)  ((buf)->pos+=(sz))
extern void init_record(struct csp_record *record,void *start,__u16 len,void *private_data);
/*declare record append interfaces*/
extern int record_append_start(struct csp_record *record);
extern int record_append_end(struct csp_record *record);
extern int record_append_bool(struct csp_record *record,int v);
extern int record_append_short(struct csp_record *record,short v);
extern int record_append_int(struct csp_record *record,int v);
extern int record_append_long(struct csp_record *record,long v);
extern int record_append_ushort(struct csp_record *record,u_short v);
extern int record_append_uint(struct csp_record *record,u_int v);
extern int record_append_ulong(struct csp_record *record,u_long v);
extern int record_append_float(struct csp_record *record,float v);
extern int record_append_double(struct csp_record *record,double v);
extern int record_append_string(struct csp_record *record,const char *v);
extern int record_append_int8(struct csp_record *record,__s8 v);
extern int record_append_int16(struct csp_record *record,__s16 v);
extern int record_append_int32(struct csp_record *record,__s32 v);
extern int record_append_int64(struct csp_record *record,__s64 v);

extern int record_append_uint8(struct csp_record *record,__u8 v);
extern int record_append_uint16(struct csp_record *record,__u16 v);
extern int record_append_uint32(struct csp_record *record,__u32 v);
extern int record_append_uint64(struct csp_record *record,__u64 v);
extern void* record_append_struct(struct csp_record *record,size_t sz);
/*declare record get interfaces*/
extern int record_get_start(struct csp_record *record);
extern int record_get_end(struct csp_record *record);
extern int record_get_bool(struct csp_record *record,int *v);
extern int record_get_short(struct csp_record *record,short *v);
extern int record_get_int(struct csp_record *record,int *v);
extern int record_get_long(struct csp_record *record,long *v);
extern int record_get_ushort(struct csp_record *record,u_short *v);
extern int record_get_uint(struct csp_record *record,u_int *v);
extern int record_get_ulong(struct csp_record *record,u_long *v);
extern int record_get_float(struct csp_record *record,float *v);
extern int record_get_double(struct csp_record *record,double *v);
extern int record_get_string(struct csp_record *record,char **v,size_t *sz);
extern int record_get_string_copy(struct csp_record *record,char* buf,size_t buf_size);
extern int record_get_int8(struct csp_record *record,__s8 *v);
extern int record_get_int16(struct csp_record *record,__s16 *v);
extern int record_get_int32(struct csp_record *record,__s32 *v);
extern int record_get_int64(struct csp_record *record,__s64 *v);

extern int record_get_uint8(struct csp_record *record,__u8 *v);
extern int record_get_uint16(struct csp_record *record,__u16 *v);
extern int record_get_uint32(struct csp_record *record,__u32 *v);
extern int record_get_uint64(struct csp_record *record,__u64 *v);
extern int record_get_struct(struct csp_record *record,void **struct_ptr,size_t *sz);
#endif /*_CSP_KERNEL_RECORD_H*/

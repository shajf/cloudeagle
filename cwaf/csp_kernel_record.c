/*
 *Authors: shajianfeng <csp001314@163.com>
 * */
#include <csp_kernel.h>

#define VALUE_APPEND(buf,value,byte_num) 			\
	do{							\
		size_t bytes,i;					\
		bytes = (byte_num)*8;				\
		for(i=1;i<=(byte_num);i++)			\
		{						\
			buf[i-1]=(value)>>(bytes-i*8)&0xff;	\
		}						\
	}while(0)

#define APPEND_CHECK(record,sz) 				\
	do{							\
		if(!record) 					\
			return -1;				\
		if(BUF_FULL(&record->buf,sz)) 			\
			return -1;				\
	}while(0)


void init_record(struct csp_record *record,void *start,__u16 len,void *private_data)
{
	struct csp_wrap_buf *buf;
	#ifdef __KERNEL__
	BUG_ON(!record||!start||len<=0);
	#else
	assert(record&&start&&len>0);
	#endif
	buf = &record->buf;

	buf->start = start;
	buf->pos = start;
	buf->end = start+len;

	record->private_data = private_data;
	record->cur_msg_hdr = NULL;
	record->start_mark = RECORD_START_MARK;
	record->record_size = 0;

}

static inline int 
append_bytes_buf(struct csp_record *record,byte_t *bytes,size_t len)
{
	struct csp_wrap_buf *buf = &record->buf;

	void *pos = buf->pos;

	memcpy(pos,(void*)bytes,len);
	BUF_POS_UPDATE(buf,len);
	record->record_size +=len;
	return 0;
}

int record_append_start(struct csp_record *record)
{
	struct csp_msg_hdr * msg_hdr;
	struct csp_wrap_buf *buf;
	APPEND_CHECK(record,sizeof(struct csp_msg_hdr));
	buf = &record->buf;
	msg_hdr = SET_MSG_HDR(buf->pos);
	msg_hdr->start_mark = record->start_mark;
	msg_hdr->record_size =0;
	record->record_size = sizeof(struct csp_msg_hdr);
	record->cur_msg_hdr = msg_hdr;
	BUF_POS_UPDATE(buf,sizeof(struct csp_msg_hdr));
	
	return 0;
}

int record_append_end(struct csp_record *record)
{
	struct csp_msg_hdr *msg_hdr = record->cur_msg_hdr;
	msg_hdr->record_size = record->record_size;
	record->cur_msg_hdr = NULL;

	return 0;
}

int record_append_bool(struct csp_record *record,int v)
{

	byte_t barr[1] ={0};
	APPEND_CHECK(record,1);
	
	VALUE_APPEND(barr,v,1);
	
	return append_bytes_buf(record,barr,1);
}

int record_append_short(struct csp_record *record,short v)
{
	size_t sz = sizeof(short);

	byte_t barr[sizeof(short)]={0};

	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

int record_append_int(struct csp_record *record,int v)
{

	size_t sz = sizeof(int);

	byte_t barr[sizeof(int)]={0};
	
	APPEND_CHECK(record,sz);
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

int record_append_long(struct csp_record *record,long v)
{

	size_t sz = sizeof(long);
	
	byte_t barr[sizeof(long)]={0};
	
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

int record_append_ushort(struct csp_record *record,u_short v)
{

	size_t sz = sizeof(u_short);
	byte_t barr[sizeof(u_short)]={0};
	
	APPEND_CHECK(record,sz);
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);

}

int record_append_uint(struct csp_record *record,u_int v)
{

	size_t sz = sizeof(u_int);
	
	byte_t barr[sizeof(u_int)] = {0};
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

int record_append_ulong(struct csp_record *record,u_long v)
{
	
	size_t sz = sizeof(u_long);
	
	byte_t barr[sizeof(u_long)] = {0};
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

#define F_EXP_BIT_MASK	2139095040
#define F_SIGNIF_BIT_MASK  8388607

static inline int 
float2_raw_int_bits(float v)
{
	
	int result;
	union{float f;int i;}u;
	u.f=v;
	
	result = u.i;

	if (((result &F_EXP_BIT_MASK) ==F_EXP_BIT_MASK)\
	&&(result & F_SIGNIF_BIT_MASK) != 0)
	     	result = 0x7fc00000;

	return result;

}

#define D_EXP_BIT_MASK	9218868437227405312LL
#define D_SIGNIF_BIT_MASK  4503599627370495LL

static inline __s64 
double2_raw_int_bits(double v)
{
	
	__s64 result;
	union{double d;__s64 i;}u;
	u.d=v;
	
	result = u.i;

	if (((result &D_EXP_BIT_MASK) ==D_EXP_BIT_MASK)\
	&&(result & D_SIGNIF_BIT_MASK) != 0LL)
	     	result = 0x7ff8000000000000LL;

	return result;
}

int record_append_float(struct csp_record *record,float v)
{
	return record_append_int(record,float2_raw_int_bits(v));
}

int record_append_double(struct csp_record *record,double v)
{
	return record_append_int64(record,double2_raw_int_bits(v));
}

int record_append_string(struct csp_record *record,const char *v)
{
	size_t len;
	len = strlen(v);
	APPEND_CHECK(record,len+1);
	return append_bytes_buf(record,(byte_t*)v,len+1); 
}

int record_append_int8(struct csp_record *record,__s8 v)
{
	
	size_t sz = sizeof(__s8);
	
	byte_t barr[sizeof(__s8)] = {0};
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

int record_append_int16(struct csp_record *record,__s16 v)
{
	
	size_t sz =  sizeof(__s16);
	
	byte_t barr[sizeof(__s16)] = {0};
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

int record_append_int32(struct csp_record *record,__s32 v)
{

	size_t sz =  sizeof(__s32);
	
	byte_t barr[sizeof(__s32)] = {0};
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);

}

int record_append_int64(struct csp_record *record,__s64 v)
{

	
	size_t sz = sizeof(__s64);

	byte_t barr[sizeof(__s64)] = {0};
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

int record_append_uint8(struct csp_record *record,__u8 v)
{
	
	size_t sz = sizeof(__u8);

	byte_t barr[sizeof(__u8)] = {0};
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);

}

int record_append_uint16(struct csp_record *record,__u16 v)
{
	size_t sz = sizeof(__u16);

	byte_t barr[sizeof(__u16)] = {0};
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

int record_append_uint32(struct csp_record *record,__u32 v)
{
	size_t sz =  sizeof(__u32);

	byte_t barr[sizeof(__u32)] = {0};
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

int record_append_uint64(struct csp_record *record,__u64 v)
{

	size_t sz = sizeof(__u64);

	byte_t barr[sizeof(__u64)] = {0};
	APPEND_CHECK(record,sz);
	
	VALUE_APPEND(barr,v,sz);
	
	return append_bytes_buf(record,barr,sz);
}

void* record_append_struct(struct csp_record *record,size_t size)
{
	struct csp_wrap_buf *buf;
	void *res;
	size_t sz;
	
	buf = &record->buf;
	sz = size+sizeof(__u32);

	if(!record||BUF_FULL(buf,sz))
	{
		return NULL;
	}
	record_append_uint32(record,size);
	res = buf->pos;
	BUF_POS_UPDATE(buf,size);
	record->record_size +=sz;

	return res;
}

#define GET_CHECK(record,sz) 							\
	do{									\
		if(!record) 							\
			return -1; 						\
		if(record->record_size+sz>record->cur_msg_hdr->record_size)	\
			return -1;						\
		if(record->buf.pos+sz>record->buf.end)				\
			return -1;						\
		}while(0)

#define VALUE_GET(buf, value, byte_num)		 	 \
do{	     						 \
	size_t bytes,i;  				 \
	void *buf_p = buf;				 \
	byte_t b; 					 \
	bytes = (byte_num)*8;			         \
	for (i=1; i<=byte_num; i++)			 \
	{						 \
		b=*((byte_t*)buf_p);			 \
		buf_p+=1;			         \
		(value) += (b<<((bytes-i*8) &0xFF));	 \
	}						 \
} while(0)

int record_get_start(struct csp_record *record)
{
	
	struct csp_wrap_buf *buf;
	struct csp_msg_hdr *msg_hdr;	
	if(!record)
		return -1;
	
	buf = &record->buf;

	msg_hdr = SET_MSG_HDR(buf->pos);

	if(record->start_mark!=msg_hdr->start_mark||msg_hdr->record_size<=0)
	{
		return -1;
	}
	
	record->cur_msg_hdr = msg_hdr;
	record->record_size = sizeof(struct csp_msg_hdr);

	BUF_POS_UPDATE(buf,record->record_size);

	return 0;
}

int record_get_end(struct csp_record *record)
{
	if(!record||record->record_size!=record->cur_msg_hdr->record_size)
		return -1;
		
	record->record_size = 0;
	record->cur_msg_hdr = NULL;

	return 0;
}

static int __record_get_skip_bytes(struct csp_record *record,size_t r_size)
{
	struct csp_wrap_buf *buf = &record->buf;
	BUF_POS_UPDATE(buf,r_size);
	record->record_size +=r_size;
	return 0;
}

int record_get_skip_bytes(struct csp_record *record,size_t size)
{
	GET_CHECK(record,size);

	return __record_get_skip_bytes(record,size);
}

#define  GET_RECORD_POS(record) ((record)->buf.pos)

int record_get_bool(struct csp_record *record,int *v)
{
	
	int vv = 0;
	GET_CHECK(record,1);
	VALUE_GET(GET_RECORD_POS(record),vv,1);
	*v = vv;
	return __record_get_skip_bytes(record,1); 
	
}

int record_get_short(struct csp_record *record,short *v)
{
	
	size_t sz = sizeof(short);
	short  vv = 0;
	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_int(struct csp_record *record,int *v)
{
	
	size_t sz = sizeof(int);

	int  vv = 0;
	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_long(struct csp_record *record,long *v)
{
  
	size_t sz = sizeof(long);
	long  vv = 0;

	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_ushort(struct csp_record *record,u_short *v)
{
  
	size_t sz = sizeof(u_short);

	u_short vv = 0;
	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 

}

int record_get_uint(struct csp_record *record,u_int *v)
{
  
	size_t sz = sizeof(u_int);
	u_int vv = 0;

	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_ulong(struct csp_record *record,u_long *v)
{
 
	size_t sz = sizeof(u_long);
	u_long vv = 0;

	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_float(struct csp_record *record,float *v)
{
	union {float f;int i;}u;
	
	if(record_get_int(record,&u.i))
	{
		return -1;
	}

	*v = u.f;
	
	return 0;
}

int record_get_double(struct csp_record *record,double *v)
{
	union{double d;__s64 i;}u;

	if(record_get_int64(record,&u.i))
	{
		return -1;
	}

	*v = u.d;

	return 0;
}

int record_get_string(struct csp_record *record,char **v,size_t *sz)
{
	*v = GET_RECORD_POS(record);
	*sz = strlen(*v);

	GET_CHECK(record,*sz);

	return __record_get_skip_bytes(record,*sz);
}

int record_get_string_copy(struct csp_record *record,char* buf,size_t buf_size)
{
	char *v;
	size_t len;
	memset(buf,0,buf_size);

	if(record_get_string(record,&v,&len))
	{
		return -1;
	}

	if(buf_size<len+1)
	{
		return -1;
	}
	memcpy(buf,v,len);
	return 0;
}

int record_get_int8(struct csp_record *record,__s8 *v)
{
	
	size_t sz = sizeof(__s8);

	__s8 vv = 0;
	
	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_int16(struct csp_record *record,__s16 *v)
{

	size_t sz = sizeof(__s16);

	__s16 vv = 0;
	
	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_int32(struct csp_record *record,__s32 *v)
{

	size_t sz = sizeof(__s32);
	__s32 vv = 0;

	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_int64(struct csp_record *record,__s64 *v)
{

	size_t sz = sizeof(__s64);

	__s64 vv = 0;
	
	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_uint8(struct csp_record *record,__u8 *v)
{
	
	size_t sz = sizeof(__u8);

	__u8 vv = 0;
	
	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_uint16(struct csp_record *record,__u16 *v)
{
         	
	size_t sz = sizeof(__u16);
	__u16 vv = 0;
	
	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_uint32(struct csp_record *record,__u32 *v)
{

	size_t sz = sizeof(__u32);

	__u32 vv = 0;
	
	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_uint64(struct csp_record *record,__u64 *v)
{

	size_t sz = sizeof(__u64);

	__u64 vv = 0;
	
	GET_CHECK(record,sz);
	VALUE_GET(GET_RECORD_POS(record),vv,sz);
	*v = vv;
	return __record_get_skip_bytes(record,sz); 
}

int record_get_struct(struct csp_record *record,void **struct_ptr,size_t *sz)
{
	__u32 struct_size;
	*struct_ptr =NULL;
	*sz = 0;

	if(record_get_uint32(record,&struct_size))
	{
		return -1;		
	}
	GET_CHECK(record,struct_size);
	*struct_ptr = GET_RECORD_POS(record);
	*sz = struct_size;
	return __record_get_skip_bytes(record,struct_size);
}


/*
 * =====================================================================================
 *
 *       Filename:  ce_data_writer_encoding.c
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  08/01/2013 11:05:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */
#include "ce_data_writer.h"

#define  VALUE_WRITE(buf, value, byte_num)				\
	do {								\
		size_t bytes=(byte_num)*8;				\
		size_t i = 1;						\
		for(i=1;i<=(byte_num);i++)				\
		{							\
			buf[i-1]=(value)>>(bytes-i*8)&0xff;	        \
		}							\
	} while(0)

size_t
write_bytes2_buf(ce_data_writer_t *writer,ce_byte_t *bytes,size_t len)
{
	void *pos=CE_DATA_GET_BUF_POS(&writer->buf);
	assert(pos);

	ce_memcpy(pos,(void*)bytes,len);
	CE_DATA_UPDATE_BUF_POS(&writer->buf,len);

	return len;
}

size_t 
writer_write(ce_data_writer_t *writer,int v)
{
       ce_byte_t b_arr[1]={0};
       VALUE_WRITE(b_arr,v,1);
       return write_bytes2_buf(writer,b_arr,1);
}

size_t 
writer_write_bytes(ce_data_writer_t *writer,ce_byte_t bytes[],int len,int offset)
 {
	
	ce_byte_t *p=bytes+offset;
	return write_bytes2_buf(writer,p,len);
 }

 size_t 
 writer_write_wchar(ce_data_writer_t *writer,int v)
 {
       ce_byte_t b_arr[2]={0};
       VALUE_WRITE(b_arr,v,2);
       return write_bytes2_buf(writer,b_arr,2);
 }
 
 size_t 
 writer_write_boolean(ce_data_writer_t *writer,int v)
 {

	int bool_v=v?1:0;
	return writer_write(writer,bool_v);
 }

 size_t 
 writer_write_short(ce_data_writer_t *writer,int v)
 {
       	
       ce_byte_t b_arr[2]={0};
       VALUE_WRITE(b_arr,v,2);
       return write_bytes2_buf(writer,b_arr,2);
 }

 size_t 
 writer_write_int(ce_data_writer_t *writer,int v)
 { 
       ce_byte_t b_arr[4]={0};
       VALUE_WRITE(b_arr,v,4);
       return write_bytes2_buf(writer,b_arr,4);
 }
 
size_t 
writer_write_long(ce_data_writer_t *writer,long v)
{
	size_t b_num=sizeof(long);
	ce_byte_t b_arr[sizeof(long)]={0};
	VALUE_WRITE(b_arr,v,b_num);
	return write_bytes2_buf(writer,b_arr,b_num);
}
 
size_t 
writer_write_ushort(ce_data_writer_t *writer,unsigned short v)
{	
       ce_byte_t b_arr[2]={0};
       VALUE_WRITE(b_arr,v,2);
       return write_bytes2_buf(writer,b_arr,2);
}

size_t 
writer_write_uint(ce_data_writer_t *writer,unsigned int v)
{	
       ce_byte_t b_arr[4]={0};
       VALUE_WRITE(b_arr,v,4);
       return write_bytes2_buf(writer,b_arr,4);
}

size_t 
writer_write_ulong(ce_data_writer_t *writer,unsigned long v)
{
 
	size_t b_num=sizeof(unsigned long);
	ce_byte_t b_arr[sizeof(unsigned long)]={0};
	VALUE_WRITE(b_arr,v,b_num);
	return write_bytes2_buf(writer,b_arr,b_num);
 }
 
#define F_EXP_BIT_MASK	2139095040
#define F_SIGNIF_BIT_MASK  8388607

static inline int 
float2_raw_int_bits(float v)
{
	union{float f;int i;}u;
	u.f=v;
	int result=u.i;

	if (((result &F_EXP_BIT_MASK) ==F_EXP_BIT_MASK)\
	&&(result & F_SIGNIF_BIT_MASK) != 0)
	     	result = 0x7fc00000;

	return result;

}

#define D_EXP_BIT_MASK	9218868437227405312LL
#define D_SIGNIF_BIT_MASK  4503599627370495LL

static inline uint64_t 
double2_raw_int_bits(double v)
{
	union{double d;uint64_t i;}u;
	u.d=v;
	uint64_t result=u.i;

	if (((result &D_EXP_BIT_MASK) ==D_EXP_BIT_MASK)\
	&&(result & D_SIGNIF_BIT_MASK) != 0LL)
	     	result = 0x7ff8000000000000LL;

	return result;
}

size_t 
writer_write_chars(ce_data_writer_t *writer,char *v)
{
	size_t len=ce_strlen(v);
	
	return write_bytes2_buf(writer,(ce_byte_t *)v,len+1);
}

size_t 
writer_write_int8(ce_data_writer_t *writer,int8_t v)
{
       ce_byte_t b_arr[1]={0};
       VALUE_WRITE(b_arr,v,1);
       return write_bytes2_buf(writer,b_arr,1);
}

size_t 
writer_write_int16(ce_data_writer_t *writer,int16_t v)
{
       ce_byte_t b_arr[2]={0};
       VALUE_WRITE(b_arr,v,2);
       return write_bytes2_buf(writer,b_arr,2);
}

size_t 
writer_write_int32(ce_data_writer_t *writer,int32_t v)
{
       ce_byte_t b_arr[4]={0};
       VALUE_WRITE(b_arr,v,4);
       return write_bytes2_buf(writer,b_arr,4);
}

size_t 
writer_write_int64(ce_data_writer_t *writer,int64_t v)
{
       ce_byte_t b_arr[8]={0};
       VALUE_WRITE(b_arr,v,8);
       return write_bytes2_buf(writer,b_arr,8);
}

size_t
writer_write_uint8(ce_data_writer_t *writer,uint8_t v)
{
       ce_byte_t b_arr[1]={0};
       VALUE_WRITE(b_arr,v,1);
       return write_bytes2_buf(writer,b_arr,1);
}

size_t 
writer_write_uint16(ce_data_writer_t *writer,uint16_t v)
{
       ce_byte_t b_arr[2]={0};
       VALUE_WRITE(b_arr,v,2);
       return write_bytes2_buf(writer,b_arr,2);
}

size_t 
writer_write_uint32(ce_data_writer_t *writer,uint32_t v)
{
       ce_byte_t b_arr[4]={0};
       VALUE_WRITE(b_arr,v,4);
       return write_bytes2_buf(writer,b_arr,4);
}

size_t 
writer_write_uint64(ce_data_writer_t *writer,uint64_t v)
{ 
       ce_byte_t b_arr[8]={0};
       VALUE_WRITE(b_arr,v,8);
       return write_bytes2_buf(writer,b_arr,8);
}

size_t 
writer_write_float(ce_data_writer_t *writer,float v)
{ 
	return writer_write_int(writer,float2_raw_int_bits(v));
}

size_t 
writer_write_double(ce_data_writer_t *writer,double v)
{
	return writer_write_int64(writer,double2_raw_int_bits(v));
}

size_t 
writer_write_string(ce_data_writer_t *writer,ce_str_t *v)
{
	writer_write_uint32(writer,(uint32_t)v->len);
	write_bytes2_buf(writer,v->data,v->len);

	return (4+v->len);
}

ce_data_writer_ops_t ce_writer_encoding=
{
	.ce_write_fn = writer_write,
	.ce_write_bytes_fn = writer_write_bytes,
	.ce_write_wchar_fn = writer_write_wchar,
	.ce_write_boolean_fn = writer_write_boolean, 
	.ce_write_short_fn = writer_write_short,
	.ce_write_int_fn = writer_write_int,
	.ce_write_long_fn = writer_write_long,
	.ce_write_ushort_fn = writer_write_ushort,
	.ce_write_uint_fn = writer_write_uint,
	.ce_write_ulong_fn = writer_write_ulong,
	.ce_write_float_fn = writer_write_float,
	.ce_write_double_fn = writer_write_double,
	.ce_write_string_fn = writer_write_string,
	.ce_write_chars_fn = writer_write_chars,
	.ce_write_int8_fn = writer_write_int8,
	.ce_write_int16_fn = writer_write_int16,
	.ce_write_int32_fn = writer_write_int32,
	.ce_write_int64_fn = writer_write_int64,
	.ce_write_uint8_fn = writer_write_uint8,
	.ce_write_uint16_fn = writer_write_uint16,
	.ce_write_uint32_fn = writer_write_uint32,
	.ce_write_uint64_fn = writer_write_uint64
};


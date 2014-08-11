/*
 * =====================================================================================
 *
 *       Filename:  ce_data_reader_encoding.c
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
#include "ce_data_reader.h"

#define VALUE_READ(buf, value, byte_num)		 \
do{	     						 \
	size_t i=1;					 \
	size_t bytes=(byte_num)*8;			 \
	ce_byte_t b;					 \
	for (i=1; i<=byte_num; i++)			 \
	{						 \
		b=*((ce_byte_t*)buf);			 \
		buf+=1;					 \
		(value) += (b<<((bytes-i*8) &0xFF));	 \
	}						 \
} while(0)


#define GET_READER_POS(reader) ((reader)->buf.pos)

size_t 
reader_read_skip_bytes(ce_data_reader_t *reader,size_t len)
{
	size_t r_size=len;
	if(CE_DATA_IS_FULL(&reader->buf,len))
	{
		r_size=CE_DATA_GET_BUF_UNUSED_SIZE(&reader->buf);	
	}
	
	CE_DATA_UPDATE_BUF_POS(&reader->buf,r_size);

	return r_size;
}

static ce_int_t 
reader_read(ce_data_reader_t *reader,int *v)
{
	assert(reader&&v);
	int vv=0;
    char  *pos=(char *)GET_READER_POS(reader);
	VALUE_READ(pos,vv,1);
	*v=vv;
	reader_read_skip_bytes(reader,1);
	return CE_OK;
}

static ce_int_t
reader_read_boolean(ce_data_reader_t *reader,int *v)
{
	assert(reader&&v);
	int vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,1);
	*v=vv;
	reader_read_skip_bytes(reader,1);
	return CE_OK;
}

static ce_int_t 
reader_read_short(ce_data_reader_t *reader,short *v)
{
	assert(reader&&v);
	short vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,2);
	*v=vv;
	reader_read_skip_bytes(reader,2);  
	return CE_OK;
}

static ce_int_t 
reader_read_int(ce_data_reader_t *reader,int *v)
{
	assert(reader&&v);
	int vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,4);
	*v=vv;
	reader_read_skip_bytes(reader,4);  
	return CE_OK;
}

static ce_int_t 
reader_read_long(ce_data_reader_t *reader,long *v)
{
	assert(reader&&v);
	size_t b_num=sizeof(long);
	long vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,b_num);
	*v=vv;
	reader_read_skip_bytes(reader,b_num);  
	return CE_OK;
}

static ce_int_t 
reader_read_ushort(ce_data_reader_t *reader,unsigned short *v)
{
	
	assert(reader&&v);
	unsigned short vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,2);
	*v=vv;
	reader_read_skip_bytes(reader,2);  
	return CE_OK;
}

static ce_int_t 
reader_read_uint(ce_data_reader_t *reader,unsigned int *v)
{

	assert(reader&&v);
	unsigned int vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,4);
	*v=vv;
	reader_read_skip_bytes(reader,4);  
	return CE_OK;
}

static ce_int_t
reader_read_ulong(ce_data_reader_t *reader,unsigned long *v)
{

	assert(reader&&v);
	size_t b_num=sizeof(unsigned long);
	unsigned long vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,b_num);
	*v=vv;
	reader_read_skip_bytes(reader,b_num);  
	return CE_OK;
}

static ce_int_t 
reader_read_float(ce_data_reader_t *reader,float *v)
{
	
	assert(reader&&v);
	union{float f;int i;}u;

	if(reader_read_int(reader,&u.i)==CE_ERROR)
		return CE_ERROR;
	
	*v=u.f;
	return CE_OK;
}

static ce_int_t 
reader_read_int64(ce_data_reader_t *reader,int64_t *v)
{

	assert(reader&&v);
	int64_t vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,8);
	*v=vv;
	reader_read_skip_bytes(reader,8);  
	return CE_OK;
}

static ce_int_t 
reader_read_double(ce_data_reader_t *reader,double *v)
{
	assert(reader&&v);
	union{double d;int64_t i;}u;
	if(reader_read_int64(reader,&u.i)==CE_ERROR)
		return CE_ERROR;
	*v=u.d;
	return CE_OK;
}

static ce_int_t 
reader_read_uint32(ce_data_reader_t *reader,uint32_t *v)
{	
	assert(reader&&v);
	uint32_t vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,4);
	*v=vv;
	reader_read_skip_bytes(reader,4);  
	return CE_OK;
}

static ce_int_t 
reader_read_string(ce_data_reader_t *reader,ce_str_t *v)
{
	assert(reader&&v);
	/*read string len*/
	uint32_t len;
	if(reader_read_uint32(reader,&len)==CE_ERROR)
		return CE_ERROR;
	
	v->len=(size_t)len;
	v->data=(u_char*)GET_READER_POS(reader);
	reader_read_skip_bytes(reader,v->len);

	return CE_OK;
}

static ce_int_t
reader_read_chars(ce_data_reader_t *reader,char *buf,size_t buf_size)
{
	assert(reader&&buf&&buf_size>0);
	char *pos=(char*)GET_READER_POS(reader);
	size_t len=strlen(pos)+1;
	if(len>buf_size)
		len=buf_size;

	memcpy(buf,pos,len);
	reader_read_skip_bytes(reader,len);
	return len;
}

static ce_int_t 
reader_read_int8(ce_data_reader_t *reader,int8_t *v)
{
	assert(reader&&v);
	int8_t vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,1);
	*v=vv;
	reader_read_skip_bytes(reader,1);
	return CE_OK;
}

static ce_int_t 
reader_read_int16(ce_data_reader_t *reader,int16_t *v)
{
	assert(reader&&v);
	int16_t vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,2);
	*v=vv;
	reader_read_skip_bytes(reader,2);  
	return CE_OK;
}

static ce_int_t 
reader_read_int32(ce_data_reader_t *reader,int32_t *v)
{
	assert(reader&&v);
	int32_t vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,4);
	*v=vv;
	reader_read_skip_bytes(reader,4);  
	return CE_OK;
}

static ce_int_t 
reader_read_uint8(ce_data_reader_t *reader,uint8_t *v)
{
	assert(reader&&v);
	uint8_t vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,1);
	*v=vv;
	reader_read_skip_bytes(reader,1);
	return CE_OK;
}

static ce_int_t 
reader_read_uint16(ce_data_reader_t *reader,uint16_t *v)
{ 
	assert(reader&&v);
	uint16_t vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,2);
	*v=vv;
	reader_read_skip_bytes(reader,2);  
	return CE_OK;
}


static ce_int_t 
reader_read_uint64(ce_data_reader_t *reader,uint64_t *v)
{
	assert(reader&&v);
	uint64_t vv=0;
	char *pos=GET_READER_POS(reader);
	VALUE_READ(pos,vv,8);
	*v=vv;
	reader_read_skip_bytes(reader,8);  
	return CE_OK;
}

ce_data_reader_ops_t ce_reader_encoding={
	.ce_read_fn = reader_read,
	.ce_read_boolean_fn = reader_read_boolean,
	.ce_read_short_fn = reader_read_short,
	.ce_read_int_fn = reader_read_int,
	.ce_read_long_fn = reader_read_long,
	.ce_read_ushort_fn = reader_read_ushort,
	.ce_read_uint_fn = reader_read_uint,
	.ce_read_ulong_fn = reader_read_ulong, 
	.ce_read_float_fn = reader_read_float,
	.ce_read_double_fn = reader_read_double,
	.ce_read_string_fn = reader_read_string,
	.ce_read_chars_fn = reader_read_chars,
	.ce_read_int8_fn = reader_read_int8,
	.ce_read_int16_fn = reader_read_int16,
	.ce_read_int32_fn = reader_read_int32,
	.ce_read_int64_fn = reader_read_int64,
	.ce_read_uint8_fn = reader_read_uint8,
	.ce_read_uint16_fn = reader_read_uint16,
	.ce_read_uint32_fn = reader_read_uint32,
	.ce_read_uint64_fn = reader_read_uint64,
	.ce_read_skip_bytes_fn = reader_read_skip_bytes
};


/*
 * =====================================================================================
 *
 *       Filename:  ce_data_reader.c
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  20/02/2013 11:18:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */

#include "ce_data_reader.h"

ce_int_t 
ce_read(ce_data_reader_t *reader,int *v)
{
	return reader->ops->ce_read_fn(reader,v);
}

ce_int_t 
ce_read_boolean(ce_data_reader_t *reader,int *v)
{ 
	return reader->ops->ce_read_boolean_fn(reader,v);
}

ce_int_t 
ce_read_short(ce_data_reader_t *reader,short *v)
{
       	
	return reader->ops->ce_read_short_fn(reader,v);
}

ce_int_t 
ce_read_int(ce_data_reader_t *reader,int *v)
{
       return reader->ops->ce_read_int_fn(reader,v); 	
}

ce_int_t 
ce_read_long(ce_data_reader_t *reader,long *v)
{
        return reader->ops->ce_read_long_fn(reader,v);	
}

ce_int_t 
ce_read_ushort(ce_data_reader_t *reader,unsigned short *v)
{
 
	return reader->ops->ce_read_ushort_fn(reader,v);
}

ce_int_t 
ce_read_uint(ce_data_reader_t *reader,unsigned int *v)
{
       return reader->ops->ce_read_uint_fn(reader,v);	
}

ce_int_t 
ce_read_ulong(ce_data_reader_t *reader,unsigned long *v)
{
	return reader->ops->ce_read_ulong_fn(reader,v);
}

ce_int_t 
ce_read_float(ce_data_reader_t *reader,float *v)
{
	return reader->ops->ce_read_float_fn(reader,v);
}

ce_int_t  
ce_read_double(ce_data_reader_t *reader,double *v)
{
	return reader->ops->ce_read_double_fn(reader,v);
}

ce_int_t  
ce_read_string(ce_data_reader_t *reader,ce_str_t *v)
{
	 return reader->ops->ce_read_string_fn(reader,v);
}

ce_int_t  
ce_read_chars(ce_data_reader_t *reader,char *buf,size_t buf_size)
{
	return reader->ops->ce_read_chars_fn(reader,buf,buf_size);
}

ce_int_t 
ce_read_int8(ce_data_reader_t *reader,int8_t *v)
{
	return reader->ops->ce_read_int8_fn(reader,v);
}

ce_int_t 
ce_read_int16(ce_data_reader_t *reader,int16_t *v)
{
	 return reader->ops->ce_read_int16_fn(reader,v); 
}

ce_int_t 
ce_read_int32(ce_data_reader_t *reader,int32_t *v)
{
	return reader->ops->ce_read_int32_fn(reader,v);
}

ce_int_t 
ce_read_int64(ce_data_reader_t *reader,int64_t *v)
{
	 return reader->ops->ce_read_int64_fn(reader,v);
}

ce_int_t
ce_read_uint8(ce_data_reader_t *reader,uint8_t *v)
{
	return reader->ops->ce_read_uint8_fn(reader,v);
}

ce_int_t 
ce_read_uint16(ce_data_reader_t *reader,uint16_t *v)
{
	return reader->ops->ce_read_uint16_fn(reader,v);
}

ce_int_t 
ce_read_uint32(ce_data_reader_t *reader,uint32_t *v)
{
	return reader->ops->ce_read_uint32_fn(reader,v);
}

ce_int_t
ce_read_uint64(ce_data_reader_t *reader,uint64_t *v)
{
	return reader->ops->ce_read_uint64_fn(reader,v);  
}

 size_t  
 ce_read_skip_bytes(ce_data_reader_t *reader,size_t len)
 {
	return reader->ops->ce_read_skip_bytes_fn(reader,len);
 }


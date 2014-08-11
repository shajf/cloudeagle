/*
 * =====================================================================================
 *
 *       Filename:  ce_data_writer.c
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  20/02/2013 11:19:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */
 
#include "ce_data_writer.h"

size_t 
ce_write(ce_data_writer_t *writer,int v)
{
	return writer->ops->ce_write_fn(writer,v);
}

size_t 
ce_write_bytes(ce_data_writer_t *writer,ce_byte_t bytes[],int len,int offset)
{
	return writer->ops->ce_write_bytes_fn(writer,bytes,len,offset);
}

size_t
ce_write_wchar(ce_data_writer_t *writer,int v)
{
	return writer->ops->ce_write_wchar_fn(writer,v);
}

size_t 
ce_write_boolean(ce_data_writer_t *writer,int v)
{ 
	return writer->ops->ce_write_boolean_fn(writer,v);
}

size_t 
ce_write_short(ce_data_writer_t *writer,short v)
{
       	
	return writer->ops->ce_write_short_fn(writer,v);
}

size_t 
ce_write_int(ce_data_writer_t *writer,int v)
{
       return writer->ops->ce_write_int_fn(writer,v); 	
}

size_t 
ce_write_long(ce_data_writer_t *writer,long v)
{
        return writer->ops->ce_write_long_fn(writer,v);	
}

size_t 
ce_write_ushort(ce_data_writer_t *writer,unsigned short v)
{
 
	return writer->ops->ce_write_ushort_fn(writer,v);
}

size_t 
ce_write_uint(ce_data_writer_t *writer,unsigned int v)
{
       return writer->ops->ce_write_uint_fn(writer,v);	
}

size_t 
ce_write_ulong(ce_data_writer_t *writer,unsigned long v)
{
	return writer->ops->ce_write_ulong_fn(writer,v);
}

size_t 
ce_write_float(ce_data_writer_t *writer,float v)
{
	return writer->ops->ce_write_float_fn(writer,v);
}

size_t  
ce_write_double(ce_data_writer_t *writer,double v)
{
	return writer->ops->ce_write_double_fn(writer,v);
}

size_t  
ce_write_string(ce_data_writer_t *writer,ce_str_t *v)
{
	 return writer->ops->ce_write_string_fn(writer,v);
}

size_t  
ce_write_chars(ce_data_writer_t *writer,const char *v)
{
	return writer->ops->ce_write_chars_fn(writer,v);
}

size_t 
ce_write_int8(ce_data_writer_t *writer,int8_t v)
{
	return writer->ops->ce_write_int8_fn(writer,v);
}

size_t 
ce_write_int16(ce_data_writer_t *writer,int16_t v)
{
	 return writer->ops->ce_write_int16_fn(writer,v); 
}

size_t 
ce_write_int32(ce_data_writer_t *writer,int32_t v)
{
	return writer->ops->ce_write_int32_fn(writer,v);
}

size_t 
ce_write_int64(ce_data_writer_t *writer,int64_t v)
{
	 return writer->ops->ce_write_int64_fn(writer,v);
}

size_t
ce_write_uint8(ce_data_writer_t *writer,uint8_t v)
{
	return writer->ops->ce_write_uint8_fn(writer,v);
}

size_t 
ce_write_uint16(ce_data_writer_t *writer,uint16_t v)
{
	return writer->ops->ce_write_uint16_fn(writer,v);
}

size_t 
ce_write_uint32(ce_data_writer_t *writer,uint32_t v)
{
	return writer->ops->ce_write_uint32_fn(writer,v);
}

size_t
ce_write_uint64(ce_data_writer_t *writer,uint64_t v)
{
	return writer->ops->ce_write_uint64_fn(writer,v);  
}

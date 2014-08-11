/*
 * =====================================================================================
 *
 *       Filename:  ce_data_writer.h
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  08/01/2013 10:35:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */
#ifndef CE_DATA_WRITER_H
#define CE_DATA_WRITER_H

#include "ce_basicdefs.h"
#include "ce_data_wrapbuf.h"
#include "ce_string.h"

typedef struct ce_data_writer_t ce_data_writer_t;

typedef struct 
{
	size_t (*ce_write_fn)(ce_data_writer_t *writer,int v);
	
	size_t (*ce_write_bytes_fn)(ce_data_writer_t *writer,ce_byte_t bytes[],int len,int offset);

	size_t (*ce_write_wchar_fn)(ce_data_writer_t *writer,int v);

	size_t (*ce_write_boolean_fn)(ce_data_writer_t *writer,int v);

	size_t (*ce_write_short_fn)(ce_data_writer_t *writer,int v);

	size_t (*ce_write_int_fn)(ce_data_writer_t *writer,int v);

	size_t (*ce_write_long_fn)(ce_data_writer_t *writer,long v);

	size_t (*ce_write_ushort_fn)(ce_data_writer_t *writer,unsigned short v);

	size_t (*ce_write_uint_fn)(ce_data_writer_t *writer,unsigned int v);

	size_t (*ce_write_ulong_fn)(ce_data_writer_t *writer,unsigned long v);

	size_t (*ce_write_float_fn)(ce_data_writer_t *writer,float v);

        size_t (*ce_write_double_fn)(ce_data_writer_t *writer,double v);

	size_t (*ce_write_string_fn)(ce_data_writer_t *writer,ce_str_t *v);

       	size_t (*ce_write_chars_fn)(ce_data_writer_t *writer,const char *v);

	size_t (*ce_write_int8_fn)(ce_data_writer_t *writer,int8_t v);

	size_t (*ce_write_int16_fn)(ce_data_writer_t *writer,int16_t v);

	size_t (*ce_write_int32_fn)(ce_data_writer_t *writer,int32_t v);

	size_t (*ce_write_int64_fn)(ce_data_writer_t *writer,int64_t v);

	size_t (*ce_write_uint8_fn)(ce_data_writer_t *writer,uint8_t v);

	size_t (*ce_write_uint16_fn)(ce_data_writer_t *writer,uint16_t v);

	size_t (*ce_write_uint32_fn)(ce_data_writer_t *writer,uint32_t v);

	size_t (*ce_write_uint64_fn)(ce_data_writer_t *writer,uint64_t v);

}ce_data_writer_ops_t;

struct ce_data_writer_t
{
	void *user_data;
	ce_data_writer_ops_t *ops;
	ce_data_wrapbuf_t buf;
};

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

extern size_t ce_write(ce_data_writer_t *writer,int v);  
extern size_t ce_write_bytes(ce_data_writer_t *writer,ce_byte_t bytes[],int len,int offset);
extern size_t ce_write_wchar(ce_data_writer_t *writer,int v);
extern size_t ce_write_boolean(ce_data_writer_t *writer,int v);
extern size_t ce_write_short(ce_data_writer_t *writer,short v);
extern size_t ce_write_int(ce_data_writer_t *writer,int v);
extern size_t ce_write_long(ce_data_writer_t *writer,long v);
extern size_t ce_write_ushort(ce_data_writer_t *writer,unsigned short v);
extern size_t ce_write_uint(ce_data_writer_t *writer,unsigned int v);
extern size_t ce_write_ulong(ce_data_writer_t *writer,unsigned long v);
extern size_t ce_write_float(ce_data_writer_t *writer,float v);
extern size_t ce_write_double(ce_data_writer_t *writer,double v);
extern size_t ce_write_string(ce_data_writer_t *writer,ce_str_t *v);
extern size_t ce_write_chars(ce_data_writer_t *writer,const char *v);
extern size_t ce_write_int8(ce_data_writer_t *writer,int8_t v);
extern size_t ce_write_int16(ce_data_writer_t *writer,int16_t v);
extern size_t ce_write_int32(ce_data_writer_t *writer,int32_t v);
extern size_t ce_write_int64(ce_data_writer_t *writer,int64_t v);
extern size_t ce_write_uint8(ce_data_writer_t *writer,uint8_t v);
extern size_t ce_write_uint16(ce_data_writer_t *writer,uint16_t v);
extern size_t ce_write_uint32(ce_data_writer_t *writer,uint32_t v);
extern size_t ce_write_uint64(ce_data_writer_t *writer,uint64_t v);
extern ce_data_writer_ops_t ce_writer_encoding;
#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_DATA_WRITER_H*/

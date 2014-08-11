/*
 * =====================================================================================
 *
 *       Filename:  ce_data_reader.h
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
#ifndef CE_DATA_READER_H
#define CE_DATA_READER_H

#include "ce_basicdefs.h"
#include "ce_data_wrapbuf.h"
#include "ce_string.h"

typedef struct ce_data_reader_t ce_data_reader_t;

typedef struct 
{

       ce_int_t (*ce_read_fn)(ce_data_reader_t *reader,int *v);

       ce_int_t (*ce_read_boolean_fn)(ce_data_reader_t *reader,int *v);

       ce_int_t (*ce_read_short_fn)(ce_data_reader_t *reader,short *v);

       ce_int_t (*ce_read_int_fn)(ce_data_reader_t *reader,int *v);

       ce_int_t (*ce_read_long_fn)(ce_data_reader_t *reader,long *v);

       ce_int_t (*ce_read_ushort_fn)(ce_data_reader_t *reader,unsigned short *v);

       ce_int_t (*ce_read_uint_fn)(ce_data_reader_t *reader,unsigned int *v);

       ce_int_t (*ce_read_ulong_fn)(ce_data_reader_t *reader,unsigned long *v);

       ce_int_t (*ce_read_float_fn)(ce_data_reader_t *reader,float *v);

       ce_int_t (*ce_read_double_fn)(ce_data_reader_t *reader,double *v);

       ce_int_t (*ce_read_string_fn)(ce_data_reader_t *reader,ce_str_t *v);

       ce_int_t (*ce_read_chars_fn)(ce_data_reader_t *reader,char *buf,size_t buf_size);

       ce_int_t (*ce_read_int8_fn)(ce_data_reader_t *reader,int8_t *v);

       ce_int_t (*ce_read_int16_fn)(ce_data_reader_t *reader,int16_t *v);

       ce_int_t (*ce_read_int32_fn)(ce_data_reader_t *reader,int32_t *v);

       ce_int_t (*ce_read_int64_fn)(ce_data_reader_t *reader,int64_t *v);

       ce_int_t (*ce_read_uint8_fn)(ce_data_reader_t *reader,uint8_t *v);

       ce_int_t (*ce_read_uint16_fn)(ce_data_reader_t *reader,uint16_t *v);

       ce_int_t (*ce_read_uint32_fn)(ce_data_reader_t *reader,uint32_t *v);

       ce_int_t (*ce_read_uint64_fn)(ce_data_reader_t *reader,uint64_t *v);

       size_t    (*ce_read_skip_bytes_fn)(ce_data_reader_t *reader,size_t len);

}ce_data_reader_ops_t;


struct ce_data_reader_t
{
	void *user_data;
	ce_data_reader_ops_t *ops;
	ce_data_wrapbuf_t buf;
};

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

extern ce_int_t ce_read(ce_data_reader_t *reader,int *v);

extern ce_int_t ce_read_boolean(ce_data_reader_t *reader,int *v);

extern ce_int_t ce_read_short(ce_data_reader_t *reader,short *v);

extern ce_int_t ce_read_int(ce_data_reader_t *reader,int *v);

extern ce_int_t ce_read_long(ce_data_reader_t *reader,long *v);

extern ce_int_t ce_read_ushort(ce_data_reader_t *reader,unsigned short *v);

extern ce_int_t ce_read_uint(ce_data_reader_t *reader,unsigned int *v);

extern ce_int_t  ce_read_ulong(ce_data_reader_t *reader,unsigned long *v);

extern ce_int_t  ce_read_float(ce_data_reader_t *reader,float *v);

extern ce_int_t  ce_read_double(ce_data_reader_t *reader,double *v);

extern ce_int_t  ce_read_string(ce_data_reader_t *reader,ce_str_t *v);

extern ce_int_t  ce_read_chars(ce_data_reader_t *reader,char *buf,size_t buf_size);

extern ce_int_t ce_read_int8(ce_data_reader_t *reader,int8_t *v);

extern ce_int_t ce_read_int16(ce_data_reader_t *reader,int16_t *v);

extern ce_int_t ce_read_int32(ce_data_reader_t *reader,int32_t *v);

extern ce_int_t ce_read_int64(ce_data_reader_t *reader,int64_t *v);

extern ce_int_t ce_read_uint8(ce_data_reader_t *reader,uint8_t *v);

extern ce_int_t ce_read_uint16(ce_data_reader_t *reader,uint16_t *v);

extern ce_int_t ce_read_uint32(ce_data_reader_t *reader,uint32_t *v);

extern ce_int_t ce_read_uint64(ce_data_reader_t *reader,uint64_t *v);

extern size_t  ce_read_skip_bytes(ce_data_reader_t *reader,size_t len);

extern ce_data_reader_ops_t ce_reader_encoding;
#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*CE_DATA_READER_H*/

/*
 * =====================================================================================
 *
 *       Filename:  ce_data_record.c
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  08/01/2013 11:30:34 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */
#include "ce_data_record.h"

size_t ce_data_record_write(ce_data_writer_t *ipc_writer, ce_data_record_t *record)
{
	return record->ops.ipc_record_write_fn(ipc_writer,record);
}

size_t ce_data_record_read(ce_data_reader_t *ipc_reader, ce_data_record_t *record)
{
	return record->ops.ipc_record_read_fn(ipc_reader,record);
}


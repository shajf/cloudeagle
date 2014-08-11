/*
 * =====================================================================================
 *
 *       Filename:  ce_data_record.h
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
#ifndef CE_IPC_RECORD_H
#define CE_IPC_RECORD_H
#include "ce_data_writer.h"
#include "ce_data_reader.h"
typedef struct ce_data_record_t ce_data_record_t;
/*Define ce ipc record operation structure,
 * this method must be implemented by every
 * record*/
typedef struct 
{
	 size_t (*ipc_record_write_fn)(ce_data_writer_t *writer,ce_data_record_t *record);
	 size_t (*ipc_record_read_fn)(ce_data_reader_t *reader,ce_data_record_t *record);
}ce_data_record_ops_t;

struct ce_data_record_t
{
	ce_data_record_ops_t ops;
	uint32_t rcd_size;
};

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

extern size_t ce_data_record_write(ce_data_writer_t *ipc_writer, ce_data_record_t *record);

extern size_t ce_data_record_read(ce_data_reader_t *ipc_reader, ce_data_record_t *record);
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*CE_IPC_RECORD_H*/

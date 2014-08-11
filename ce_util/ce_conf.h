/*
 * =====================================================================================
 *
 *       Filename:  ce_conf.h
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  07/01/2013 13:56:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */

#ifndef CE_CONF_H
#define CE_CONF_H
#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/

#include "ce_basicdefs.h"
#include "ce_string.h"
#include "ce_palloc.h"
#include "ce_tables.h"
#include "ce_thread_mutex.h"
#include "ce_ctype.h"

typedef struct
{
	ce_str_t c_id;

	ce_pool_t *kv_pool;

	ce_table_t *kv_map;
	ce_array_t *k_arr;

	ce_thread_mutex_t *conf_lock;

	unsigned use_conf_lock:1;

}ce_conf_t;

#define CE_CONF_T_SIZE \
	sizeof(ce_conf_t)

typedef ce_int_t (*conf_config_fun)(ce_conf_t *conf,
				      u_char *prefix);
#define CONF_PTR_NAME	"__conf_ptr"


#define ce_get_my_conf(who) ((who)->conf)
#define ce_set_my_conf(who,m_conf) ((who)->conf=(m_conf))

extern ce_int_t
ce_load_conf(u_char *conf_full_name,ce_conf_t *conf);

extern ce_conf_t *ce_create_conf(void);

extern void ce_destroy_conf(ce_conf_t *conf);


extern ce_conf_t * ce_config(u_char *prefix,
				 u_char *conf_dir,
				 u_char *conf_file_name,
				 conf_config_fun config);

extern ce_conf_t * ce_config2(u_char *full_name);

static inline  ce_int_t
ce_conf_setInt(ce_conf_t *conf,
		 const char *key,
		int value)
{
	u_char *k_res;
	Int *v_res;
	ce_pool_t *pool;
	ce_table_t *kv_map;

	pool=conf->kv_pool;
	kv_map=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pIntdup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(kv_map,k_res,(u_char*)v_res);

	return CE_OK;
	
}

static inline  ce_int_t
ce_conf_setShort(ce_conf_t *conf,
		 const char *key,
		short int value)
{
	u_char *k_res;
	Short *v_res;
	ce_pool_t *pool;
	ce_table_t *kv_map;

	pool=conf->kv_pool;
	kv_map=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pShortdup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(kv_map,k_res,(u_char*)v_res);

	return CE_OK;
	
}


static inline  ce_int_t
ce_conf_setInt8(ce_conf_t *conf,
		 const char *key,
		int8_t value)
{
	u_char *k_res;
	Int8 *v_res;
	ce_pool_t *pool;
	ce_table_t *table;

	pool=conf->kv_pool;
	table=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pInt8dup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(table,k_res,(u_char*)v_res);

	return CE_OK;
	
}

static inline  ce_int_t
ce_conf_setInt16(ce_conf_t *conf,
		 const char *key,
		int16_t value)
{
	u_char *k_res;
	Int16 *v_res;
	ce_pool_t *pool;
	ce_table_t *table;

	pool=conf->kv_pool;
	table=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pInt16dup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(table,k_res,(u_char*)v_res);

	return CE_OK;
	
}


static inline  ce_int_t
ce_conf_setInt32(ce_conf_t *conf,
		 const char *key,
		int32_t value)
{
	u_char *k_res;
	Int32 *v_res;
	ce_pool_t *pool;
	ce_table_t *table;

	pool=conf->kv_pool;
	table=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pInt32dup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(table,k_res,(u_char*)v_res);

	return CE_OK;
	
}


static inline  ce_int_t
ce_conf_setInt64(ce_conf_t *conf,
		 const char *key,
		int64_t value)
{
	u_char *k_res;
	Int64 *v_res;
	ce_pool_t *pool;
	ce_table_t *table;

	pool=conf->kv_pool;
	table=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pInt64dup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(table,k_res,(u_char*)v_res);

	return CE_OK;
	
}

static inline  ce_int_t
ce_conf_setUInt(ce_conf_t *conf,
		 const char *key,
		unsigned int value)
{
	u_char *k_res;
	UInt *v_res;
	ce_pool_t *pool;
	ce_table_t *table;

	pool=conf->kv_pool;
	table=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pUIntdup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(table,k_res,(u_char*)v_res);

	return CE_OK;
	
}

static inline  ce_int_t
ce_conf_setUShort(ce_conf_t *conf,
		 const char *key,
		unsigned short int value)
{
	u_char *k_res;
	UShort *v_res;
	ce_pool_t *pool;
	ce_table_t *table;

	pool=conf->kv_pool;
	table=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pUShortdup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(table,k_res,(u_char*)v_res);

	return CE_OK;
	
}


static inline  ce_int_t
ce_conf_setUInt8(ce_conf_t *conf,
		 const char *key,
		uint8_t value)
{
	u_char *k_res;
	UInt8 *v_res;
	ce_pool_t *pool;
	ce_table_t *table;

	pool=conf->kv_pool;
	table=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pUInt8dup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(table,k_res,(u_char*)v_res);

	return CE_OK;
	
}

static inline  ce_int_t
ce_conf_setUInt16(ce_conf_t *conf,
		 const char *key,
		uint16_t value)
{
	u_char *k_res;
	UInt16 *v_res;
	ce_pool_t *pool;
	ce_table_t *table;

	pool=conf->kv_pool;
	table=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pUInt16dup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(table,k_res,(u_char*)v_res);

	return CE_OK;
	
}


static inline  ce_int_t
ce_conf_setUInt32(ce_conf_t *conf,
		 const char *key,
		uint32_t value)
{
	u_char *k_res;
	UInt32 *v_res;
	ce_pool_t *pool;
	ce_table_t *table;

	pool=conf->kv_pool;
	table=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pUInt32dup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(table,k_res,(u_char*)v_res);

	return CE_OK;
	
}


static inline  ce_int_t
ce_conf_setUInt64(ce_conf_t *conf,
		 const char *key,
		uint64_t value)
{
	u_char *k_res;
	UInt64 *v_res;
	ce_pool_t *pool;
	ce_table_t *table;

	pool=conf->kv_pool;
	table=conf->kv_map;

	k_res=(u_char*)ce_pstrdup(pool,key);

	v_res=ce_pUInt64dup(pool,value);

	if(k_res==NULL||v_res==NULL)
	{
		return CE_ERROR;
	}

	ce_table_setn(table,k_res,(u_char*)v_res);

	return CE_OK;
	
}
static inline  ce_int_t
ce_conf_setBoolean(ce_conf_t *conf,
		      const char *key,
		     int value)
{
	 u_char *k_res;
	 Boolean *v_res;
	 ce_pool_t *pool;
	 ce_table_t *table;
	 
	 pool=conf->kv_pool;
	 table=conf->kv_map;

	 k_res=(u_char*)ce_pstrdup(pool,key);
	 v_res=ce_pBooleandup(pool,value);

	 if(k_res==NULL||v_res==NULL)
	 {
		 return CE_ERROR;  
	 }

	  ce_table_setn(table,k_res,(u_char*)v_res);
	  return CE_OK; 

}

static inline ce_int_t
ce_conf_setLong(ce_conf_t *conf,
		   const char *key,
		  long value)
{
	u_char *k_res; 
	Long *v_res;

	ce_pool_t *pool;
	ce_table_t *table;
	pool=conf->kv_pool; 
	table=conf->kv_map;

	 k_res=(u_char*)ce_pstrdup(pool,key);
	 v_res=ce_pLongdup(pool,value);

	 if(k_res==NULL||v_res==NULL) 
	 {
		 return CE_ERROR;
	 }

	 ce_table_setn(table,k_res,(u_char*)v_res);
	 return CE_OK;
}

static inline ce_int_t
ce_conf_setULong(ce_conf_t *conf,
		   const char *key,
		  unsigned long value)
{
	u_char *k_res; 
	ULong *v_res;

	ce_pool_t *pool;
	ce_table_t *table;
	pool=conf->kv_pool; 
	table=conf->kv_map;

	 k_res=(u_char*)ce_pstrdup(pool,key);
	 v_res=ce_pULongdup(pool,value);

	 if(k_res==NULL||v_res==NULL) 
	 {
		 return CE_ERROR;
	 }

	 ce_table_setn(table,k_res,(u_char*)v_res);
	 return CE_OK;
}

static inline ce_int_t
ce_conf_setSize(ce_conf_t *conf,
		   const char *key,
		  size_t value)
{
	 u_char *k_res;
	 Size *v_res;
	 ce_pool_t *pool;
	 ce_table_t *table;
	 
	 pool=conf->kv_pool;
	 table=conf->kv_map;

	 k_res=(u_char*)ce_pstrdup(pool,key);
	 v_res=ce_pSizedup(pool,value);

	 if(k_res==NULL||v_res==NULL)
	 {
		return CE_ERROR;	
	 }
	 ce_table_setn(table,k_res,(u_char*)v_res);
	 return CE_OK;
}

static inline ce_int_t
ce_conf_setSSize(ce_conf_t *conf,
		    const char *key,
		   ssize_t value)
{
	u_char *k_res;
	SSize *v_res;
	
	ce_pool_t *pool;
	ce_table_t *table;
	
	pool=conf->kv_pool;
	table=conf->kv_map;

	 k_res=(u_char*)ce_pstrdup(pool,key);
	 v_res=ce_pSSizedup(pool,value);

	 if(k_res==NULL||v_res==NULL)
	 {

		return CE_ERROR;	
	 }

	 ce_table_setn(table,k_res,(u_char*)v_res);
	 return CE_OK;
}




static inline  ce_int_t
ce_cont_setOffset(ce_conf_t *conf,
		    const char *key,
		   off_t value)
{

	u_char *k_res;
	Offset *v_res;
	
	ce_pool_t *pool;
	ce_table_t *table;
	
	pool=conf->kv_pool;
	table=conf->kv_map;

	 k_res=(u_char*)ce_pstrdup(pool,key);
	 v_res=ce_pOffsetdup(pool,value);

	 if(k_res==NULL||v_res==NULL)
	 {
	 
		return CE_ERROR;	
	 }

	 ce_table_setn(table,k_res,(u_char*)v_res);
	 return CE_OK;

}

static inline  ce_int_t
ce_conf_setDouble(ce_conf_t *conf,
		     const char *key,
		    double value)
{
	  u_char *k_res; 
	  Double *v_res; 
	  ce_pool_t *pool;
	  ce_table_t *table; 
	  
	  pool=conf->kv_pool;
	  table=conf->kv_map;

	  k_res=(u_char*)ce_pstrdup(pool,key);
	  v_res=ce_pDoubledup(pool,value);
	  
	  if(k_res==NULL||v_res==NULL)
	  {
		  return CE_ERROR;
	  }

	   ce_table_setn(table,k_res,(u_char*)v_res);
	   return CE_OK;  
}

static inline  ce_int_t
ce_conf_setFloat(ce_conf_t *conf,
		     const char *key,
		    float value)
{
	  u_char *k_res; 
	  Float *v_res; 
	  ce_pool_t *pool;
	  ce_table_t *table; 
	  
	  pool=conf->kv_pool;
	  table=conf->kv_map;

	  k_res=(u_char*)ce_pstrdup(pool,key);
	  v_res=ce_pFloatdup(pool,value);
	  
	  if(k_res==NULL||v_res==NULL)
	  {
		  return CE_ERROR;
	  }

	   ce_table_setn(table,k_res,(u_char*)v_res);
	   return CE_OK;  
}
static inline ce_int_t
ce_conf_setOffset(ce_conf_t *conf,
		     const char *key,
		    off_t value)
{

	  u_char *k_res; 
	  Offset *v_res; 
	  ce_pool_t *pool;
	  ce_table_t *table; 
	  
	  pool=conf->kv_pool;
	  table=conf->kv_map;

	  k_res=(u_char*)ce_pstrdup(pool,key);
	  v_res=ce_pOffsetdup(pool,value);
	  
	  if(k_res==NULL||v_res==NULL)
	  {
		  return CE_ERROR;
	  }

	   ce_table_setn(table,k_res,(u_char*)v_res);
	   return CE_OK;  
}
static inline ce_int_t
ce_conf_setText(ce_conf_t *conf,
		      const char *key,
		      const char *value)
{
	
	  u_char *k_res; 
	  u_char *v_res; 
	  ce_pool_t *pool;
	  ce_table_t *table; 
	  
	  pool=conf->kv_pool;
	  table=conf->kv_map;

	  k_res=(u_char*)ce_pstrdup(pool,key);
	  v_res=(u_char*)ce_pstrdup(pool,value);
	  
	  if(k_res==NULL||v_res==NULL)
	  {
		  return CE_ERROR;
	  }

	   ce_table_setn(table,k_res,(u_char*)v_res);
	   return CE_OK;  
}


static inline ce_int_t
ce_conf_setByte(ce_conf_t *conf,
		      const char *key,
		     unsigned char value)
{
	
	  u_char *k_res; 
	  Byte *v_res; 
	  ce_pool_t *pool;
	  ce_table_t *table; 
	  
	  pool=conf->kv_pool;
	  table=conf->kv_map;

	  k_res=(u_char*)ce_pstrdup(pool,key);
	  v_res=ce_pBytedup(pool,value);
	  
	  if(k_res==NULL||v_res==NULL)
	  {
		  return CE_ERROR;
	  }

	   ce_table_setn(table,k_res,(u_char*)v_res);
	   return CE_OK;  
}

static inline  int
ce_conf_getInt(ce_conf_t *conf,
		  const char *key,
		 int def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	Int *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Int*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}


static inline  short int
ce_conf_getShort(ce_conf_t *conf,
		  const char *key,
		 short int def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	Short *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Short*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}


static inline  int8_t
ce_conf_getInt8(ce_conf_t *conf,
		  const char *key,
		 int8_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	Int8 *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Int8*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline  int16_t
ce_conf_getInt16(ce_conf_t *conf,
		  const char *key,
		 int16_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	Int16 *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Int16*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}


static inline  int32_t
ce_conf_getInt32(ce_conf_t *conf,
		  const char *key,
		 int32_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	Int32 *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Int32*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline  int64_t
ce_conf_getInt64(ce_conf_t *conf,
		  const char *key,
		 int64_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	Int64 *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Int64*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}


static inline  unsigned int
ce_conf_getUInt(ce_conf_t *conf,
		  const char *key,
		 unsigned  int def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	UInt *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(UInt*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}


static inline unsigned short int
ce_conf_getUShort(ce_conf_t *conf,
		  const char *key,
		unsigned short int def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	UShort *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(UShort*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}


static inline  uint8_t
ce_conf_getUInt8(ce_conf_t *conf,
		  const char *key,
		 uint8_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	UInt8 *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(UInt8*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline  uint16_t
ce_conf_getUInt16(ce_conf_t *conf,
		  const char *key,
		 uint16_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	UInt16 *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(UInt16*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}


static inline  uint32_t
ce_conf_getUInt32(ce_conf_t *conf,
		  const char *key,
		 uint32_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	UInt32 *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(UInt32*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline  uint64_t
ce_conf_getUInt64(ce_conf_t *conf,
		  const char *key,
		 uint64_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	UInt64 *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(UInt64*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline  long
ce_conf_getLong(ce_conf_t *conf,
		  const char *key,
		 long def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	Long *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Long*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline  unsigned long
ce_conf_getULong(ce_conf_t *conf,
		  const char *key,
		 unsigned long def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	ULong *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(ULong*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline  size_t
ce_conf_getSize(ce_conf_t *conf,
		  const char *key,
		 size_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	Size *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Size*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline  ssize_t
ce_conf_getSSize(ce_conf_t *conf,
		  const char *key,
		 ssize_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	SSize *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(SSize*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}


static inline  off_t
ce_conf_getOffset(ce_conf_t *conf,
		  const char *key,
		 off_t def)
{
	ce_table_t *kv_map=conf->kv_map;
	
	Offset *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Offset*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline int
ce_conf_getBoolean(ce_conf_t *conf,
		     const char* key,
		     int def)
{
	
	ce_table_t *kv_map=conf->kv_map;
	
	Boolean *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Boolean*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline  double
ce_conf_getDouble(ce_conf_t *conf,
		     const char* key,
		     double def)
{
	
	ce_table_t *kv_map=conf->kv_map;
	
	Double *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Double*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static inline  float
ce_conf_getFloat(ce_conf_t *conf,
		     const char *key,
		    float def)
{
		
	ce_table_t *kv_map=conf->kv_map;
	
	Float *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Float*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

static  inline   char *
ce_conf_getText(ce_conf_t *conf,
		  const char *key,
		   char *def)
{
	
	ce_table_t *kv_map=conf->kv_map;
	
	 char *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res;
}


static  inline unsigned char 
ce_conf_getByte(ce_conf_t *conf,
		   const char *key,
		  unsigned char def)
{
	
	ce_table_t *kv_map=conf->kv_map;
	
	Byte *res;

	if(kv_map==NULL)
	{
		return def;
	}
	
	res=(Byte*)ce_table_get(kv_map,key);

	if(res==NULL)
	{
		return def;
	}

	return res->val;
}

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*CE_CONF_H*/

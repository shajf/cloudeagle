/*
 * =====================================================================================
 *
 *       Filename:  ce_ctype.h
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

#ifndef CE_CTYPE_H
#define CE_CTYPE_H
#ifdef __cplusplus
extern "c" {
#endif /*__cplusplus*/

#include "ce_basicdefs.h"

typedef struct Int 	Int;
typedef struct Short	Short;
typedef struct Int8 	Int8;
typedef struct Int16 	Int16;
typedef struct Int32	Int32;
typedef struct Int64	Int64;


typedef struct UInt 	UInt;
typedef struct UShort	UShort;
typedef struct UInt8 	UInt8;
typedef struct UInt16 	UInt16;
typedef struct UInt32	UInt32;
typedef struct UInt64	UInt64;


typedef struct Double 	Double;
typedef struct Boolean 	Boolean;
typedef struct Text 	Text;
typedef struct Byte 	Byte;
typedef struct Float 	Float;
typedef struct Long 	Long;
typedef struct ULong    ULong;

typedef struct Size 	Size;
typedef struct SSize 	SSize;
typedef struct Offset 	Offset;

struct Int
{
	int val;
};
#define INT_SIZE sizeof(Int)
static inline Int * 
ce_pIntdup(ce_pool_t *pool,
	     int v)
{
	Int *res=(Int*)ce_palloc(pool,INT_SIZE);
	if(res==NULL)
		return NULL;
	res->val=v;
	return res;
	
}

struct Short
{
	short int val;
};

#define SHORT_SIZE sizeof(Short)

static inline Short * 
ce_pShortdup(ce_pool_t *pool,
	     short int v)
{
	Short *res=(Short*)ce_palloc(pool,SHORT_SIZE);
	
	if(res==NULL)
	{
		return NULL;
	
	}
	res->val=v;
	return res;
	
}

struct Int8
{
	int8_t val;
};

#define INT8_SIZE sizeof(Int8)

static inline Int8 * 
ce_pInt8dup(ce_pool_t *pool,
	      int8_t v)
{
	Int8 *res=(Int8*)ce_palloc(pool,INT8_SIZE);
	
	if(res==NULL)
	{
		return NULL;
	
	}
	res->val=v;
	return res;
	
}


struct Int16
{
	int16_t val;
};

#define INT16_SIZE sizeof(Int16)

static inline Int16 * 
ce_pInt16dup(ce_pool_t *pool,
	      int16_t v)
{
	Int16 *res=(Int16*)ce_palloc(pool,INT16_SIZE);
	
	if(res==NULL)
	{
		return NULL;
	
	}
	res->val=v;
	return res;
	
}


struct Int32
{
	int32_t val;
};

#define INT32_SIZE sizeof(Int32)

static inline Int32 * 
ce_pInt32dup(ce_pool_t *pool,
	      int32_t v)
{
	Int32 *res=(Int32*)ce_palloc(pool,INT32_SIZE);
	
	if(res==NULL)
	{
		return NULL;
	
	}
	res->val=v;
	return res;
	
}


struct Int64
{
	int64_t val;
};

#define INT64_SIZE sizeof(Int64)

static inline Int64 * 
ce_pInt64dup(ce_pool_t *pool,
	      int32_t v)
{
	Int64 *res=(Int64*)ce_palloc(pool,INT64_SIZE);
	
	if(res==NULL)
	{
		return NULL;
	
	}
	res->val=v;
	return res;
	
}


struct UInt
{
	unsigned int val;
};

#define UINT_SIZE sizeof(UInt)
static inline UInt * 
ce_pUIntdup(ce_pool_t *pool,
	     unsigned int v)
{
	UInt *res=(UInt*)ce_palloc(pool,UINT_SIZE);
	if(res==NULL)
		return NULL;
	res->val=v;
	return res;
	
}

struct UShort
{
	unsigned short int val;
};

#define USHORT_SIZE sizeof(UShort)

static inline UShort * 
ce_pUShortdup(ce_pool_t *pool,
	     unsigned short int v)
{
	UShort *res=(UShort*)ce_palloc(pool,USHORT_SIZE);
	
	if(res==NULL)
	{
		return NULL;
	
	}
	res->val=v;
	return res;
	
}

struct UInt8
{
	uint8_t val;
};

#define UINT8_SIZE sizeof(UInt8)

static inline UInt8 * 
ce_pUInt8dup(ce_pool_t *pool,
	      uint8_t v)
{
	UInt8 *res=(UInt8*)ce_palloc(pool,UINT8_SIZE);
	
	if(res==NULL)
	{
		return NULL;
	
	}
	res->val=v;
	return res;
	
}

struct UInt16
{
	uint16_t val;
};

#define UINT16_SIZE sizeof(UInt16)

static inline UInt16 * 
ce_pUInt16dup(ce_pool_t *pool,
	      uint16_t v)
{
	UInt16 *res=(UInt16*)ce_palloc(pool,UINT16_SIZE);
	
	if(res==NULL)
	{
		return NULL;
	
	}
	res->val=v;
	return res;
	
}


struct UInt32
{
	uint32_t val;
};

#define UINT32_SIZE sizeof(UInt32)

static inline UInt32 * 
ce_pUInt32dup(ce_pool_t *pool,
	      uint32_t v)
{
	UInt32 *res=(UInt32*)ce_palloc(pool,UINT32_SIZE);
	
	if(res==NULL)
	{
		return NULL;
	
	}
	res->val=v;
	return res;
	
}


struct UInt64
{
	uint64_t val;
};

#define UINT64_SIZE sizeof(UInt64)

static inline UInt64 * 
ce_pUInt64dup(ce_pool_t *pool,
	      uint32_t v)
{
	UInt64 *res=(UInt64*)ce_palloc(pool,UINT64_SIZE);
	
	if(res==NULL)
	{
		return NULL;
	
	}
	res->val=v;
	return res;
	
}

struct Double
{
	double val;
};

#define DOUBLE_SIZE sizeof(Double)
static inline Double * 
ce_pDoubledup(ce_pool_t *pool,
	     double v)
{
	 Double *res=(Double*)ce_palloc(pool,DOUBLE_SIZE);
	 if(res==NULL)
		 return NULL;
	res->val=v;
	 return res;
}

struct Boolean
{
	unsigned val:1;
};

#define BOOLEAN_SIZE sizeof(Boolean)
#define setBVal(k,v) ((k)->val=(v)?1:0)
#define getBVal(k) ((k)->val)

static inline Boolean *
ce_pBooleandup(ce_pool_t *pool,
		int v)
{
	Boolean *res=(Boolean*)ce_palloc(pool,BOOLEAN_SIZE);
	if(res==NULL)
		 return NULL;
	if(v)
	{
		res->val=1;
	}
	else
	{
		res->val=0;
	}
	return res;
}

struct Text
{
	u_char *val;
	size_t len;
};
#define TEXT_SIZE sizeof(Text)

struct Byte
{
	unsigned char val;
};
#define BYTE_SIZE sizeof(Byte)
static inline Byte*
ce_pBytedup(ce_pool_t *pool,
	      unsigned char v)
{
	Byte *res=(Byte*)ce_palloc(pool,BYTE_SIZE);
	if(res==NULL)
	{
		return NULL;
	}

	res->val=v;
	return res;
}

struct Float
{
	float val;
};
#define FLOAT_SIZE sizeof(Float)
static inline Float *
ce_pFloatdup(ce_pool_t *pool,
	       float v)
{
	Float *res=(Float*)ce_palloc(pool,FLOAT_SIZE);
	if(res==NULL)
	{
		return NULL;
	}

	res->val=v;
	return res;
}

struct Long
{
	long val;
};
#define LONG_SIZE sizeof(Long)
static inline Long *
ce_pLongdup(ce_pool_t *pool,long v)
{
	Long *res=(Long*)ce_palloc(pool,LONG_SIZE);
	if(res==NULL)
	{
		return NULL;
	}
	res->val=v;
	return res;
}

struct ULong
{
	unsigned long val;
};
#define ULONG_SIZE sizeof(ULong)
static inline ULong *
ce_pULongdup(ce_pool_t *pool,unsigned long v)
{
	ULong *res=(ULong*)ce_palloc(pool,ULONG_SIZE);
	if(res==NULL)
	{
		return NULL;
	}
	res->val=v;
	return res;
}

struct Size
{
	size_t val;
};
#define SIZE_SIZE sizeof(Size)

static inline Size*
ce_pSizedup(ce_pool_t *pool,size_t v)
{
	Size* res=(Size*)ce_palloc(pool,SIZE_SIZE);
	if(res==NULL)
	{
		return NULL;
	}
	res->val=v;
	return res;
}

struct SSize
{
	ssize_t val;
};
#define SSIZE_SIZE sizeof(SSize)
static inline SSize*
ce_pSSizedup(ce_pool_t *pool,ssize_t v)
{
	SSize *res=(SSize*)ce_palloc(pool,SSIZE_SIZE);
	if(res==NULL)
	{
		return NULL;
	}
	res->val=v;
	return res;
}

struct Offset
{
	off_t val;
};

#define OFFSET_SIZE sizeof(Offset)

static inline Offset*
ce_pOffsetdup(ce_pool_t *pool,off_t v)
{
	Offset *res=(Offset*)ce_palloc(pool,OFFSET_SIZE);
	if(res==NULL)
	{
		return NULL;
	}
	res->val=v;
	return res;
}

#define ce_getv(k) ((k)->val)
#define ce_setv(k,v) ((k)->val=v)

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*CE_CTYPE_H*/

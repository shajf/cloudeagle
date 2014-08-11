/*
 * =====================================================================================
 *
 *       Filename:  ce_map.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  10/26/2012 11:32:49 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng 
 *   Organization:  
 *
 * =====================================================================================
 */
#include "ce_map.h"

#define PERTURB_SHIFT 5
static u_char  _dummy_struct[1];

#define dummy (&_dummy_struct[0])


#define MK_SIZE(mk) ((mk)->m_key_num) 
#define MK_MASK(mk) (((mk)->m_key_num)-1)
#define IS_POWER_OF_2(x) (((x) & (x-1)) == 0)
#define USABLE_FRACTION(n) ((((n) << 1)+1)/3)
#define GROWTH_RATE(x) ((x) * 2)
#define MAP_MINSIZE	4
#define MAP_INIT_D_SIZE	8

static ce_map_keys_t empty_keys_struct={
	1,
	0,
	{{0,NULL,NULL}}
};

#define EMPTY_KEYS &empty_keys_struct;

static ce_hash_t string_hash(ce_str_t *key)
{
	
	ce_hash_t hash=0,x=0; 
	size_t i;
	u_char *k_data=key->data;

	if(key==NULL||key->data==NULL||key->len==0)
	{
		return 0;
	}

	for(i=0;i<key->len;i++)
	{
		hash=(hash<<4)+(k_data[i]);
		if((x=hash&0xF0000000L)!=0)
		{
			hash^=(x>>24);
			hash&=~x;
		}
	}

	return (hash&0x7FFFFFFF);
}

static ce_map_keys_t *create_map_keys(int k_num)
{
	ce_map_keys_t *mp_keys;
	int i;
	ce_map_entry_t *ep0;
	assert(k_num>=MAP_MINSIZE);
	assert(IS_POWER_OF_2(k_num));

	int k_size=sizeof(ce_map_entry_t)*(k_num-1);
	mp_keys=(ce_map_keys_t*)malloc(sizeof(ce_map_keys_t)+\
						k_size);
	
	if(mp_keys==NULL)
	{
		return NULL;
	}
	mp_keys->m_key_num=k_num;
	mp_keys->m_key_usable=USABLE_FRACTION(k_num);
	ep0=&mp_keys->m_key_entries[0];
	ep0->m_hash=0;
	
	for(i=0;i<k_num;i++)
	{
		ep0[i].m_key=NULL;
		ep0[i].m_value=NULL;
	}

	return mp_keys; 
}

static void free_map_keys(ce_map_keys_t *mp_keys,void *private_data,
			  kv_free_fun_ptr kv_free_fun)
{
	ce_map_entry_t *entries=&mp_keys->m_key_entries[0];
	
	if(kv_free_fun)
	{
		int i,n;

		for(i=0;n=MK_SIZE(mp_keys),i<n;i++)
		{
			if(entries[i].m_key==NULL||entries[i].m_key==dummy)
			{
				continue;
			}

			kv_free_fun(private_data,
					entries[i].m_key,
					entries[i].m_value);	
		}
	}

	free(mp_keys);
} 


ce_map_t *ce_create_map(void* private_data,
			  kv_free_fun_ptr kv_free_fun)
{
	ce_map_t *mp=(ce_map_t*)malloc(sizeof(ce_map_t));

	if(mp==NULL)
	{
		return NULL;
	}

	mp->ma_used=0;
	mp->private_data=private_data;
	mp->kv_free_fun=kv_free_fun;
	mp->ma_keys=create_map_keys(MAP_INIT_D_SIZE);

	if(mp->ma_keys==NULL)
	{
		free(mp);
		return NULL;
	}
	return mp;
}

static ce_map_entry_t *find_key_entry(ce_map_t *mp,ce_str_t *key,ce_hash_t hash)
{
	register int i;
	register int perturb;
	register ce_map_entry_t *freeslot;
	register int mask=MK_MASK(mp->ma_keys); //keynum -1
	ce_map_entry_t *ep0=&mp->ma_keys->m_key_entries[0];
	register ce_map_entry_t *ep;
	i=(int)hash&mask;
	ep=&ep0[i];
	
	if(ep->m_key==NULL)
	{
		return ep;	
	}
	
	if(ep->m_key==dummy)
	{
		freeslot=ep;	
	}
	
	else
	{
		if(ep->m_hash==hash&&ce_memn2cmp(key->data,
					 ep->m_key,
					 key->len,
					 ce_strlen(ep->m_key))==0)
		{
			return ep;	
		}
		freeslot=NULL;
	}
	
	for(perturb=(int)hash;;perturb>>=PERTURB_SHIFT)
	{
		i=(i<<2)+i+perturb+1;
		ep=&ep0[i&mask];
		if(ep->m_key==NULL)
		{
			if(freeslot==NULL)
				return ep;
			else
				return freeslot;
		}
		
		if((ep->m_hash==hash)&&(ep->m_key!=dummy)&&\
		 (ce_memn2cmp(key->data,
			      ep->m_key,
			      key->len,
			      ce_strlen(ep->m_key))==0))
		{
			return ep;
		}

		if(ep->m_key==dummy&&freeslot==NULL)
		{
			freeslot=ep;
		}
	}
	assert(0); /*not reached here*/
	return NULL;
}

static ce_map_entry_t *find_empty_slot(ce_map_t *mp,ce_str_t *key,
					ce_hash_t hash)
{
	int i;
	int perturb;
	int mask=MK_MASK(mp->ma_keys);
	ce_map_entry_t *ep0=&mp->ma_keys->m_key_entries[0];
	ce_map_entry_t *ep;
	assert(key!=NULL);
	i=hash&mask;
	ep=&ep0[i];

	for(perturb=hash;ep->m_key!=NULL;perturb>>=PERTURB_SHIFT)
	{
		i=(i<<2)+i+perturb+1;
		ep=&ep0[i&mask];
	}

	assert(ep->m_value==NULL);

	return ep;
}

static void insertmap_clean(ce_map_t *mp,u_char *key,ce_hash_t hash,
			    void *value)
{
	int i;
	int perturb;
	ce_map_keys_t *k=mp->ma_keys;
	int mask=(int)MK_MASK(k);
	ce_map_entry_t *ep0=&k->m_key_entries[0];
	ce_map_entry_t *ep;
	assert(value!=NULL);
	assert(key!=NULL);
	assert(key!=dummy);
	i=hash&mask;
	ep=&ep0[i];
	
	for(perturb=hash;ep->m_key!=NULL;perturb>>=PERTURB_SHIFT)
	{
		i=(i<<2)+i+perturb+1;
		ep=&ep0[i&mask];
	}

	assert(ep->m_value==NULL);
	ep->m_hash=hash;
	ep->m_key=key;
	ep->m_value=value;
}

static ce_int_t map_resize(ce_map_t *mp,int minused)
{
	int new_size;
	ce_map_keys_t *old_keys;
	int i,old_size;

	for(new_size=MAP_INIT_D_SIZE;
	    new_size<=minused&&new_size>0;
	    new_size<<=1);

	if(new_size<=0)
	{
		return CE_ERROR;
	}

	old_keys=mp->ma_keys;

	mp->ma_keys=create_map_keys(new_size);

	if(mp->ma_keys==NULL)
	{
		mp->ma_keys=old_keys;
		return CE_ERROR;
	}
	old_size=MK_SIZE(old_keys);

	if(old_size==1)
	{
		free_map_keys(old_keys,mp->private_data,mp->kv_free_fun);
		return CE_OK;	
	}

	for(i=0;i<old_size;i++)
	{
		ce_map_entry_t *ep=&old_keys->m_key_entries[i];
		if(ep->m_value!=NULL)
		{
			assert(ep->m_key!=dummy);
			insertmap_clean(mp,ep->m_key,ep->m_hash,ep->m_value);
		}
	}

//	free_map_keys(old_keys,mp->private_data,mp->kv_free_fun);

	free(old_keys);

	mp->ma_keys->m_key_usable-=mp->ma_used;

	return CE_OK;
}

static ce_int_t insertion_resize(ce_map_t *mp)
{
	return map_resize(mp,GROWTH_RATE(mp->ma_used));
}

static ce_int_t insert_map(ce_map_t *mp,ce_str_t *key,ce_hash_t hash,
			      void *value)
{
	ce_map_entry_t *ep;
	ep=find_key_entry(mp,key,hash);
	if(ep==NULL)
	{
		return CE_ERROR;
	}
	if(ep->m_key&&ep->m_key!=dummy)
	{
		ep->m_value=value;
	}
	else
	{
		if(ep->m_key==NULL)
		{
			if(mp->ma_keys->m_key_usable<=0)
			{
				if(insertion_resize(mp)==CE_ERROR)
				{
					return CE_ERROR;
				}
				ep=find_empty_slot(mp,key, hash);
			}
			mp->ma_keys->m_key_usable-=1;
			assert(mp->ma_keys->m_key_usable>=0);
			ep->m_key=key->data;
			ep->m_value=value;
			ep->m_hash=hash;
		}
		
		else
		{
			assert(ep->m_key==dummy);
			ep->m_key=key->data;
			ep->m_value=value;
			ep->m_hash=hash; 
		}
	}
	mp->ma_used++;
	return CE_OK;
}

void * ce_map_get_value(ce_map_t *mp,ce_str_t *key)
{
	ce_hash_t hash;
	ce_map_entry_t *ep;
	hash=string_hash(key);
	ep=find_key_entry(mp,key,hash);
	if(ep->m_key==NULL||ep->m_key==dummy)
		return NULL;
	
	return ep->m_value;
}

ce_int_t ce_map_set_value(ce_map_t *mp,
			       ce_str_t *key,
			       void *value)
{
	assert(key&&key->data);
	ce_hash_t hash;
	hash=string_hash(key);

	return insert_map(mp,key,hash,value);
}

ce_int_t ce_map_del_value(ce_map_t *mp,ce_str_t *key)
{
	ce_hash_t hash;
	ce_map_entry_t *ep;
	assert(key&&key->data);

	hash=string_hash(key);

	ep=find_key_entry(mp,key,hash);
	if(ep==NULL)
		return CE_ERROR;
	if(mp->kv_free_fun)
	{
		mp->kv_free_fun(mp->private_data,ep->m_key,ep->m_value);
	}
	ep->m_key=dummy;
	ep->m_value=NULL;
	mp->ma_used--;
	return CE_OK;
}

void ce_clear_map(ce_map_t *mp)
{
	ce_map_keys_t *old_keys=mp->ma_keys;
	mp->ma_keys=EMPTY_KEYS;
	mp->ma_used=0;

	free_map_keys(old_keys,mp->private_data,mp->kv_free_fun);
}

int ce_map_coceins(ce_map_t *mp,ce_str_t *key)
{
	if(key==NULL||key->data==NULL)
		return 0;
	ce_hash_t hash=string_hash(key);

	ce_map_entry_t *ep=find_key_entry(mp,key,hash);
	return (ep->m_key!=NULL&&ep->m_key!=dummy);
}


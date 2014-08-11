/*
 * =====================================================================================
 *
 *       Filename:  ce_maptree.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/16/2012 11:09:48 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  
 *
 * =====================================================================================
 */
#include "ce_maptree.h"
#include "ce_string.h"
#include "ce_compiler.h"
#define TREEMAP_INIT_SIZE	8

static int str_key_cmp(u_char *key1,u_char *key2)
{
	if(key1==NULL&&key2==NULL)
		return 0;
	
	if(key1==NULL)
		return -1;
	if(key2==NULL)
		return 1;
	return ce_memn2cmp(key1,key2,ce_strlen(key1),ce_strlen(key2));
}

ce_int_t ce_init_maptree(ce_maptree_t *mpt,void *private_data,key_cmp_fun k_cmp,
		              maptree_node_clean_fun mpt_clean)
{
	assert(mpt);
	mpt->private_data=private_data;
	mpt->key_cmp=k_cmp?:str_key_cmp;
	mpt->node_clean=mpt_clean;
	mpt->maptree_node_root=RB_ROOT;
	return CE_OK;
}

ce_maptree_t *ce_create_maptree(ce_pool_t *pool,void *private_data,key_cmp_fun k_cmp,
				  maptree_node_clean_fun mpt_clean)
{
	ce_maptree_t *mpt=NULL;
	assert(pool);

	mpt=(ce_maptree_t*)ce_palloc(pool,sizeof(ce_maptree_t));
	
	if(mpt==NULL)
		return NULL;
	
	mpt->pool=pool;
	if(ce_init_maptree(mpt,private_data,k_cmp,mpt_clean)==CE_OK)
		return mpt;
		
	return NULL;
}

static void __do_clear_mptmap(ce_maptree_t *mpt)
{
	
	struct rb_node *root_node=mpt->maptree_node_root.rb_node;
	struct rb_node *l_node=rb_last(&mpt->maptree_node_root);
	struct rb_node *f_node=rb_first(&mpt->maptree_node_root);
	
	if(l_node==NULL||f_node==NULL)
		return;

	struct ce_maptree_node_t *mpt_node;
	if(mpt->node_clean)
	{
		while((l_node&&l_node!=root_node)||(f_node&&f_node!=root_node))
		{
			if(l_node!=root_node)
			{
				mpt_node=rb_entry(l_node,ce_maptree_node_t,rb_maptree_node);	
				mpt->node_clean(mpt->private_data,mpt_node->key,mpt_node->value);
				rb_erase(l_node,&mpt->maptree_node_root);
			}

			if(f_node!=root_node&&f_node!=l_node)
			{
				mpt_node=rb_entry(f_node,ce_maptree_node_t,rb_maptree_node);	
				mpt->node_clean(mpt->private_data,mpt_node->key,mpt_node->value);
				rb_erase(f_node,&mpt->maptree_node_root);
			}

			l_node=rb_last(&mpt->maptree_node_root);
			f_node=rb_first(&mpt->maptree_node_root);
		}

		if(l_node==root_node&&f_node==root_node)
		{

			mpt_node=rb_entry(l_node,ce_maptree_node_t,rb_maptree_node);	
			mpt->node_clean(mpt->private_data,mpt_node->key,mpt_node->value);
			rb_erase(l_node,&mpt->maptree_node_root);
		}
	}

}

ce_int_t ce_clear_maptree(ce_maptree_t *mpt)
{
	assert(mpt);
	__do_clear_mptmap(mpt);
	return ce_init_maptree(mpt,mpt->private_data,mpt->key_cmp,mpt->node_clean);	
}

void ce_destroy_maptree(ce_maptree_t *mpt)
{
	assert(mpt);
	__do_clear_mptmap(mpt);
}

#define MAKE_TEMP_NODE(node,k,v) (node->key=(k),node->value=(v)) 

static inline ce_maptree_node_t * rb_search_maptree(ce_maptree_t *mpt,
							u_char *key)
{

	struct rb_node *n=mpt->maptree_node_root.rb_node;
	struct ce_maptree_node_t *mpt_node;

	while(n)
	{
		mpt_node=rb_entry(n,ce_maptree_node_t,rb_maptree_node);
		int rc=mpt->key_cmp(key,mpt_node->key);
		if(rc<0)
			n=n->rb_left;
		else if(rc>0)
			n=n->rb_right;
		else
			return mpt_node;
	}
	return NULL;	
}

ce_int_t ce_maptree_set(ce_maptree_t *mpt,u_char *key,u_char *value)
{
	assert(mpt);
	assert(key);
	ce_maptree_node_t *mpt_node=NULL;
	struct rb_node *parent=NULL;
	struct rb_node **m_link=&mpt->maptree_node_root.rb_node;

	while(*m_link)
	{
		parent=*m_link;

		mpt_node=rb_entry(*m_link,ce_maptree_node_t,rb_maptree_node);
		int rc=mpt->key_cmp(key,mpt_node->key);
		if(rc<0)
			m_link=&(*m_link)->rb_left;
		else if(rc>0)
			m_link=&(*m_link)->rb_right;
		else
		{
			
			if(mpt->node_clean)
				mpt->node_clean(mpt->private_data,mpt_node->key==key?NULL:mpt_node->key,
						mpt_node->value==value?NULL:mpt_node->value);
			mpt_node->key=key;
			mpt_node->value=value;
			return CE_OK;
		}
	}

	mpt_node=(ce_maptree_node_t*)ce_palloc(mpt->pool,sizeof(ce_maptree_node_t));
        if(mpt_node==NULL)
		return CE_ERROR;
	mpt_node->key=key;
	mpt_node->value=value;
	
	rb_link_node(&mpt_node->rb_maptree_node,parent,m_link);
	rb_insert_color(&mpt_node->rb_maptree_node,&mpt->maptree_node_root);
	return CE_OK;
}

u_char *ce_maptree_get(ce_maptree_t *mpt,u_char *key)
{
	assert(mpt);
	assert(key);
	ce_maptree_node_t *mpt_node=NULL;
	mpt_node=rb_search_maptree(mpt,key);
	return (mpt_node==NULL?NULL:mpt_node->value);
}

void ce_maptree_del(ce_maptree_t *mpt,u_char *key)
{

	assert(mpt);
	assert(key);
	ce_maptree_node_t *mpt_node=NULL;
	mpt_node=rb_search_maptree(mpt,key);
	if(mpt_node)
	{
		if(mpt->node_clean)
		{
			mpt->node_clean(mpt->private_data,mpt_node->key,mpt_node->value);
		}
		rb_erase(&mpt_node->rb_maptree_node,&mpt->maptree_node_root);		
	}
}


/*
 *Authors: shajianfeng <csp001314@163.com>
 * */

#include <csp_kernel.h>

#ifndef KMANGRP_MAX
#define KMANGRP_MAX 0
#endif 

struct csp_kman_family 
{
	struct list_head anchor;
	__u16 family;
	void * private_data;
	struct csp_kman_cmd *cmd_arr;
};

static struct csp_kernel_manager g_kman,*g_kman_ptr = &g_kman;

static DEFINE_MUTEX(kmannetlink_mutex);

static inline void kman_lock(void)
{
	mutex_lock(&kmannetlink_mutex);
}

static inline void kman_unlock(void)
{
	mutex_unlock(&kmannetlink_mutex);
}

static struct csp_kman_family * find_kan_family(__u16 family)
{
	struct csp_kman_family *fa;
	
	list_for_each_entry(fa,&g_kman_ptr->famlies,anchor)
	{
		if(fa->family == family)
		{
			return fa;
		}
	}

	return NULL;
}

static inline int is_last_cmd(struct csp_kman_cmd *cmd)
{
	return (cmd->do_cmd==NULL&&cmd->done_cmd==NULL);
}
static struct csp_kman_cmd * find_kman_cmd(struct csp_kman_family *fa,__u16 cmd)
{
	struct csp_kman_cmd *cmd_res,*cmd_arr;
	__u16 i = 0;
	cmd_arr = fa->cmd_arr;

	while(!is_last_cmd(&cmd_arr[i]))
	{
		cmd_res = &cmd_arr[i];
		if(cmd_res->cmd==cmd)
		{
			return cmd_res;
		}
		i++;
	}

	return NULL;
} 

void register_kernel_manager_cmds(__u16 family,struct csp_kman_cmd *cmd_arr,void *private_data)
{
	struct csp_kman_family *fa = NULL;
	BUG_ON(!cmd_arr||find_kan_family(family));
	
	fa = kmem_cache_alloc(g_kman_ptr->kman_family_cachep,GFP_KERNEL);
	
	BUG_ON(!fa);
	
	fa->family = family;
	fa->cmd_arr = cmd_arr;
	fa->private_data = private_data;
	
	list_add_tail(&fa->anchor,&g_kman_ptr->famlies);

}

void unregister_kernel_manager_cmds(__u16 famliy)
{
	
	struct csp_kman_family *fa = find_kan_family(famliy);

	BUG_ON(!fa);
	
	list_del(&fa->anchor);

	kmem_cache_free(g_kman_ptr->kman_family_cachep,fa);
}

static int kmannetlink_rcv_skb(struct sk_buff *skb,struct nlmsghdr *nlh)
{
	struct csp_kman_family *fa;
	struct csp_kman_hdr *kman_hdr;
	struct csp_kman_cmd *kman_cmd;

	struct csp_record record;
	
	if(!KMAN_MSG_OK(nlh))
		return -EINVAL;
	
	kman_hdr = KMAN_MSG_HDR(nlh);
	
	if(kman_hdr->mark!=KMAN_HDR_MARK)
		return -EINVAL;
	
	fa = find_kan_family(kman_hdr->family);
	if(!fa)
		return -EINVAL;
	
	kman_cmd = find_kman_cmd(fa,kman_hdr->cmd);
	
	if(!kman_cmd)
		return -EINVAL;

	if(KMAN_MSG_HAS_RECORD(nlh))
	{
		init_record(&record,KMAN_MSG_RECORD(nlh),KMAN_MSG_RECORD_LEN(nlh),(void*)skb);			
		
		return kman_cmd->do_cmd(kman_hdr,&record,fa->private_data);
	}
	

	return kman_cmd->do_cmd(kman_hdr,NULL,fa->private_data);
}

static void kmannetlink_rcv(struct sk_buff *skb)
{
	kman_lock();
	netlink_rcv_skb(skb,kmannetlink_rcv_skb);
	kman_unlock();
}

static int __net_init kmannetlink_net_init(struct net *net)
{
	struct sock *sk;
	sk = netlink_kernel_create(net,NETLINK_KMAN,KMANGRP_MAX,kmannetlink_rcv,&kmannetlink_mutex,THIS_MODULE);

	if(!sk)
		return -ENOMEM;
	
	g_kman_ptr->netlink_sk = sk;

	return 0;
}

static void __net_exit kmannetlink_net_exit(struct net *net)
{
	netlink_kernel_release(g_kman_ptr->netlink_sk);
	g_kman_ptr->netlink_sk = NULL;
}

static struct pernet_operations kmannetlink_net_ops = {
	.init = kmannetlink_net_init,
	.exit = kmannetlink_net_exit,
};

static int kmannetlink_event(struct notifier_block *nb,unsigned long event,void *ptr)
{
	struct net_device *dev = (struct net_device*)ptr;
	
	switch(event)
	{
		case NETDEV_UP:
			printk("NETDEV_UP......\n");
			break;

		case NETDEV_DOWN:
			printk("NETDEV_DOWN......\n");
			break;
		case NETDEV_PRE_UP:
			printk("NETDEV_PRE_UP......\n");
			break;
		case NETDEV_POST_INIT:
			printk("NETDEV_POST_INIT......\n");
			break;
		case NETDEV_REGISTER:
			printk("NETDEV_REGISTER......\n");
			break;
		case NETDEV_CHANGE:
			printk("NETDEV_CHANGE......\n");
			break;

		case NETDEV_PRE_TYPE_CHANGE:
			printk("NETDEV_PRE_TYPE_CHANGE......\n");
			break;
		case NETDEV_GOING_DOWN:
			printk("NETDEV_GOING_DOWN......\n");
			break;
		case NETDEV_UNREGISTER:
			printk("NETDEV_UNREGISTER......\n");
			break;
		case NETDEV_UNREGISTER_BATCH:
			printk("NETDEV_UNREGISTER_BATCH......\n");
			break;

		default:
			printk("other .........\n");
			break;

	}

	return NOTIFY_DONE;
}

static struct notifier_block kmannetlink_dev_notifier = {
	.notifier_call = kmannetlink_event,
};

int __init init_kernel_manager(void)
{
	INIT_LIST_HEAD(&g_kman_ptr->famlies);

	if(register_pernet_subsys(&kmannetlink_net_ops))
		panic("init_kernel_manager: cannot initialize netlink\n");
	
	netlink_set_nonroot(NETLINK_KMAN,NL_NONROOT_RECV);
	
	register_netdevice_notifier(&kmannetlink_dev_notifier);
	
	g_kman_ptr->kman_family_cachep = kmem_cache_create("kman_family_cache",
							   sizeof(struct csp_kman_family),
							   0,
							   SLAB_PANIC,NULL);
	return 0;
}

void exit_kernel_manager(void)
{
	unregister_pernet_subsys(&kmannetlink_net_ops);
	unregister_netdevice_notifier(&kmannetlink_dev_notifier);
	
	if(g_kman_ptr->kman_family_cachep)
		kmem_cache_destroy(g_kman_ptr->kman_family_cachep);
}




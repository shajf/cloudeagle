/*
 *Authors: shajianfeng <csp001314@163.com>
 * */
#ifndef __VDEV_GROUP_H
#define __VDEV_GROUP_H
#if __KERNEL__

struct vdev_group;

struct vdev_group_ops
{
	bool 			(*vdev_do_forward)(struct sk_buff **skb);
	struct net_device*	(*vdev_do_find)(struct net_device *real_dev,u16 vdev_id);
	int 			(*vdev_hwaccel_rx)(struct sk_buff *skb,struct vdev_group *vdgrp,u16 id,int polling);
	bool 			(*vdev_hwaccel_do_receive)(struct sk_buff **skb);
	gro_result_t 		(*vdev_gro_receive)(struct napi_struct *napi,struct vdev_group *vdgrp,u16 id,struct sk_buff *skb);
	gro_result_t		(*vdev_gro_frags)(struct napi_struct *napi,struct vdev_group *vdgrp,u16 id);
};

struct vdev_group
{
	struct hlist_head vdevs;
	struct net_device *real_dev;
	struct vdev_group *forward_pair;
	struct vdev_group_ops *ops;
	unsigned int nr_vdevs;
	int property;
	int if_type;
	int work_mode;
	struct rcu_head rcu;
};

extern struct net_device * vdev_real_dev(const struct net_device *vdev);
extern u16  vdev_id(const struct net_device *vdev);

static inline void set_vdev_group_real_dev(struct vdev_group *vgrp,struct net_device *real_dev)
{
	vgrp->real_dev = real_dev;
}
#endif /*__KERNEL__*/
#endif /*__VDEV_GROUP_H*/

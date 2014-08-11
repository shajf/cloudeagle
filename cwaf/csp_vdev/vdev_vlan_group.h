/*
 *Authors: shajianfeng <csp001314@163.com>
 * */

#ifndef __VDEV_VLAN_GROUP_H
#define __VDEV_VLAN_GROUP_H

#if __KERNEL__

/*The additional bytes (on top of the Ethernet header)
 * that VLAN requires.*/
#define VLAN_HLEN	4

/*octets in one ethernet addr*/
#define VLAN_ETH_ALEN	6
/*total octets in ethernet header*/
#define VLAN_ETH_HLEN	18

/*Min. octets in fram sans FCS*/
#define VLAN_ETH_ZLEN	64

/*MAX. octets in payload*/
#define VLAN_ETH_DATA_LEN 1500

/*MAX. octets in fram sans FCS*/
#define VLAN_ETH_FRAME_LEN	1518

/*Priority Code Point*/
#define VLAN_PRIO_MASK	 0XE000
#define VLAN_PRIO_SHIFT	 13
/*Canonical Format Indicator*/
#define VLAN_CFI_MASK	 0X1000
#define VLAN_TAG_PRESENT VLAN_CFI_MASK
/*VLAN Identifier*/
#define VLAN_VID_MASK	0X0FFF
#define VLAN_N_VID	4096

#define VLAN_GROUP_ARRAY_SPLIT_PARTS  8
#define VLAN_GROUP_ARRAY_PART_LEN     (VLAN_N_VID/VLAN_GROUP_ARRAY_SPLIT_PARTS)

/*struct vlan_hdr --vlan header
 * @h_vlan_TCI: priority and VLAN ID
 * @h_vlan_encapsulated_proto: packet type ID or len
 **/
struct  vlan_hdr 
{
	__be16 h_vlan_TCI;
	__be16 h_vlan_encapsulated_proto;
};

/*struct vlan_ethhdr --vlan ethernet header (ethhdr + vlan_hdr)
 *@h_dest: destination ethernet address
 *@h_source: source ethernet address
 *@h_vlan_proto: ethernet protocol (always 0x8100)
 *@h_vlan_TCI: priority and VLAN ID
 *@h_vlan_encapsulated_proto:  packet type ID or len
 * */
struct vlan_ethhdr
{
	unsigned char h_dest[ETH_ALEN];
	unsigned char h_source[ETH_ALEN];
	__be16	      h_vlan_proto;
	__be16	      h_vlan_TCI;
	__be16 	      h_vlan_encapsulated_proto;
};

static inline struct vlan_ethhdr *vlan_eth_hdr(const struct sk_buff *skb)
{
	return (struct vlan_ethhdr*)skb_mac_header(skb);
}

struct vlan_group
{
	struct vdev_group vdgrp;
	struct net_device **vlan_devices_arrays[VLAN_GROUP_ARRAY_SPLIT_PARTS];
};

#define vlan_tx_tag_present(__skb) ((__skb)->vlan_tci & VLAN_TAG_PRESENT)
#define vlan_tx_tag_get(__skb)     ((__skb)->vlan_tci & ~VLAN_TAG_PRESENT)

static inline struct net_device *vlan_group_get_device(struct vlan_group *vg,u16 vlan_id)
{
	struct net_device **array;
	array = vg->vlan_devices_arrays[vlan_id/VLAN_GROUP_ARRAY_PART_LEN];

	return array? array[vlan_id%VLAN_GROUP_ARRAY_PART_LEN]:NULL;
}

static inline void vlan_group_set_device(struct vlan_group *vg,u16 vlan_id,struct net_device *dev)
{
	
	struct net_device **array;
	if(!vg)
		return;

	array = vg->vlan_devices_arrays[vlan_id/VLAN_GROUP_ARRAY_PART_LEN];
	
	array[vlan_id%VLAN_GROUP_ARRAY_PART_LEN] = dev;
}


/* __vlan_put_tag --regular VLAN tag inserting
 *@skb: skbuff to tag
 *@vlan_tci: VLAN TCI to insert
 *
 *Inserts the VLAN tag into @skb as part of the payload
 *Returns a VLAN tagged skb. If a new skb is created,@skb is freed.
 *
 *Following the skb_unshare() example,in case of error,the calling function
 *doest's have to worry about freeing the original skb.
 *
 * */
static inline struct sk_buff *__vlan_put_tag(struct sk_buff *skb,u16 vlan_tci)
{
	struct vlan_ethhdr *veth;

	if(skb_cow_head(skb,VLAN_HLEN) < 0)
	{
		kfree_skb(skb);
		return NULL;
	}
	
	veth = (struct vlan_ethhdr*)skb_push(skb,VLAN_HLEN);
	/*move the mac address to the beginning of the new header.*/
	memmove(skb->data,skb->data+VLAN_HLEN,2*VLAN_ETH_ALEN);
	skb->mac_header -= VLAN_HLEN;
	veth->h_vlan_proto = htons(ETH_P_8021Q);
	veth->h_vlan_TCI = htons(vlan_tci);
	skb->protocol = htons(ETH_P_8021Q);
	return skb;
}

/*__vlan_haccel_put_tag -- hardware accelerated VLAN inserting
 *@skb: skbuff to tag
 *@vlan_tci:VLAN TCI to tag
 *
 *Puts the VLAN TCI in @skb->vlan_tci and lets the device do the rest
 * */
static inline struct sk_buff * __vlan_hwaccel_put_tag(struct sk_buff *skb,u16 vlan_tci)
{
	skb->vlan_tci = VLAN_TAG_PRESENT |vlan_tci;
	return skb;
}

/*
 *vlan_put_tag -- inserts VLAN tag according to device features
 *@skb: skbuff to tag
 *@vlan_tci: VLAN TCI to insert
 *
 *Assumes skb->dev is the target that will xmit this frame.
 *Returns a VLAN tagged skb
 * */
static inline struct sk_buff * vlan_put_tag(struct sk_buff *skb,u16 vlan_tci)
{
	/*device support vlan*/
	if(skb->dev->features & NETIF_F_HW_VLAN_TX)
	{
		return __vlan_hwaccel_put_tag(skb,vlan_tci);
	}

	else
	{
		return __vlan_put_tag(skb,vlan_tci);
	}
}

/*__vlan_get_tag -- get ths VLAN ID that is part of the payload
 *@skb: skbuff to querry
 *@vlan_tci: buffer to store value
 *
 *Returns error if the skb is not of VLAN type
 * */
static inline int __vlan_get_tag(const struct sk_buff *skb,u16 *vlan_tci)
{
	struct vlan_ethhdr *veth = vlan_eth_hdr(skb);

	if(veth->h_vlan_proto != htons(ETH_P_8021Q))
	{
		return -EINVAL;
	}
	*vlan_tci = ntohs(veth->h_vlan_TCI);
	return 0;
}

/*__vlan_hwaccel_get_tag -- get ths VLAN ID that is in @skb->cb[]
 *@skb: skbuff to querry
 *@vlan_tci: buffer to store value
 *
 *Returns error if the skb is not set correctly
 * */
static inline int __vlan_hwaccel_get_tag(const struct sk_buff *skb,u16 *vlan_tci)
{
	if(vlan_tx_tag_present(skb))
	{
		*vlan_tci = vlan_tx_tag_get(skb);
		return 0;
	}
	else
	{
		*vlan_tci = 0;
		return -EINVAL;
	}
}

/*vlan_get_tag --  get the VLAN ID from the skb
 *@skb: skbuff to querry
 *@vlan_tci: buffer to store value
 *
 *Returns error if the skb is not VLAN tagged
 * */
static inline int vlan_get_tag(const struct sk_buff *skb,u16 *vlan_tci)
{
	if(skb->dev->features &NETIF_F_HW_VLAN_TX)
	{
		return __vlan_hwaccel_get_tag(skb,vlan_tci);
	}
	else
	{
		return __vlan_get_tag(skb,vlan_tci);
	}
}

/*vlan_get_protocol - get protocol EtherType
 *@skb: skbuff to querry
 *
 *Returns the EtherType of the packet,regardless of whether if it is vlan
 *encapsulated(normal or hardware accelerated) or not
 * 
 */
static inline __be16 vlan_get_protocol(const struct sk_buff *skb)
{
	__be16 protocol = 0;
	if(vlan_tx_tag_present(skb)||skb->protocol!=cpu_to_be16(ETH_P_8021Q))
		protocol = skb->protocol;
	else
	{
		__be16 proto,*protop;
		protop = skb_header_pointer(skb,offsetof(struct vlan_eth_hdr,h_vlan_encapsulated_proto),sizeof(proto),&proto);
		if(likely(protop))
			protocol = *protop;
	}

	return protocol;
} 
#endif /*__KERNEL__*/
#endif /*__VDEV_VLAN_GROUP_H*/

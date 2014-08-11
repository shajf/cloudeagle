/*
 *Authors: shajianfeng <csp001314@163.com>
 * */

#ifndef __VLAN_GVRP_H
#define __VLAN_GVRP_H

#ifdef __KERNEL__

extern int vlan_gvrp_request_join(const struct net_device *dev);
extern void vlan_gvrp_request_leave(const struct net_device *dev);
extern int vlan_gvrp_init_applicant(struct net_device *dev);
extern void vlan_gvrp_uninit_applicant(struct net_device *dev);
extern int vlan_gvrp_init(void);
extern void vlan_gvrp_uninit(void);
#endif /*__KERNEL__*/
#endif /*__VLAN_GVRP_H*/

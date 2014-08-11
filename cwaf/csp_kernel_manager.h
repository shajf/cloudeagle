/*
 *Authors: shajianfeng <csp001314@163.com>
 * */
#ifndef _CSP_KERNEL_MANAGER_H
#define _CSP_KERNEL_MANAGER_H
#include <csp_kernel.h>

#define NETLINK_KMAN NETLINK_UNUSED
#define KMAN_HDR_MARK 0XFE

struct csp_kman_hdr
{
	__u16 family;
	__u16 cmd;
	__u16 flags;
	__u16 mark;
};
#ifdef __KERNEL__
struct csp_kman_cmd
{
	__u16 cmd;
	int (*do_cmd)(struct csp_kman_hdr *hdr,struct csp_record *record,void *private_data);
	int (*done_cmd)(struct csp_kman_hdr *hdr,struct csp_record *record,void *private_data);
};


struct csp_kernel_manager
{
	struct list_head famlies;
	struct kmem_cache *kman_family_cachep;
	struct sock * netlink_sk;
};

#define KMAN_MSG_MIN_LEN (NLMSG_HDRLEN+sizeof(struct csp_kman_hdr)) 
#define KMAN_MSG_HAS_RECORD(nlh) ((nlh)->nlmsg_len>KMAN_MSG_MIN_LEN)
#define KMAN_MSG_HDR(nlh) ((struct csp_kman_hdr*)(nlh+1))
#define KMAN_MSG_RECORD(nlh) (((void*)(nlh))+KMAN_MSG_MIN_LEN)
#define KMAN_MSG_OK(nlh) (nlh->nlmsg_len>=KMAN_MSG_MIN_LEN)
#define KMAN_MSG_RECORD_LEN(nlh) ((nlh)->nlmsg_len-KMAN_MSG_MIN_LEN)

extern int init_kernel_manager(void);
extern void exit_kernel_manager(void);
extern void register_kernel_manager_cmds(__u16 family,struct csp_kman_cmd *cmd_arr,void *private_data);
extern void unregister_kernel_manager_cmds(__u16 family);
#endif
#endif /*_CSP_KERNEL_MANAGER_H*/

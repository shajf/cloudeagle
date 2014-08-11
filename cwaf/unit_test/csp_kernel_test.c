#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <csp_kernel.h>

static void write_msg(struct nlmsghdr *nlhdr)
{
	size_t len=0,cur_len=0;
	struct csp_record record;
	struct csp_kman_hdr *kman_hdr;
	void *data = NLMSG_DATA(nlhdr);
	int intv;
	long longv;
	char *str;
	size_t sz;

	kman_hdr = (struct csp_kman_hdr*)data;
	kman_hdr->family = 1234;
	kman_hdr->cmd = 1;
	kman_hdr->flags = 1234;
	kman_hdr->mark = KMAN_HDR_MARK;
	
	cur_len = sizeof(struct csp_kman_hdr);
	len +=cur_len;
	data+=cur_len;
	
	init_record(&record,data,1024-NLMSG_LENGTH(len),NULL);
	
	record_append_start(&record);

	record_append_int(&record,897879);
	record_append_long(&record,897879);
	record_append_string(&record,"shajianfeng");

	record_append_end(&record);
	
	len += (record.record_size);

	init_record(&record,data,record.record_size,NULL);
	record_get_start(&record);
	record_get_int(&record,&intv);
	record_get_long(&record,&longv);
	record_get_string(&record,&str,&sz);
	record_get_end(&record);

	printf("intv==%d,longv==%d,str==%s,strlen==%d\n",intv,longv,str,sz);
	nlhdr->nlmsg_len = NLMSG_LENGTH(len);
	nlhdr->nlmsg_type = 32;
	nlhdr->nlmsg_flags =NLM_F_REQUEST;
	nlhdr->nlmsg_pid = getpid();
	
}

int main(int argc, char ** argv)
{
	
	struct sockaddr_nl saddr,daddr;
	struct nlmsghdr *nlhdr = NULL;
	struct msghdr  msg;
	char * text = "shajianfeng";

	struct iovec iov;

	int sd;

	int ret =-1;

	sd = socket(AF_NETLINK,SOCK_RAW,1);


	memset(&saddr,0,sizeof(saddr));
	memset(&daddr,0,sizeof(daddr));

	saddr.nl_family = AF_NETLINK;
	saddr.nl_pid = getpid();
	saddr.nl_groups = 0;

	bind(sd,(struct sockaddr*)&saddr,sizeof(saddr));

	daddr.nl_family = AF_NETLINK;
	daddr.nl_pid = 0;
	daddr.nl_groups = 0;

	nlhdr = (struct nlmsghdr*)malloc(1024);

	memcpy(NLMSG_DATA(nlhdr),text,strlen(text));
	memset(&msg,0,sizeof(struct msghdr));

	write_msg(nlhdr);

	iov.iov_base = (void*)nlhdr;
	iov.iov_len = nlhdr->nlmsg_len;
	msg.msg_name = (void*)&daddr;
	msg.msg_namelen = sizeof(daddr);
	msg.msg_iov = &iov;
	msg.msg_iovlen =1;

	ret = sendmsg(sd,&msg,0);

	if(ret==-1)
	{
		perror("sendmsg error:");
	}

	free(nlhdr);	
	close(sd);

	return ret;

}

#ifdef __KERNEL__
#include <csp_kernel.h>

static int do_cmd_fn(struct csp_kman_hdr *kman_hdr,struct csp_record *record,void *private_data)
{
	
	int intv =0;
	long longv =0;
	char *strv;
	size_t strvlen=0;
	int rc =0;
	printk("print kernel manager header info:\n");
	printk("family:%d\n",kman_hdr->family);
	printk("cmd   :%d\n",kman_hdr->cmd);
	printk("flags :%d\n",kman_hdr->flags);
	printk("mark  :%d\n",kman_hdr->mark);
	
	record_get_start(record);
	printk("print record header info:\n");
	printk("start_mark :%d\n",record->cur_msg_hdr->start_mark);
	printk("record_size:%d\n",record->cur_msg_hdr->record_size);

	rc = record_get_int(record,&intv);
	rc = record_get_long(record,&longv);
	rc = record_get_string(record,&strv,&strvlen);

	printk("print record content info:\n");
	printk("intv   :%d\n",intv);
	printk("longv  :%d\n",longv);
	printk("strv   :%s\n",strv);
	printk("strvlen:%d\n",strvlen);
	record_get_end(record);
	
	return 0;
}

static int done_cmd_fn(struct csp_kman_hdr *kman_hdr,struct csp_record *record,void *private_data)
{
	kman_hdr = kman_hdr;
	record = record;
	private_data = private_data;
	return 0;
}
static struct csp_kman_cmd cmds[]={
	{1,do_cmd_fn,done_cmd_fn},
	{0,NULL,NULL}
};

void init_kman_test(void)
{
	register_kernel_manager_cmds(1234,cmds,NULL);
}
void exit_kman_test(void)
{
	unregister_kernel_manager_cmds(1234);
}
#endif /*__KERNEL__*/

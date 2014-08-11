/*
 *Authors: shajianfeng <csp001314@163.com>
 * */
#ifndef CSP_KERNEL_VERSION
#define CSP_KERNEL_VERSION	"0"
#endif 

#include <csp_kernel.h>

static int __init csp_module_init(void)
{
	
	/*initialize kernel manger*/
	init_kernel_manager();

	init_kman_test();

	return 0;
}

static void __exit csp_module_exit(void)
{
	
	exit_kman_test();
	exit_kernel_manager();

}

module_init(csp_module_init);
module_exit(csp_module_exit);

MODULE_AUTHOR("CSP ALL MEMBERS");
MODULE_DESCRIPTION("CSP KERNEL DRIVER");
MODULE_LICENSE("GPL");
MODULE_VERSION(CSP_KERNEL_VERSION);

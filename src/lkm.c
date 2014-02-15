
#include <linux/kernel.h>
#include <linux/module.h>

static int __init my_init(void)
{
    printk(KERN_INFO "I am alive.\n");
    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "Bye.\n");
}

module_init(my_init)
module_exit(my_exit)

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("UW-Madison");

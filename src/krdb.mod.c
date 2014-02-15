
#include <linux/kernel.h>
#include <linux/module.h>
#include <net/genetlink.h>

#include "kr_common.h"

static struct sock * kr_nlsock = NULL;

static void kr_nl_recv(struct sk_buff *skb) {
    printk(KERN_INFO "krdb: recv\n");
}

static struct netlink_kernel_cfg kr_nl_cfg = {
    .input = kr_nl_recv
};

static int __init kr_module_init(void)
{
    kr_nlsock = netlink_kernel_create(&init_net, KR_NETLINK_USER, &kr_nl_cfg);

    if (!kr_nlsock) {
        printk(KERN_ALERT "krdb: Error creating socket\n");
        return -10;
    }

    printk(KERN_INFO "krdb: Loaded\n");
    return 0;
}

static void __exit kr_module_exit(void)
{
    if (kr_nlsock)
        netlink_kernel_release(kr_nlsock);

    printk(KERN_INFO "krdb: Unloading...\n");
}

module_init(kr_module_init)
module_exit(kr_module_exit)

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("UW-Madison");

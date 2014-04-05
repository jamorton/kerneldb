
#include <linux/kernel.h>
#include <linux/module.h>
#include <net/genetlink.h>

#include "kr_common.h"
#include "internal.h"
#include "user.h"
#include "db.h"

static struct sock * kr_nlsock = NULL;

static void kr_nl_recv(struct sk_buff *skb) {
    struct nlmsghdr * nlh = (struct nlmsghdr *)skb->data;
    kr_dataptr data = nlmsg_data(nlh);
    //int size = nlmsg_len(nlh);
    int pid = nlh->nlmsg_pid;

    printk(KERN_INFO "received data from client %d\n", pid);

    switch (nlh->nlmsg_type) {

    case KR_COMMAND_OPEN: {
        const char * path = (const char *)data;
        KrUser * user = kr_user_get(pid);
        printk(KERN_INFO "KR_COMMAND_OPEN - \"%s\"\n", path);
        if (user->db != NULL)
            ; /* user already has a DB open... */
        kr_db_open(&user->db, path);
        break;
    }

    case KR_COMMAND_CLOSE: {
        KrUser * user = kr_user_get(pid);
        printk(KERN_INFO "KR_COMMAND_CLOSE\n");
        if (user->db == NULL)
            ; /* no db to close... */
        kr_db_close(user->db);
        break;
    }

    case KR_COMMAND_PUT: {
        KrUser * user = kr_user_get(pid);

        KrSlice key = { *(uint64_t *)data, data + 8 };
        kr_dataptr valstart = data + 8 + key.size;
        KrSlice val = { *(uint64_t *)valstart, valstart + 8 };

        printk(KERN_INFO "KR_COMMAND_PUT: key %.*s, val %llu bytes\n",  (int)key.size, key.data, val.size);

        kr_db_put(user->db, key, val);
        break;
    }

    case KR_COMMAND_GET: {
        KrUser * user = kr_user_get(pid);

        KrSlice val, key = { *(uint64_t *)data, data + 8 };
        kr_db_get(user->db, key, &val);
        // todo: send result

        printk(KERN_INFO "KR_COMMAND_GET: %.*s\n",  (int)key.size, key.data);
        break;
    }

    case KR_COMMAND_NOP:
    default:
        break;
    }
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

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("UW-Madison");


#include <linux/kernel.h>
#include <linux/module.h>
#include <net/genetlink.h>

#include "kr_common.h"
#include "internal.h"
#include "db.h"
#include "outbuf.h"

static struct sock * kr_nlsock = NULL;

static void kr_nl_send(int pid, int seq, int cmd, void* data, size_t len)
{
    struct sk_buff* skb = nlmsg_new(len, GFP_KERNEL);
    struct nlmsghdr* nlh = nlmsg_put(skb, pid, seq, cmd, len, 0);
    char* dest = nlmsg_data(nlh);
    memcpy(dest, data, len);
    nlmsg_unicast(kr_nlsock, skb, pid);
}

/**
 * Called by kernel for each new netlink socket message receieved
 */
static void kr_nl_recv(struct sk_buff *skb) {

    struct nlmsghdr * nlh = (struct nlmsghdr *)skb->data;
    kr_dataptr data = nlmsg_data(nlh);

    int size = nlmsg_len(nlh);
    int pid = nlh->nlmsg_pid;
    int seq = nlh->nlmsg_seq;

    KrDb* db = NULL; /* used differently by all switch cases */

    KrOutbuf outbuf = kr_outbuf(sizeof(u64));

    /* helper macros for reading message data */
#define NEXT_U8()  (*(u8* )((data += sizeof(u8))  - sizeof(u8)))
#define NEXT_U64() (*(u64*)((data += sizeof(u64)) - sizeof(u64)))
#define NEXT_PTR(len) ((data += len) - len)
#define GET_DB() do {                                       \
        db = kr_db_from_id(NEXT_U8());                      \
        if (!db) {                                          \
            printk(KERN_INFO "GET_DB WITH INVALID ID\n");   \
            return;                                         \
        }                                                   \
    } while (0)

    /* -------------- command type switch */

    switch (nlh->nlmsg_type) {

        //--------------------------------------------------
        // Command: OPEN
        //--------------------------------------------------
    case KR_COMMAND_OPEN: {
        const char * path = (const char *)data;
        printk(KERN_INFO "KR_COMMAND_OPEN - \"%s\"\n", path);
        kr_db_open(&db, path);
        kr_nl_send(pid, seq, KR_COMMAND_OPEN, &db->id, sizeof(db->id));
        break;
    }

        //--------------------------------------------------
        // Command: Close
        //--------------------------------------------------
    case KR_COMMAND_CLOSE: {
        GET_DB();
        printk(KERN_INFO "KR_COMMAND_CLOSE\n");
        kr_db_close(db);
        break;
    }

        //--------------------------------------------------
        // Command: PUT
        //--------------------------------------------------
    case KR_COMMAND_PUT: {

        u64 sz;
        GET_DB();

        sz = NEXT_U64();
        KrSlice key = { sz, NEXT_PTR(sz) };
        sz = NEXT_U64();
        KrSlice val = { sz, NEXT_PTR(sz) };
        printk(KERN_INFO "KR_COMMAND_PUT: key %.*s, data sz: %llu\n",  (int)key.size, key.data, val.size);
        kr_db_put(db, key, val);
        break;
    }

        //--------------------------------------------------
        // Command: GET
        //--------------------------------------------------
    case KR_COMMAND_GET: {
        GET_DB();
        u64 size;
        KrSlice val, key = { NEXT_U64(), data };
        //printk(KERN_INFO "KR_COMMAND_GET: key %.*s\n",  (int)key.size, key.data);
        kr_db_get(db, key, &outbuf, &size);
        *(u64 *)outbuf.data = size;
        kr_nl_send(pid, seq, KR_COMMAND_GET, outbuf.data, sizeof(u64) + size);
        break;
    }

        //--------------------------------------------------
        // Command: BENCH
        //--------------------------------------------------
    case KR_COMMAND_BENCH: {
        GET_DB();
        printk(KERN_INFO "Running kr_bench(\"%s\")...\n", db->path);
        kr_bench(db);
        printk(KERN_INFO "...done.\n");
    }

    case KR_COMMAND_NOP:
    default:
        printk(KERN_INFO "Received unknown command\n");
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

MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("UW-Madison");

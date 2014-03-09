
/* functions for communicating with the kernel module
   thru netlink sockets */

#include "internal.h"

void conn_setup(KrClient * cl)
{
    cl->sock = socket(PF_NETLINK, SOCK_RAW, KR_NETLINK_USER);
    if (cl->sock < 0)
        ;

    memset(&cl->src_addr, 0, sizeof(cl->src_addr));
    memset(&cl->dst_addr, 0, sizeof(cl->dst_addr));

    cl->src_addr.nl_family = AF_NETLINK;
    cl->src_addr.nl_pid = getpid();
    cl->src_addr.nl_groups = 0;

    bind(cl->sock, (struct sockaddr *)&cl->src_addr, sizeof(cl->src_addr));

    cl->dst_addr.nl_family = AF_NETLINK;
    cl->dst_addr.nl_pid = 0;
    cl->dst_addr.nl_groups = 0;

    /*
    cl->iov.iov_base = (void *)nlh;

    cl->msg.msg_name = (void *)&cl->dst_addr;
    cl->msg.msg_namelen = sizeof(cl->dst_addr);
    cl->msg.msg_iov = &cl->iov;
    cl->msg.msg_iovlen = 1;
    */
}

void conn_teardown(KrClient * cl)
{
    close(cl->sock);
}

/*

#define MAX_PAYLOAD 1024

struct nlmsghdr * nlh;
struct msghdr msg;

*/

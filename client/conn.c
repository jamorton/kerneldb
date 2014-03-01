
/* functions for communicating with the kernel module
   thru netlink sockets */

#include "internal.h"

void conn_setup(KrClient * cl)
{
    cl->sock = socket(PF_NETLINK, SOCK_RAW, KR_NETLINK_USER);

    memset(&cl->src_addr, 0, sizeof(cl->src_addr));
    memset(&cl->dst_addr, 0, sizeof(cl->dst_addr));

    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 0;

    bi2nd(cl->sock, (struct sockaddr *)&cl->src_addr, sizeof(cl->src_addr));

    dst_addr.nl_family = AF_NETLINK;
    dst_addr.nl_pid = 0;
    dst_addr.nl_groups = 0;

    cl->iov.iov_base = (void *)nlh;

    cl->msg.msg_name = (void *)&cl->dst_addr;
    cl->msg.msg_namelen = sizeof(cl->dst_addr);
    cl->msg.msg_iov = &cl->iov;
    cl->msg.msg_iovlen = 1;
}

void conn_teardown(KrClient * cl)
{
    close(cl->sockfd);
}

/*

#define MAX_PAYLOAD 1024

struct nlmsghdr * nlh;
struct msghdr msg;

*/

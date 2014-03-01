
#ifndef KR_INTERNAL_H
#define KR_INTERNAL_H

#include "kr_client.h"
#include "kr_common.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#include <sys/socket.h>
#include <linux/netlink.h>

struct KrClient {
    char * dev;

    /* netlink connection related */
    int sock;
    struct sockaddr_nl src_addr;
    struct sockaddr_nl dst_addr;
    struct iovec iov;
    struct msghdr msg;
};

void conn_setup    (KrClient * cl);
void conn_teardown (KrClient * cl);

#endif

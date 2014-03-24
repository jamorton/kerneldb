
#ifndef KR_INTERNAL_H
#define KR_INTERNAL_H

#include "kr_client.h"
#include "kr_common.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#include <netlink/netlink.h>

struct KrClient {
    char * dev;
    struct nl_sock* sock;
};

int conn_create  (KrClient* cl);
int conn_destroy (KrClient* cl);
int conn_send    (KrClient* cl, int cmd, void* data, size_t len);

#endif

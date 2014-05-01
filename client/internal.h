
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

typedef struct KrClient {
    char * dev;
    struct nl_sock* sock;
    uint8_t db_id;
    struct nl_msg* conn_msg;
} KrClient;

typedef struct KrMsg {
    struct nl_msg* _nlmsg;
    char* data;
    size_t len;
} KrMsg;

int  conn_create     (KrClient* cl);
int  conn_destroy    (KrClient* cl);
int  conn_send       (KrClient* cl, int cmd, void* data, size_t len);

int  conn_wait_reply (KrClient* cl, KrMsg* msg_out);
void conn_msg_done   (KrMsg* msg);

#endif

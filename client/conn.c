
/* functions for communicating with the kernel module
   thru netlink sockets */

#include "internal.h"

#include <netlink/msg.h>

static int conn_recv_cb(struct nl_msg* msg, void* arg)
{
    KrClient* client = (KrClient*)arg;
    /* add a reference to the message and store it for processing
       by conn_wait_reply */
    nlmsg_get(msg);
    client->conn_msg = msg;
    return NL_OK;
}

int conn_create(KrClient * cl)
{
    cl->sock = nl_socket_alloc();
    nl_connect(cl->sock, KR_NETLINK_USER);
    nl_socket_modify_cb(cl->sock, NL_CB_VALID, NL_CB_CUSTOM, conn_recv_cb,
        (void*)cl);
    nl_socket_modify_cb(cl->sock, NL_CB_INVALID, NL_CB_VERBOSE, NULL, NULL);
    nl_socket_disable_auto_ack(cl->sock);
    cl->conn_msg = NULL;
    return 0;
}

int conn_wait_reply(KrClient* cl, KrMsg* msg_out)
{
    /* libnl will call conn_recv_cb (above) for each new message, which will
       store the retrieved message in cl->conn_msg temporarily. We construct
       the KrMsg using that here */

    nl_recvmsgs_default(cl->sock);
    if (!cl->conn_msg)
        return -1;

    struct nl_msg* nlmsg = cl->conn_msg;
    cl->conn_msg = NULL;

    msg_out->_nlmsg = nlmsg;

    struct nlmsghdr* hdr = nlmsg_hdr(nlmsg);
    msg_out->data = nlmsg_data(hdr);
    msg_out->len = nlmsg_datalen(hdr);

    return 0;
}

void conn_msg_done(KrMsg* msg)
{
    nlmsg_free(msg->_nlmsg);
}

int conn_destroy(KrClient * cl)
{
    nl_close(cl->sock);
    return 0;
}

int conn_send (KrClient* cl, int cmd, void* data, size_t len)
{
    nl_send_simple(cl->sock, cmd, 0, data, len);
    return 0;
}

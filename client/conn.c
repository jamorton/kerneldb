
/* functions for communicating with the kernel module
   thru netlink sockets */

#include "internal.h"

int conn_create(KrClient * cl)
{
    cl->sock = nl_socket_alloc();
    nl_connect(cl->sock, KR_NETLINK_USER);
    return 0;
}

int conn_destroy(KrClient * cl)
{
    nl_close(cl->sock);
    return 0;
}

int conn_send (KrClient* cl, int cmd, void* data, size_t len)
{
    int flags = (cmd == KR_COMMAND_GET) ? NLM_F_REQUEST : 0;
    nl_send_simple(cl->sock, cmd, flags, data, len);
    return 0;
}

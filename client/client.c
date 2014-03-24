
#include "internal.h"

KrClient * kr_open(const char * dev)
{
    KrClient * client = (KrClient *)malloc(sizeof(KrClient));

    size_t len = strlen(dev);
    client->dev = malloc(len + 1);
    strcpy(client->dev, dev);

    conn_create(client);
    conn_send(client, KR_COMMAND_OPEN, client->dev, len);

    return client;
}

void kr_close(KrClient * client)
{
    conn_send(client, KR_COMMAND_CLOSE, NULL, 0);
    conn_destroy(client);
    free(client->dev);
    free(client);
}

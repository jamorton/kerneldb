
#include "internal.h"

KrClient * kr_open(const char * dev)
{
    KrClient * client = (KrClient *)malloc(sizeof(KrClient));

    client->dev = malloc(strlen(dev));
    strcpy(client->dev, dev);

    conn_setup(client);

    return client;
}

void kr_close(KrClient * client)
{
    conn_teardown(client);
    free(client->dev);
    free(client);
}

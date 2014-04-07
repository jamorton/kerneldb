
#include "internal.h"

KrClient * kr_open(const char * dev)
{
    KrClient * client = (KrClient *)malloc(sizeof(KrClient));

    size_t len = strlen(dev);
    client->dev = malloc(len + 1);
    strcpy(client->dev, dev);

    printf("Creating connection...\n");
    conn_create(client);
    printf("Opening db...\n");
    conn_send(client, KR_COMMAND_OPEN, client->dev, len);

    return client;
}

void kr_put(KrClient * client, size_t keysz, void* key, size_t valsz, void* val)
{
    printf("put: keysz %zu, key %s, valsz %zu, val %s\n", keysz, (char*)key, valsz, (char*) val);
    //build the message first
    //msg layout:  8byte key size followed by key data followed by 8byte val sz followed by val data
    size_t msglen = 8 + keysz + 8 + valsz;
    void* msgbgn = malloc(msglen);
    void* msgcurr = msgbgn;
    memcpy(msgcurr, (char*)&keysz, 8);
    msgcurr += 8;
    memcpy(msgcurr, (char*)key, keysz);
    msgcurr += keysz;
    memcpy(msgcurr, (char*)&valsz, 8);
    msgcurr += 8;
    memcpy(msgcurr, (char*)val, valsz);

    conn_send(client, KR_COMMAND_PUT, msgbgn, msglen);
}

void kr_get(KrClient* client, size_t keysz, void* key, size_t* valszout, void** valout)
{
    printf("get: keysz %zu, key %s\n", keysz, (char*) key);
    size_t msglen = 8 + keysz;
    void* msgbgn = malloc(msglen);
    memcpy(msgbgn, (char*)&keysz, 8);
    memcpy(msgbgn + 8, (char*)key, 8);

    conn_send(client, KR_COMMAND_GET, msgbgn, msglen);
}

void kr_close(KrClient * client)
{
    conn_send(client, KR_COMMAND_CLOSE, NULL, 0);
    conn_destroy(client);
    free(client->dev);
    free(client);
}

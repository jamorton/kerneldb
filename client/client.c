
#include "internal.h"

KrClient * kr_open(const char * dev)
{
    KrClient * client = (KrClient *)malloc(sizeof(KrClient));
    KrMsg msg;

    size_t len = strlen(dev);
    client->dev = malloc(len + 1);
    strcpy(client->dev, dev);

    printf("Creating connection...\n");
    conn_create(client);

    printf("Opening db \"%s\"...\n", client->dev);
    conn_send(client, KR_COMMAND_OPEN, client->dev, len + 1);

    conn_wait_reply(client, &msg);
    client->db_id = *(uint8_t*)msg.data;
    conn_msg_done(&msg);

    return client;
}

void kr_put(KrClient * client, size_t keysz, void* key, size_t valsz, void* val)
{
    //build the message first
    //msg layout:  8byte key size followed by key data followed by 8byte val sz followed by val data
    size_t msglen = 8 + keysz + 8 + valsz + 1;
    void* msgbgn = malloc(msglen);
    *(uint8_t*)msgbgn = client->db_id;
    void* msgcurr = msgbgn + 1;
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
    size_t msglen = 8 + keysz + 1;
    void* msgbgn = malloc(msglen);
    *(uint8_t*)msgbgn = client->db_id;
    memcpy(msgbgn + 1, (char*)&keysz, 8);
    memcpy(msgbgn + 1 + 8, (char*)key, keysz);

    conn_send(client, KR_COMMAND_GET, msgbgn, msglen);

    KrMsg msg;
    conn_wait_reply(client, &msg);

    uint64_t len = *(uint64_t*)msg.data;

    *valszout = (size_t)len;
    *valout = malloc(len);
    memcpy(*valout, msg.data + 8, len);

    conn_msg_done(&msg);
}

void kr_close(KrClient * client)
{
    conn_send(client, KR_COMMAND_CLOSE, &client->db_id, sizeof(client->db_id));
    conn_destroy(client);
    free(client->dev);
    free(client);
}

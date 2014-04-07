#include "kr_client.h"

int main()
{
    KrClient * client = kr_open("/dev/sda");
    char key[6] = "Test1";
    char val[6] = "Allen";
    size_t* valoutsz = NULL;
    void** valout = NULL;
    kr_put(client, sizeof(char) * 6, key, sizeof(char) * 6, val);
    kr_get(client, 6, key, valoutsz, valout);
    kr_close(client);
    return 0;
}

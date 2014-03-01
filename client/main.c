
#include "kr_client.h"

int main()
{
    KrClient * client = kr_open("/dev/sda");
    kr_close(client);
    return 0;
}

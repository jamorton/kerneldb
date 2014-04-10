
#include "kr_client.h"
#include "internal.h"

#include <string.h>

static void exit_help(void)
{
    printf(
        "Usage: krcl [db_path] [command]\n"
        "Commands:\n"
        "  bench - run the benchmark\n"
    );
    exit(0);
}

int main(int argc, char* argv[])
{
    /*
    KrClient * client = kr_open("/dev/sda");
    char key[6] = "Test1";
    char val[6] = "Allen";
    size_t* valoutsz = NULL;
    void** valout = NULL;
    kr_put(client, sizeof(char) * 6, key, sizeof(char) * 6, val);
    kr_get(client, 6, key, valoutsz, valout);
    kr_close(client);
    */

    if (argc < 2)
        exit_help();

    KrClient* client = kr_open(argv[1]);

    if (strcmp(argv[2], "bench") == 0) {
        conn_send(client, KR_COMMAND_BENCH, NULL, 0);
    } else {
        printf("Usage: krcl bench [db_path]\n");
    }

    kr_close(client);

    return 0;
}

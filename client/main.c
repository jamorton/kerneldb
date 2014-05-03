
#include "kr_client.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

static void exit_help(void)
{
    printf(
        "Usage: krcl [db_path] [command]\n"
        "Commands:\n"
        "  bench - run the benchmark\n"
        "  put [key] [val]\n"
        "  get [key]\n"
    );
    exit(0);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        exit_help();

    KrClient* client = kr_open(argv[1]);

    if (strcmp(argv[2], "bench") == 0) {
        //----
    }
    else if (strcmp(argv[2], "put") == 0) {
        kr_put(client, strlen(argv[3]), argv[3], strlen(argv[4]), argv[4]);
    }
    else if (strcmp(argv[2], "get") == 0) {
        size_t len;
        char * data;
        kr_get(client, strlen(argv[3]), argv[3], &len, (void**)&data);
        printf("get: %.*s\n", (int)len, data);
        free(data);
    }
    else
        exit_help();

    kr_close(client);

    return 0;
}

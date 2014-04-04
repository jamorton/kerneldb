
#ifndef KR_DB_H
#define KR_DB_H

#include "internal.h"

struct KrDevice;

typedef struct KrDb {
    char path[255];
    int refcnt;
    struct KrDevice* dev;
} KrDb;

int kr_db_open  (KrDb ** dp, const char * path);
int kr_db_close (KrDb * db);
int kr_db_put   (KrDb * db, KrSlice key, KrSlice val);
int kr_db_get   (KrDb * db, KrSlice key, KrSlice * out);

#endif

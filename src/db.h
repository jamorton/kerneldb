
#ifndef KR_DB_H
#define KR_DB_H

#include "slice.h"

typedef struct KrDb {
    char path[255];
    char val[255];
    int refcnt;
} KrDb;

int kr_db_open  (KrDb ** db, const char * path);
int kr_db_close (KrDb * db);
int kr_db_put   (KrDb * db, KrSlice key, KrSlice val);
int kr_db_get   (KrDb * db, KrSlice key, KrSlice * val);

#endif

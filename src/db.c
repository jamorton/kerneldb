
#include "db.h"

#define MAX_DB 256

static KrDb databases[MAX_DB];

KrDb * kr_db_open(const char * path) {

    KrDb * empty = NULL;

    /* try to find an existing DB with this path */
    for (int i = 0; i < MAX_DB; i++) {
        if (databases[i].refcnt == 0)
            empty = &databases[i];
        else if (strcmp(databases[i].path, path) == 0) {
            databases[i].refcnt++;
            return &databases[i];
        }
    }

    /* all databses taken... abort? */
    if (empty == NULL)
        ;

    /* no existing db found... make a new one */
    strcpy(empty->path, path);
    empty->refcnt = 1;
    return empty;
}

void kr_db_close (KrDb * db)
{
    db->refcnt--;
}

int kr_db_put  (KrDb * db, KrSlice key, KrSlice val)
{
    return 0;
}

int kr_db_get  (KrDb * db, KrSlice * out)
{
    return 0;
}

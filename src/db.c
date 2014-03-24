
#include "db.h"

#define MAX_DB 256

static KrDb databases[MAX_DB];

int kr_db_open(KrDb ** db, const char * path) {

    KrDb * empty = NULL;

    /* try to find an existing DB with this path */
    for (int i = 0; i < MAX_DB; i++) {
        if (databases[i].refcnt == 0)
            empty = &databases[i];
        else if (strcmp(databases[i].path, path) == 0) {
            databases[i].refcnt++;
            *db = &databases[i];
            return 0;
        }
    }

    /* all databses taken... abort? */
    if (empty == NULL)
        ;

    /* no existing db found... make a new one */
    strcpy(empty->path, path);
    empty->refcnt = 1;
    *db = empty;
    return 0;
}

int kr_db_close (KrDb * db)
{
    db->refcnt--;
    return 0;
}

int kr_db_put (KrDb * db, KrSlice key, KrSlice val)
{
    return 0;
}

int kr_db_get (KrDb * db, KrSlice * out)
{
    return 0;
}

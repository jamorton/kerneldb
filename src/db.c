
#include "db.h"
#include "io.h"

#define MAX_DB 256
static KrDb databases[MAX_DB];

/*-----------------------------------------------------------*/
/* Superblock */
/*-----------------------------------------------------------*/

#define KR_SUPERBLOCK_MAGIC 0xFFEEDDCCBBAA9988ull

typedef struct KrSuperBlock {
    u64 magic;
    u64 opened;
} KrSuperBlock;

/**
 * Reads in the superblock, if no existing stored DB is found,
 * initializes a new one
 */
static void kr_db_init(KrDb* db)
{
    db->sb_buf = kr_buf_read(db->dev, 0);
    db->sb = (KrSuperBlock*)db->sb_buf->data;

    if (db->sb->magic == KR_SUPERBLOCK_MAGIC) {
        /* Existing db here: magic constant matches */
        db->sb->opened++;
    } else {
        /* Initialize new DB */
        db->sb->magic = KR_SUPERBLOCK_MAGIC;
        db->sb->opened = 0;
    }
    printk(KERN_INFO "kr_db_init: %llu opens\n", db->sb->opened);
    kr_buf_markdirty(db->sb_buf);
}

int kr_db_open(KrDb** db, const char * path) {
    KrDb * empty = NULL;
    int i;

    /* try to find an existing DB with this path */
    for (i = 0; i < MAX_DB; i++) {
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
        return -1;

    /* no existing db found... make a new one */
    strncpy(empty->path, path, 255);
    empty->refcnt = 1;
    empty->dev = kr_device_create(path, 512);
    *db = empty;

    kr_db_init(*db);

    return 0;
}

int kr_db_close (KrDb * db)
{
    kr_buf_unpin(db->sb_buf);
    if (--db->refcnt == 0) {
        kr_device_release(db->dev);
    }

    return 0;
}

int kr_db_put (KrDb * db, KrSlice key, KrSlice val)
{
    return 0;
}

int kr_db_get (KrDb * db, KrSlice key, KrSlice * out)
{
    return 0;
}

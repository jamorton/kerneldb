
#include "db.h"
#include "io.h"
#include "bucket.h"

#define MAX_DB 256
static KrDb databases[MAX_DB];

/*-----------------------------------------------------------*/
/* DB open, close, and setup                                 */
/*-----------------------------------------------------------*/

#define KR_SUPERBLOCK_MAGIC 0xFEEDBEEF8BADF00Dull

typedef struct KrSuperBlock {
    u64 magic;
    u64 opened;
    u64 num_entries;
    u64 num_buckets;

    /* linear hashing related */
    u64 r;
    u64 i;
} KrSuperBlock;

/**
 * Reads in the superblock, if no existing stored DB is found,
 * initializes a new one
 */
static void kr_db_init(KrDb* db)
{
    int i;
    KrBuf* buf;

    db->sb_buf = kr_buf_read(db->dev, 0);
    db->sb = (KrSuperBlock*)db->sb_buf->data;

    if (db->sb->magic == KR_SUPERBLOCK_MAGIC) {
        /* Existing db here: magic constant matches */
        db->sb->opened++;
    } else {
        /* Initialize new DB */
        db->sb->magic = KR_SUPERBLOCK_MAGIC;
        db->sb->opened = 0;
        db->sb->num_entries = 0;
        db->sb->i = 18;
        db->sb->num_buckets = 1 << db->sb->i;

        for (i = 0; i < db->sb->num_buckets; i++) {
            buf = kr_buf_alloc(db->dev, i + 1);
            kr_bucket_init(buf);
            kr_buf_unpin(buf);
        }
    }
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

    /* all databases taken... abort? */
    if (empty == NULL)
        return -1;

    /* no existing db found... make a new one */
    strncpy(empty->path, path, 255);
    empty->refcnt = 1;
    empty->dev = kr_device_create(path, 2048*128);
    *db = empty;

    kr_db_init(*db);
    printk(KERN_INFO "Opened DB %s (%llu entries)\n", path, (*db)->sb->num_entries);

    return 0;
}

int kr_db_close (KrDb * db)
{
    if (--db->refcnt == 0) {
        kr_buf_unpin(db->sb_buf);
        kr_device_release(db->dev);
    }

    return 0;
}

/*-----------------------------------------------------------*/
/* GET/PUT implementation                                    */
/*-----------------------------------------------------------*/

/* returns a KrBuf for the bucket that the given key should
   reside at */
static __always_inline u64 kr_bucket_location(KrSuperBlock* sb, KrSlice key)
{
    kr_hash hash = kr_slice_hash(key);
    kr_block b = hash & ((1 << sb->i) - 1); /* last i bits */

    if (unlikely(b >= sb->num_buckets))
        b ^= (1 << (sb->i - 1)); /* unset the top bit */

    return b + 1;
}

int kr_db_put (KrDb * db, KrSlice key, KrSlice val)
{
    u64 loc = kr_bucket_location(db->sb, key);
    KrBuf* bucket = kr_buf_read(db->dev, loc);
    int ret;
    if (!bucket)
        return -KR_ENOMEM;
    ret =  kr_bucket_add(bucket, key, val);
    kr_buf_unpin(bucket);
    return ret;
}

int kr_db_get (KrDb * db, KrSlice key, KrSlice * out)
{
    u64 loc = kr_bucket_location(db->sb, key);
    KrBuf* bucket = kr_buf_read(db->dev, loc);
    KrSlice val;

    if (!bucket)
        return -KR_ENOMEM;

    kr_bucket_get(bucket, key, &val);
    kr_buf_unpin(bucket);
    return 0;
}

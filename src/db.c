
#include "db.h"
#include "io.h"
#include "bucket.h"
#include "outbuf.h"

#define MAX_DB 16
static KrDb databases[MAX_DB];

/**
 * Bucket size and block size are individually configurable, so we support
 * multiple buckets per block.
 */

/* Get the block a bucket number should reside at  */
#define kr_bucket_block(b) ((b) / KR_BUCKETS_PER_BLOCK + 1)

/* Get a bucket data ptr from a buffer and bucket number */
static __always_inline char* kr_bucket_data(KrBuf* buf, u64 bkt_no)
{
    return (char*)buf->data + (bkt_no % KR_BUCKETS_PER_BLOCK) * KR_BUCKET_SIZE;
}

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
    char* data;

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
        db->sb->num_buckets = 1024;

        /* initialize the buckets */
        for (i = 0; i < db->sb->num_buckets; i++) {
            buf = kr_buf_alloc(db->dev, kr_bucket_block(i));
            data = kr_bucket_data(buf, i);
            kr_bucket_init(data);
            kr_buf_markdirty(buf);
            kr_buf_unpin(buf);
        }
    }
    kr_buf_markdirty(db->sb_buf);
}

int kr_db_open(KrDb** db, const char * path) {
    KrDb * empty = NULL;
    u32 i;

    /* try to find an existing DB with this path */
    for (i = 0; i < MAX_DB; i++) {
        if (databases[i].refcnt == 0) {
            empty = &databases[i];
            empty->id = i + 1;
        }
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

    empty->dev = kr_device_create(path, 2048+1);

    if (!empty->dev)
        return -1;

    empty->refcnt = 1;
    *db = empty;
    kr_db_init(empty);
    printk(KERN_INFO "Opened DB %s (%llu entries) with id %u\n",
        path, empty->sb->num_entries, empty->id);

    return 0;
}

KrDb* kr_db_from_id(u8 id)
{
    if (id <= 0 || (uint)id > (uint)MAX_DB)
        return NULL;
    if (databases[id - 1].refcnt < 1)
        return NULL;
    return &databases[id - 1];
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
static __always_inline u64 kr_get_bucket_num(KrSuperBlock* sb, KrSlice key)
{
    kr_hash hash = kr_slice_hash(key);
    kr_block b = hash & (sb->num_buckets - 1); /* last i bits */
    return b;
}

int kr_db_put (KrDb * db, KrSlice key, KrSlice val)
{
    /* hash the key to get the bucket number */
    u64 bkt_no = kr_get_bucket_num(db->sb, key);

    /* read the buffer that the bucket resides at */
    KrBuf* buf = kr_buf_read(db->dev, kr_bucket_block(bkt_no));

    /* get the pointer to the bucket's data from the buffer */
    char* data = kr_bucket_data(buf, bkt_no);

    int ret;

    if (!buf)
        return -KR_ENOMEM;

    ret = kr_bucket_add(data, key, val);

    if (ret == 0)
        db->sb->num_entries++;

    kr_buf_markdirty(buf);
    kr_buf_unpin(buf);
    return ret;
}

int kr_db_get (KrDb * db, KrSlice key, KrOutbuf* outbuf, u64* size)
{
    /* as above */
    u64 bkt_no = kr_get_bucket_num(db->sb, key);
    KrBuf* buf = kr_buf_read(db->dev, kr_bucket_block(bkt_no));
    char* data = kr_bucket_data(buf, bkt_no);
    KrSlice tmp;

    if (!buf)
        return -KR_ENOMEM;

    if (kr_bucket_get(data, key, &tmp) < 0) {
        printk(KERN_INFO " KEY NOT FOUND\n");
        return -KR_ENOTFOUND;
    }

    kr_outbuf_reserve_val(outbuf, tmp.size);
    kr_outbuf_put(outbuf, tmp.data, tmp.size);
    *size = tmp.size;

    kr_buf_unpin(buf);
    return 0;
}

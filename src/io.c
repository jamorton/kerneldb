
#include "io.h"

#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/bio.h>
#include <linux/log2.h>
#include <linux/hash.h>

/*-----------------------------------------------------------------------------
 * Buffer Hash
 *
 * Used to map buffer ID's to KrBuf instances
 * We can tell if a buffer is already cached by looking it up in this table
 *-----------------------------------------------------------------------------
 */

static __always_inline u32 kr_bufhash_bucket(KrDevice* dev, kr_block block)
{
    return block % dev->maxbufs;
    //return hash_64(block, 32) % dev->maxbufs;
}

static KrBuf* kr_bufhash_find(KrDevice* dev, kr_block block)
{
    KrBuf* buf = dev->bufhash[kr_bufhash_bucket(dev, block)];
    while (buf) {
        if (buf->block == block)
            return buf;
        buf = buf->next;
    }
    return NULL;
}

static void kr_bufhash_insert(KrBuf* buf)
{
    KrBuf* first = buf->dev->bufhash[buf->bucket];
    if (first)
        first->prev = buf;
    buf->next = first;
    buf->dev->bufhash[buf->bucket] = buf;
}

static void kr_bufhash_del(KrBuf* buf)
{
    KrBuf** bucket = &buf->dev->bufhash[buf->bucket];
    if (buf->prev)
        buf->prev->next = buf->next;
    if (buf->next)
        buf->next->prev = buf->prev;

    if (*bucket == buf) /* this buf is the first one in this bucket */
        *bucket = buf->next;

    buf->prev = buf->next = NULL;
}

/*-----------------------------------------------------------------------------
 * I/O Requests
 *
 * Functions to issue Linux bio requests for reading and writing disk blocks.
 * Entry points are _ and _.
 *-----------------------------------------------------------------------------
 */

#define KR_BUF_SECTOR(b) ((b->block)*(KR_BLOCK_SIZE/512))

static struct bio* kr_create_bio(KrDevice* dev, struct page* page, int sector)
{
    struct bio* bio = bio_alloc(GFP_NOIO, 1);

    if (!bio)
        return NULL;

    /* setup bio. */
    bio->bi_bdev = dev->bdev;
    bio->bi_sector = sector;

    bio_add_page(bio, page, KR_BLOCK_SIZE, 0);

    return bio;
}

/**
 * Callback given to bio requests, it will be called when the request completes.
 * See issue_bio_sync below.
 */
static void finish_bio_sync(struct bio *bio, int err)
{
    complete((struct completion*)bio->bi_private);
}

/**
 * This is synchronous, so we use a struct completion to block
 * until the request fnishes. The completion will be finished by the callback
 * finish_bio_sync
 */
static int issue_bio_sync(KrDevice* dev, struct page* page, int sector, int rw)
{
    struct completion event;
    struct bio* bio = kr_create_bio(dev, page, sector);
    if (!bio)
        return -KR_ENOMEM;

    bio->bi_end_io = finish_bio_sync;
    bio->bi_private = &event;

    init_completion(&event);
    submit_bio(rw | REQ_SYNC, bio);
    wait_for_completion(&event);

    bio_put(bio);
    return 0;
}

static void finish_bio_async(struct bio* bio, int err)
{
    bio_put(bio);
}

static int issue_bio_async(KrDevice* dev, struct page* page, int sector, int rw)
{
    struct bio* bio = kr_create_bio(dev, page, sector);

    if (!bio)
        return -KR_ENOMEM;

    bio->bi_end_io = finish_bio_async;
    submit_bio(rw, bio);
    return 0;
}

/*-----------------------------------------------------------------------------
 * Buffer Manager
 *
 *-----------------------------------------------------------------------------
 */

static __always_inline void kr_buf_maybe_write(KrBuf* buf)
{
    if (kr_buf_isdirty(buf)) {
        issue_bio_async(buf->dev, buf->page, KR_BUF_SECTOR(buf), WRITE_FLUSH_FUA);
        buf->flags &= ~KR_BUF_DIRTY; /* clear dirty bit */
    }
}

/**
 * Find an unpinned buf suitable for replacement
 * For now, just evicts the first available buffer it sees.
 */
static KrBuf* kr_buf_evict(KrDevice* dev)
{
    size_t i;
    KrBuf* buf = NULL;

    for (i = 0; i < dev->maxbufs; i++) {
        buf = dev->bufhash[i];
        while (buf) {
            if (buf->pincnt == 0)
                goto found;
            buf = buf->next;
        }
    }

    /* no unpinned bufs! */
    return NULL;

 found:
    kr_bufhash_del(buf);
    kr_buf_maybe_write(buf);
    return buf;
}

KrBuf* kr_buf_get(KrDevice* dev, kr_block loc, bool read)
{
    KrBuf* buf = kr_bufhash_find(dev, loc);

    /* it's in the cache already: just increase the pincount */
    if (buf) {
        dev->n_hit++;
        buf->pincnt++;
        return buf;
    }

    if (dev->bufcnt >= dev->maxbufs) {
        dev->n_evict++;
        /* cache is already at max size: must evict another unused buffer */
        buf = kr_buf_evict(dev);
        if (!buf)
            return NULL;
    } else {
        /* if the cache isn't at max size yet, allocate a new buffer */
        buf = kzalloc(sizeof(KrBuf), GFP_KERNEL);
        if (!buf)
            return NULL;
        buf->page = alloc_pages(GFP_KERNEL, KR_PAGE_ALLOC_ORDER);
        buf->data = page_address(buf->page);
    }

    buf->block = loc;
    buf->pincnt = 1;
    buf->dev = dev;
    buf->bucket = kr_bufhash_bucket(dev, loc);

    kr_bufhash_insert(buf);

    dev->bufcnt++;

    if (read) {
        dev->n_read++;
        issue_bio_sync(dev, buf->page, KR_BUF_SECTOR(buf), READ);
    }

    return buf;
}

void kr_buf_free(KrBuf* buf)
{
    __free_pages(buf->page, KR_PAGE_ALLOC_ORDER);
    kfree(buf);
}

/*-----------------------------------------------------------------------------
 * KrDevice
 *
 * Implementation
 *-----------------------------------------------------------------------------
 */

KrDevice* kr_device_create (const char* path, size_t cachesz)
{
    struct block_device* block_dev;
    KrDevice* dev;

    /* get a struct block_device pointer from a string path */
    block_dev = blkdev_get_by_path(path, FMODE_READ | FMODE_WRITE, NULL);
    if (!block_dev)
        return NULL;

    /* allocate the KrDevice */
    dev = (KrDevice*)kmalloc(sizeof(KrDevice), GFP_KERNEL);
    dev->bufcnt = 0;
    dev->bdev = block_dev;
    dev->maxbufs = cachesz;
    dev->bufhash = kcalloc(dev->maxbufs, sizeof(KrBuf*), GFP_KERNEL);

    dev->n_evict = dev->n_read = dev->n_hit = dev->n_dbl = 0;

    return dev;
}

void kr_device_release(KrDevice* dev)
{
    int i;
    KrBuf *buf, *tmp;

    printk(KERN_INFO "evict %u read %u hit %u dbl %u\n",
        dev->n_evict, dev->n_read, dev->n_hit, dev->n_dbl);

    /* write dirty bufs, and free all allocated bufs */
    for (i = 0; i < dev->maxbufs; i++) {
        buf = dev->bufhash[i];
        while (buf) {
            /* we're freeing buf and can't access buf->next after we do */
            tmp = buf->next;
            kr_buf_maybe_write(buf);
            kr_buf_free(buf);
            buf = tmp;
        }
    }

    blkdev_put(dev->bdev, FMODE_READ | FMODE_WRITE);
    kfree(dev->bufhash);
    kfree(dev);
}

void kr_device_flush(KrDevice* dev)
{
    int i;
    KrBuf* buf;

    for (i = 0; i < dev->maxbufs; i++) {
        buf = dev->bufhash[i];
        while (buf) {
            kr_buf_maybe_write(buf);
            buf = buf->next;
        }
    }
}

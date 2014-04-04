
#include "io.h"

#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/bio.h>
#include <linux/log2.h>

/*-----------------------------------------------------------------------------
 * Buffer Hash
 *
 * Used to map buffer ID's to KrBuf instances
 * We can tell if a buffer is already cached by looking it up in this table
 *-----------------------------------------------------------------------------
 */

static __always_inline u64 kr_bufhash_bucket(KrDevice* dev, kr_bufid id)
{
    return hash_64(id, 64) % dev->maxbufs;
}

static KrBuf* kr_bufhash_find(KrDevice* dev, kr_bufid id)
{
    KrBuf* buf = dev->bufhash[kr_bufhash_bucket(dev, id)];
    while (buf) {
        if (buf->id == id)
            return buf;
        buf = buf->next;
    }
    return NULL;
}

static void kr_bufhash_insert(KrBuf* buf)
{
    KrBuf** bufptr = &buf->dev->bufhash[buf->bucket];
    if (*bufptr)
        (*bufptr)->prev = buf;
    buf->next = *bufptr;
    *bufptr = buf;
}

static void kr_bufhash_del(KrBuf* buf)
{
    if (buf->prev)
        buf->prev = buf->next;
    if (buf->next)
        buf->next = buf->prev;
    else if (buf->prev == NULL) /* no next, no prev: length 1 list */
        buf->dev->bufhash[buf->bucket] = NULL;
    buf->prev = buf->next = NULL;
}

/*-----------------------------------------------------------------------------
 * I/O Requests
 *
 * Functions to issue Linux bio requests for reading and writing disk blocks.
 * Entry points are _ and _.
 *-----------------------------------------------------------------------------
 */

#define KR_BUF_SECTOR(b) ((b->id)*(KR_BUFFER_SIZE/512))

static struct bio* kr_create_bio(KrDevice* dev, struct page* page, int sector)
{
    struct bio* bio = bio_alloc(GFP_NOIO, 1);

    // setup bio.
    bio->bi_bdev = dev->bdev;
    bio->bi_sector = sector;

    bio_add_page(bio, page, KR_BUFFER_SIZE, 0);

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
    if (kr_buf_isdirty(buf))
        issue_bio_async(buf->dev, buf->page, KR_BUF_SECTOR(buf), WRITE_FLUSH_FUA);
}

/**
 * Find an unpinned buf suitable for replacement
 * For now, just evicts the first available buffer it sees.
 */
static KrBuf* kr_buf_evict(KrDevice* dev)
{
    size_t i;
    KrBuf* buf;

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

KrBuf* kr_buf_get(KrDevice* dev, kr_bufid id, bool read)
{
    KrBuf* buf = kr_bufhash_find(dev, id);

    /* it's in the cache already: just increase the pincount */
    if (buf) {
        buf->pincnt++;
        return buf;
    }

    if (dev->bufcnt >= dev->maxbufs)
        buf = kr_buf_evict(dev);
    else {
        buf = kzalloc(sizeof(KrBuf), GFP_KERNEL);
        buf->page = alloc_pages(GFP_KERNEL, KR_PAGE_ALLOC_ORDER);
        buf->data = page_address(buf->page);
    }

    buf->id = id;
    buf->pincnt = 1;
    buf->dev = dev;
    buf->bucket = kr_bufhash_bucket(dev, id);

    kr_bufhash_insert(buf);

    if (read)
        issue_bio_sync(dev, buf->page, KR_BUF_SECTOR(buf), READ);

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

    // linux block layer function to get a struct block_device pointer
    // from a string path
    block_dev = blkdev_get_by_path(path, FMODE_READ | FMODE_WRITE, NULL);
    if (!block_dev)
        return NULL;

    // allocate the KrDevice
    dev = (KrDevice*)kmalloc(sizeof(KrDevice), GFP_KERNEL);
    dev->bufcnt = 0;
    dev->bdev = block_dev;
    dev->maxbufs = cachesz;
    dev->bufhash = kcalloc(dev->maxbufs, sizeof(KrBuf*), GFP_KERNEL);

    return dev;
}

void kr_device_release(KrDevice* dev)
{
    int i;
    KrBuf* buf;

    /* write dirty bufs, and free all allocated bufs */
    for (i = 0; i < dev->maxbufs; i++) {
        buf = dev->bufhash[i];
        while (buf) {
            kr_buf_maybe_write(buf);
            kr_buf_free(buf);
            buf = buf->next;
        }
    }

    blkdev_put(dev->bdev, FMODE_READ | FMODE_WRITE);
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


/**
 * io.h, io.c - low level storage related implementation
 */

#ifndef KR_IO_H
#define KR_IO_H

#include "internal.h"

/*-----------------------------------------------------------------------------
 * Simple device API
 *
 * Used to translate user-specified paths (e.g. to partitions)
 * into generic block devices for reading and writing.
 *-----------------------------------------------------------------------------
 */

// forward declarations
struct block_device;
struct KrBuf;

typedef struct KrDevice {
    size_t bufcnt;
    size_t maxbufs;
    struct KrBuf** bufhash; /* maps buf IDs (disk locations) to KrBufs */

    /* linux implementation */
    struct block_device* bdev;
} KrDevice;

/**
 * kr_device_create
 *
 *     Create and return a new KrDevice instance for a given linux block device
 *     path: VFS path of the block device (e.g. /dev/sda)
 *     cachesz: maximum number of bufs to keep in memory
 */
KrDevice*  kr_device_create   (const char* path, size_t cachesz);

/**
 * kr_device_release
 *
 *     Frees this KrDevice instance.
 */

void kr_device_release (KrDevice* dev);

/**
 * kr_device_flush
 *
 *     Write all dirty in-memory buffers to disk.
 */
void kr_device_flush(KrDevice* dev);

/*-----------------------------------------------------------------------------
 * Buffer Manager
 *
 * THE base unit of storage used by the rest of the system. Internally uses
 * low-level kernel bio requests to issue reads and writes.
 *-----------------------------------------------------------------------------
 */

/**
 * Linux allocates pages in powers of two only, so we specify how many pages
 * constitute a single buffer in the form 2^order
 */
#define KR_PAGE_ALLOC_ORDER 2 /* 2 pages per buffer (8912 bytes) */

/**
 * So as above, the buffer size is then PAGE_SIZE * 2^order
 */
#define KR_BUFFER_SIZE (PAGE_SIZE * (1 << KR_PAGE_ALLOC_ORDER))

typedef u64 kr_block;

/**
 * Buffer flags
 */
#define KR_BUF_DIRTY 0x01u

/**
 * KrBuf struct
 *
 *     Holds information about active buffers
 *     Keep this 64 bytes long: exactly one cache line
 */
typedef struct KrBuf {
    kr_block block;
    u32 flags;
    u32 pincnt; /* pin count */
    u32 bucket;
    u32 unused; /* not used yet (padding) */
    KrDevice* dev;
    struct KrBuf* next; /* for linked lists */
    struct KrBuf* prev;
    void* data; /* pointer to the buffer's data, BUFFER_SIZE bytes long */

    // linux impl. fields
    struct page* page;
} KrBuf;

KrBuf* kr_buf_get(KrDevice*, kr_block, bool);

/**
 * kr_buf_alloc
 *
 *     Get a buffer an unused location in memory. Will never read the existing
 *     data at that location into the buffer from disk.
 */
static __always_inline KrBuf* kr_buf_alloc(KrDevice* dev, kr_block loc)
{
    return kr_buf_get(dev, loc, false);
}

/**
 * kr_buf_read
 *
 *     Get a buffer for a location on disk. May block to read in the data
 *     from disk if it isn't in the cache.
 */
static __always_inline KrBuf* kr_buf_read(KrDevice* dev, kr_block loc)
{
    return kr_buf_get(dev, loc, true);
}

/**
 * kr_buf_unpin
 *
 *     Give up a reference to the given buffer. Every kr_buf_alloc or
 *     kr_buf_read should have a corresponding kr_buf_unpin call
 *     when the buffer is no longer needed.
 */
static __always_inline void kr_buf_unpin(KrBuf* buf)
{
    buf->pincnt--;
}

/**
 * kr_buf_dirty
 *
 *    Tell the buffer manager this buffer's data has changed, so it
 *    will be flushed to disk before being freed
 */
static __always_inline void kr_buf_markdirty(KrBuf* buf)
{
    buf->flags |= KR_BUF_DIRTY;
}

static __always_inline int kr_buf_isdirty(KrBuf* buf)
{
    return buf->flags & KR_BUF_DIRTY;
}

#endif

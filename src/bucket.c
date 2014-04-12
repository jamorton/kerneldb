
#include "bucket.h"
#include "io.h"

/**
 * A bucket has a header at byte 0, followed by an array of `count` KrTupleInfos
 * directly after.
 * The actual tuple data is filled in from the end of the bucket
 * backwards. `data_off` marks the beginning of this region, as an offset
 * from byte 0.
 */
typedef struct __attribute__((packed)) KrBucketHdr {
    u16 data_off; /* offset to space used from bottom */
    u16 count; /* number of elements in this bucket */
    u16 deleted_space;
} KrBucketHdr;

typedef struct __attribute__((packed)) KrTupleInfo {
    u16 offs; /* offset into bucket storing the actual key/value data */
    u16 klen; /* key length */
    u16 vlen; /* value length */
} KrTupleInfo;

static __always_inline KrTupleInfo* kr_tuple_info_array(char* data)
{
    return (KrTupleInfo*)(data + sizeof(KrBucketHdr));
}

static __always_inline uint kr_bucket_freespace(char* data)
{
    KrBucketHdr* hdr = (KrBucketHdr*)data;
    return hdr->data_off - hdr->count * sizeof(KrTupleInfo) - sizeof(KrBucketHdr);
}

void kr_bucket_init(char* data)
{
    KrBucketHdr* hdr = (KrBucketHdr*)data;
    hdr->data_off = KR_BUCKET_SIZE;
    hdr->count = 0;
    hdr->deleted_space = 0;
}

static __always_inline KrTupleInfo* kr_bucket_find_key(char* data, KrSlice* key)
{
    KrBucketHdr* hdr = (KrBucketHdr*)data;
    KrTupleInfo* infos = kr_tuple_info_array(data);
    int i;

    for (i = 0; i < hdr->count; i++) {
        //printk(KERN_INFO "info: offs %d  klen %d  vlen %d\n", infos[i].offs, infos[i].klen, infos[i].vlen);
        if (infos[i].klen == key->size) {
            if (memcmp(key->data, data + infos[i].offs, key->size) == 0)
                return &infos[i];
        }
    }
    return NULL;
}

/**
 * Note that we don't delete any of the actual key/value data from the bucket
 * here. Instead, we just remove the entry's KrTupleInfo and keep track of how
 * much previously used space has been deleted.
 *
 * A bucket can later reclaim all of the dead data through a defrag.
 *
 * So in general we just keep filling a bucket's data section through
 * puts/deletes, and if we run out of data space, and we really want
 * to insert into this bucket, we can do a defrag to recliam space.
 */
static __always_inline void __kr_bucket_del(char* data, KrTupleInfo* to_del)
{
    KrBucketHdr* hdr = (KrBucketHdr*)data;
    KrTupleInfo* last = kr_tuple_info_array(data) + hdr->count - 1;

    hdr->count--;
    hdr->deleted_space += to_del->klen + to_del->vlen;
    *to_del = *last;
}

int kr_bucket_del(char* data, KrSlice* key)
{
    KrTupleInfo* to_del = kr_bucket_find_key(data, key);

    /* key isn't even in this bucket? */
    if (unlikely(!to_del))
        return -KR_ENOTFOUND;

    __kr_bucket_del(data, to_del);
    return 0;
}

int kr_bucket_get(char* data, KrSlice key, KrSlice* val)
{
    KrTupleInfo* found = kr_bucket_find_key(data, &key);

    if (unlikely(!found))
        return -KR_ENOTFOUND;

    val->size = found->vlen;
    val->data = data + found->offs + found->klen;
    return 0;
}

int kr_bucket_add(char* data, KrSlice key, KrSlice val)
{
    KrBucketHdr* hdr = (KrBucketHdr*)data;
    u16 kvlen = key.size + val.size;
    u16 freespace;
    KrTupleInfo* tuple = kr_bucket_find_key(data, &key);

    //printk(KERN_INFO "header: count %d  upper %d  lower %d\n", hdr->count, hdr->upper, hdr->lower);

    if (tuple) {
        /* if the entry already exists, just delete it before re-inserting */
        __kr_bucket_del(data, tuple);
    }

    freespace = kr_bucket_freespace(data);
    //printk(KERN_INFO "freespace: %d\n", freespace);
    if (unlikely(freespace < kvlen + sizeof(KrTupleInfo))) {
        /* no space left in this bucket */
        printk(KERN_INFO "No space left in bucket! Not implemented\n");
        return 0;
    }

    /* do insert */
    tuple = (KrTupleInfo*)(data + sizeof(KrBucketHdr) + hdr->count * sizeof(KrTupleInfo));
    tuple->klen = key.size;
    tuple->vlen = val.size;
    tuple->offs = hdr->data_off - kvlen;

    //printk(KERN_INFO "new tuple: offs %d  klen %d  vlen %d\n", tuple->offs, tuple->klen, tuple->vlen);

    memcpy(data + tuple->offs, key.data, key.size);
    memcpy(data + tuple->offs + key.size, val.data, val.size);

    hdr->count++;
    hdr->data_off -= kvlen;

    return 0;
}
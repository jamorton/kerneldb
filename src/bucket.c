
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
    uint i;

    /* search backwards so we find updated keys first */
    for (i = 0; i < hdr->count; i++) {
        if (infos[i].klen == key->size &&
            memcmp(key->data, data + infos[i].offs, key->size) == 0)
          return &infos[i];
    }
    return NULL;
}

static __always_inline void kr_bucket_defrag(char* data)
{
    KrBucketHdr* hdr = (KrBucketHdr*)data;
    KrTupleInfo* infos = kr_tuple_info_array(data);
    uint i;
    u16 cur_off = KR_BUCKET_SIZE;
    char* tmp = kmalloc(KR_BUCKET_SIZE, GFP_KERNEL);

    for (i = 0; i < hdr->count; i++) {
        u16 len = infos[i].klen + infos[i].vlen;
        cur_off -= len;
        memcpy(tmp + cur_off, data + infos[i].offs, len);
        infos[i].offs = cur_off;
    }

    memcpy(data + cur_off, tmp + cur_off, KR_BUCKET_SIZE - cur_off);
    kfree(tmp);

    hdr->deleted_space = 0;
    hdr->data_off = cur_off;
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
    KrTupleInfo* last;

    if (hdr->count == 0) {
        printk("BAD");
        return;
    }

    last = kr_tuple_info_array(data) + (hdr->count - 1);

    hdr->count--;
    hdr->deleted_space += to_del->klen + to_del->vlen;

    if (to_del != last)
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
    int ret = 0;
    u16 kvlen = key.size + val.size;
    u16 freespace = kr_bucket_freespace(data);

    KrTupleInfo* tuple = kr_bucket_find_key(data, &key);

    if (tuple) {
        __kr_bucket_del(data, tuple);
        ret = 1;
    }

    if (unlikely(freespace < kvlen + sizeof(KrTupleInfo))) {
        /* no space left in this bucket */
        kr_bucket_defrag(data);
    }

    /* do insert */
    tuple = kr_tuple_info_array(data) + hdr->count;
    tuple->klen = key.size;
    tuple->vlen = val.size;
    tuple->offs = hdr->data_off - kvlen;

    //printk(KERN_INFO "new tuple: offs %d  klen %d  vlen %d  free %d\n", tuple->offs, tuple->klen, tuple->vlen, freespace);

    memcpy(data + tuple->offs, key.data, key.size);
    memcpy(data + tuple->offs + key.size, val.data, val.size);

    if (hdr->data_off < kvlen) {
        printk(KERN_INFO " ERROR data_off (%u) < kvlen (%u)\n", (uint)hdr->data_off, (uint)kvlen);
    }

    hdr->count++;
    hdr->data_off -= kvlen;

    return ret;
}

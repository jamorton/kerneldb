
#ifndef KR_OUTBUF_H
#define KR_OUTBUF_H

#include "internal.h"

typedef struct KrOutbuf {
    u64 hdrsz;
    u64 capacity;
    u8* data;
    u8* cur;
} KrOutbuf;

static __always_inline u64 kr_outbuf_used(KrOutbuf* buf)
{
    return (buf->cur - buf->data);
}

static __always_inline u64 kr_outbuf_free(KrOutbuf* buf)
{
    return buf->capacity - kr_outbuf_used(buf);
}

static __always_inline KrOutbuf kr_outbuf(u64 hdrsize)
{
    KrOutbuf ret;
    ret.hdrsz = hdrsize;
    ret.capacity = 0;
    ret.data = ret.cur = NULL;
    return ret;
}

static __always_inline void kr_outbuf_reserve_val(KrOutbuf* buf, u64 len)
{
    u64 size = buf->hdrsz + len;
    if (size > buf->capacity) {
        if (buf->data != NULL)
            kfree(buf->data);
        buf->data = kmalloc(size, GFP_KERNEL);
        buf->capacity = size;
    }
    buf->cur = buf->data + buf->hdrsz;
}

static __always_inline void kr_outbuf_put(KrOutbuf* buf, const void* data, u64 len)
{
    BUG_ON(len > kr_outbuf_free(buf));
    memcpy(buf->cur, data, len);
    buf->cur += len;
}

#endif

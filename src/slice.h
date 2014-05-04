
#ifndef KR_SLICE_H
#define KR_SLICE_H

typedef struct KrSlice {
    u64 size;
    kr_dataptr data;
} KrSlice;

/**
 * Slice hashing (used as primary key hash)
 */

#define kr_rol(x, n) (((x)<<(n)) | ((x)>>(-(int)(n)&(8*sizeof(x)-1))))
#define kr_getu32(p) (*(const u32 *)(p))

#if 0
static __always_inline kr_hash kr_slice_hash(KrSlice key)
{
    const char *str = key.data;
    u64 len = key.size;
    kr_hash a, b, h = (kr_hash)key.size;

    if (len >= 4) {  /* Caveat: unaligned access! */
        a = kr_getu32(str);
        h ^= kr_getu32(str+len-4);
        b = kr_getu32(str+(len>>1)-2);
        h ^= b; h -= kr_rol(b, 14);
        b += kr_getu32(str+(len>>2)-1);
    } else {
        a = *(const u8 *)str;
        h ^= *(const u8 *)(str+len-1);
        b = *(const u8 *)(str+(len>>1));
        h ^= b; h -= kr_rol(b, 14);
    }
    a ^= h; a -= kr_rol(h, 11);
    b ^= a; b -= kr_rol(a, 25);
    h ^= b; h -= kr_rol(b, 16);
    return h;
}
#endif

static __always_inline kr_hash kr_slice_hash(KrSlice key)
{
    unsigned long hash = 5381;
    int i;

    for (i = 0; i < key.size; i++)
        hash = ((hash << 5) + hash) + key.data[i]; /* hash * 33 + c */

    return hash;
}

#endif

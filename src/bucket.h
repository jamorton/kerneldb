
#ifndef KR_BUCKET_H
#define KR_BUCKET_H

#include "internal.h"
#include "io.h"

#define KR_BUCKET_SIZE KR_BLOCK_SIZE

struct KrBuf;

void kr_bucket_init(struct KrBuf* buf);
int kr_bucket_del(KrBuf* buf, KrSlice* key);
int kr_bucket_get(struct KrBuf* buf, KrSlice key, KrSlice* val);
int kr_bucket_add(struct KrBuf* buf, KrSlice key, KrSlice val);

#endif
